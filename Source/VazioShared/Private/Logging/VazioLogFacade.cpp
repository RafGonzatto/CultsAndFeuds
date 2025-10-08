#include "Logging/VazioLogFacade.h"

#include "Misc/DateTime.h"
#include "Misc/OutputDevice.h"
#include "Misc/ScopeLock.h"
#include "Misc/Paths.h"
#include "HAL/PlatformTime.h"
#include "HAL/PlatformMisc.h"
#include "HAL/PlatformCrt.h"
#include "Containers/Array.h"
#include "Templates/UniquePtr.h"

namespace FVazioLog
{
	namespace
	{
		class FVazioConsoleSink final : public IVazioLogSink
		{
		public:
			virtual void Emit(const FString& Line, EVazioLogLevel Level) override
			{
				const FString WithNewline = Line + LINE_TERMINATOR;
				FPlatformMisc::LocalPrint(*WithNewline);

				if (GLog)
				{
					GLog->Serialize(*Line, ConvertLevel(Level), NAME_None);
				}
			}

		private:
			static ELogVerbosity::Type ConvertLevel(EVazioLogLevel Level)
			{
				switch (Level)
				{
				case EVazioLogLevel::Trace:
					return ELogVerbosity::Verbose;
				case EVazioLogLevel::Debug:
					return ELogVerbosity::VeryVerbose;
				case EVazioLogLevel::Info:
					return ELogVerbosity::Log;
				case EVazioLogLevel::Warn:
					return ELogVerbosity::Warning;
				case EVazioLogLevel::Error:
					return ELogVerbosity::Error;
				default:
					return ELogVerbosity::Log;
				}
			}
		};

		struct FLoopThrottleState
		{
			uint64 CallCount = 0;
			double LastEmitSeconds = 0.0;
		};

		FVazioLogConfigLoader ConfigLoader;
		TAtomic<bool> bInitialized(false);
		TAtomic<bool> bGlobalEnabled(false);
		TAtomic<uint32> EnabledCategoriesMask(0u);
		TAtomic<int32> MinLogLevel(static_cast<int32>(EVazioLogLevel::Info));

		FCriticalSection SinkMutex;
		TUniquePtr<IVazioLogSink> ActiveSink;

		FCriticalSection LoopMutex;
		TMap<FName, FLoopThrottleState> LoopStates;

		FString BuildTimestamp()
		{
			const FDateTime Now = FDateTime::UtcNow();
			return FString::Printf(TEXT("%02d:%02d:%02d.%03d"), Now.GetHour(), Now.GetMinute(), Now.GetSecond(), Now.GetMillisecond());
		}

		FString FormatMessage(const TCHAR* Format, va_list Args)
		{
			TArray<TCHAR, TInlineAllocator<512>> Scratch;
			Scratch.SetNumUninitialized(512);

			while (true)
			{
				va_list ArgsCopy;
				va_copy(ArgsCopy, Args);

				const TCHAR* LocalFormat = Format;
				const int32 Result = FCString::GetVarArgs(Scratch.GetData(), Scratch.Num(), LocalFormat, ArgsCopy);
				va_end(ArgsCopy);

				if (Result > -1 && Result < Scratch.Num())
				{
					Scratch[Result] = TEXT('\0');
					return FString(Result, Scratch.GetData());
				}

				const int32 NewSize = (Result > -1) ? (Result + 1) : Scratch.Num() * 2;
				Scratch.SetNumUninitialized(NewSize);
			}
		}

		FString ComposeLine(EVazioLogCategory Category, EVazioLogLevel Level, bool bIsLoop, const FString& Message)
		{
			const FString Timestamp = BuildTimestamp();
			const FString CategoryLabel = ToString(Category);
			const FString LevelLabel = ToString(Level);

			if (bIsLoop)
			{
				return FString::Printf(TEXT("[%s][%s][%s][LOOP] %s"), *Timestamp, *CategoryLabel, *LevelLabel, *Message);
			}

			return FString::Printf(TEXT("[%s][%s][%s] %s"), *Timestamp, *CategoryLabel, *LevelLabel, *Message);
		}

		void EnsureSink_Locked()
		{
			if (!ActiveSink.IsValid())
			{
				ActiveSink = MakeUnique<FVazioConsoleSink>();
			}
		}
	}

	void FVazioLogFacade::Initialize()
	{
		const bool bWasInitialized = bInitialized.Exchange(true);
		if (!bWasInitialized)
		{
			ReloadConfig();
			FScopeLock Lock(&SinkMutex);
			EnsureSink_Locked();
		}
	}

	void FVazioLogFacade::Shutdown()
	{
		if (bInitialized.Exchange(false))
		{
			FScopeLock Lock(&SinkMutex);
			ActiveSink.Reset();

			FScopeLock LoopLock(&LoopMutex);
			LoopStates.Empty();
		}
	}

	bool FVazioLogFacade::ReloadConfig()
	{
		FVazioLogRuntimeConfig LoadedConfig;
		if (!ConfigLoader.Load(LoadedConfig))
		{
			return false;
		}

		bGlobalEnabled.Store(LoadedConfig.bGlobalEnabled);
		EnabledCategoriesMask.Store(LoadedConfig.CategoryMask);
		MinLogLevel.Store(static_cast<int32>(LoadedConfig.MinLevel));

		{
			FScopeLock LoopLock(&LoopMutex);
			LoopStates.Empty();
		}

		return true;
	}

	void FVazioLogFacade::SetMinLevel(EVazioLogLevel Level)
	{
		MinLogLevel.Store(static_cast<int32>(Level));
	}

	bool FVazioLogFacade::IsEnabled(EVazioLogCategory Category)
	{
		if (!bGlobalEnabled.Load())
		{
			return false;
		}

		const uint32 Mask = EnabledCategoriesMask.Load();
		return (Mask & CategoryToBit(Category)) != 0u;
	}

	bool FVazioLogFacade::ShouldLog(EVazioLogCategory Category, EVazioLogLevel Level, bool bIsLoop)
	{
		return InternalShouldLog(Category, Level, bIsLoop);
	}

	bool FVazioLogFacade::InternalShouldLog(EVazioLogCategory Category, EVazioLogLevel Level, bool /*bIsLoop*/)
	{
		if (!bInitialized.Load())
		{
			return false;
		}

		if (!bGlobalEnabled.Load())
		{
			return false;
		}

		const int32 ConfiguredMinLevel = MinLogLevel.Load();
		if (static_cast<int32>(Level) < ConfiguredMinLevel)
		{
			return false;
		}

		const uint32 Mask = EnabledCategoriesMask.Load();
		const uint32 Bit = CategoryToBit(Category);
		return (Mask & Bit) != 0u;
	}

	bool FVazioLogFacade::ShouldEmitLoop(const FName& Key, int32 EveryNth, float MinSeconds)
	{
		const int32 Interval = FMath::Max(1, EveryNth);
		const double MinDelta = FMath::Max(0.f, MinSeconds);
		const double NowSeconds = FPlatformTime::Seconds();

		FScopeLock Lock(&LoopMutex);
		FLoopThrottleState& State = LoopStates.FindOrAdd(Key);
		State.CallCount += 1;

		const bool bCountSatisfied = (State.CallCount == 1) || ((State.CallCount % static_cast<uint64>(Interval)) == 0);
		const bool bTimeSatisfied = (MinDelta <= 0.0) || ((NowSeconds - State.LastEmitSeconds) >= MinDelta);

		if (bCountSatisfied && bTimeSatisfied)
		{
			State.LastEmitSeconds = NowSeconds;
			return true;
		}

		return false;
	}

	void FVazioLogFacade::Log(EVazioLogCategory Category, EVazioLogLevel Level, bool bIsLoop, const TCHAR* Format, ...)
	{
		if (!InternalShouldLog(Category, Level, bIsLoop))
		{
			return;
		}

		va_list ArgPtr;
		va_start(ArgPtr, Format);
		const FString Message = FormatMessage(Format, ArgPtr);
		va_end(ArgPtr);

		const FString Line = ComposeLine(Category, Level, bIsLoop, Message);

		FScopeLock Lock(&SinkMutex);
		EnsureSink_Locked();
		ActiveSink->Emit(Line, Level);
	}

	void FVazioLogFacade::SetSink(TUniquePtr<IVazioLogSink>&& InSink)
	{
		FScopeLock Lock(&SinkMutex);
		ActiveSink = MoveTemp(InSink);
	}

	FVazioLogRuntimeConfig FVazioLogFacade::GetConfigSnapshot()
	{
		FVazioLogRuntimeConfig Snapshot;
		Snapshot.bGlobalEnabled = bGlobalEnabled.Load();
		Snapshot.CategoryMask = EnabledCategoriesMask.Load();
		Snapshot.MinLevel = static_cast<EVazioLogLevel>(MinLogLevel.Load());
		return Snapshot;
	}
}
