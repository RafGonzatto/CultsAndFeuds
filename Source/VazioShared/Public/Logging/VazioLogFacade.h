#pragma once

#include "CoreMinimal.h"
#include "Logging/VazioLogConfig.h"
#include "Templates/UniquePtr.h"
#include "Misc/Char.h"

namespace FVazioLog
{
	class VAZIOSHARED_API IVazioLogSink
	{
	public:
		virtual ~IVazioLogSink() = default;
		virtual void Emit(const FString& Line, EVazioLogLevel Level) = 0;
	};

	FORCEINLINE FName MakeLoopKey(const ANSICHAR* FunctionName)
	{
		return FName(ANSI_TO_TCHAR(FunctionName));
	}

#if !PLATFORM_TCHAR_IS_ANSICHAR
	FORCEINLINE FName MakeLoopKey(const WIDECHAR* FunctionName)
	{
		return FName(FunctionName);
	}
#endif

	class VAZIOSHARED_API FVazioLogFacade
	{
	public:
		static void Initialize();
		static void Shutdown();

		static bool ReloadConfig();
		static void SetMinLevel(EVazioLogLevel Level);
		static bool IsEnabled(EVazioLogCategory Category);
		static bool ShouldLog(EVazioLogCategory Category, EVazioLogLevel Level, bool bIsLoop);
		static bool ShouldEmitLoop(const FName& Key, int32 EveryNth, float MinSeconds);
		static void Log(EVazioLogCategory Category, EVazioLogLevel Level, bool bIsLoop, const TCHAR* Format, ...);
		static void SetSink(TUniquePtr<IVazioLogSink>&& InSink);

		static FVazioLogRuntimeConfig GetConfigSnapshot();

	private:
		static bool InternalShouldLog(EVazioLogCategory Category, EVazioLogLevel Level, bool bIsLoop);
	};
}

#define VAZIO_LOG_GUARD(CategoryEnum, LevelExpr, bLoopFlag, Format, ...) \
	do \
	{ \
		const FVazioLog::EVazioLogLevel VazioResolvedLevel = (LevelExpr); \
		if (FVazioLog::FVazioLogFacade::ShouldLog(CategoryEnum, VazioResolvedLevel, bLoopFlag)) \
		{ \
			FVazioLog::FVazioLogFacade::Log(CategoryEnum, VazioResolvedLevel, bLoopFlag, Format, ##__VA_ARGS__); \
		} \
	} while (0)

#define LOG_UI(LevelToken, Format, ...) VAZIO_LOG_GUARD(FVazioLog::EVazioLogCategory::UI, ::FVazioLog::LevelTokens::LevelToken, false, Format, ##__VA_ARGS__)
#define LOG_ENEMIES(LevelToken, Format, ...) VAZIO_LOG_GUARD(FVazioLog::EVazioLogCategory::Enemies, ::FVazioLog::LevelTokens::LevelToken, false, Format, ##__VA_ARGS__)
#define LOG_WEAPONS(LevelToken, Format, ...) VAZIO_LOG_GUARD(FVazioLog::EVazioLogCategory::Weapons, ::FVazioLog::LevelTokens::LevelToken, false, Format, ##__VA_ARGS__)
#define LOG_UPGRADES(LevelToken, Format, ...) VAZIO_LOG_GUARD(FVazioLog::EVazioLogCategory::Upgrades, ::FVazioLog::LevelTokens::LevelToken, false, Format, ##__VA_ARGS__)
#define LOG_MASS(LevelToken, Format, ...) VAZIO_LOG_GUARD(FVazioLog::EVazioLogCategory::Mass, ::FVazioLog::LevelTokens::LevelToken, false, Format, ##__VA_ARGS__)
#define LOG_SYSTEMS(LevelToken, Format, ...) VAZIO_LOG_GUARD(FVazioLog::EVazioLogCategory::Systems, ::FVazioLog::LevelTokens::LevelToken, false, Format, ##__VA_ARGS__)
#define LOG_NETWORK(LevelToken, Format, ...) VAZIO_LOG_GUARD(FVazioLog::EVazioLogCategory::Network, ::FVazioLog::LevelTokens::LevelToken, false, Format, ##__VA_ARGS__)

#define VAZIO_LOG_LOOP_THROTTLED(LevelToken, KeyName, EveryNth, MinSeconds, Format, ...) \
	do \
	{ \
	const FVazioLog::EVazioLogLevel VazioResolvedLevel = ::FVazioLog::LevelTokens::LevelToken; \
		if (FVazioLog::FVazioLogFacade::ShouldLog(FVazioLog::EVazioLogCategory::Loops, VazioResolvedLevel, true) && \
			FVazioLog::FVazioLogFacade::ShouldEmitLoop(KeyName, EveryNth, MinSeconds)) \
		{ \
			FVazioLog::FVazioLogFacade::Log(FVazioLog::EVazioLogCategory::Loops, VazioResolvedLevel, true, Format, ##__VA_ARGS__); \
		} \
	} while (0)

#define LOG_LOOP(LevelEnum, Format, ...) VAZIO_LOG_LOOP_THROTTLED(LevelEnum, FVazioLog::MakeLoopKey(__FUNCTION__), 60, 0.0f, Format, ##__VA_ARGS__)
#define LOG_LOOP_KEY(LevelEnum, KeyName, Format, ...) VAZIO_LOG_LOOP_THROTTLED(LevelEnum, KeyName, 60, 0.0f, Format, ##__VA_ARGS__)
#define LOG_LOOP_EVERY(LevelEnum, KeyName, EveryNth, Format, ...) VAZIO_LOG_LOOP_THROTTLED(LevelEnum, KeyName, EveryNth, 0.0f, Format, ##__VA_ARGS__)
#define LOG_LOOP_THROTTLE(LevelEnum, KeyName, EveryNth, MinSeconds, Format, ...) VAZIO_LOG_LOOP_THROTTLED(LevelEnum, KeyName, EveryNth, MinSeconds, Format, ##__VA_ARGS__)
