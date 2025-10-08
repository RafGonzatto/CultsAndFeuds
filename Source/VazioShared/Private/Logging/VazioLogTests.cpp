#include "Logging/VazioLogFacade.h"

#if WITH_DEV_AUTOMATION_TESTS

#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "Misc/ScopeExit.h"
#include "Misc/ScopeLock.h"
#include "HAL/FileManager.h"
#include "HAL/CriticalSection.h"
#include "HAL/PlatformProcess.h"
#include "Templates/UniquePtr.h"

using namespace FVazioLog;

namespace
{
	class FSpySink final : public IVazioLogSink
	{
	public:
		virtual void Emit(const FString& Line, EVazioLogLevel /*Level*/) override
		{
			FScopeLock Lock(&Mutex);
			Lines.Add(Line);
		}

		int32 Count() const
		{
			FScopeLock Lock(&Mutex);
			return Lines.Num();
		}

		FString Last() const
		{
			FScopeLock Lock(&Mutex);
			return Lines.Num() > 0 ? Lines.Last() : FString();
		}

		void Reset()
		{
			FScopeLock Lock(&Mutex);
			Lines.Reset();
		}

	private:
		mutable FCriticalSection Mutex;
		TArray<FString> Lines;
	};

	void WriteEnv(const FString& Content)
	{
		const FString EnvPath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".env"));
		ensureAlwaysMsgf(FFileHelper::SaveStringToFile(Content, *EnvPath), TEXT("Failed to write .env for logging tests"));
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FVazioLoggingSystemTest, "Vazio.Logging.System", EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)

bool FVazioLoggingSystemTest::RunTest(const FString& Parameters)
{
	FVazioLogFacade::Initialize();

	const FString EnvPath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".env"));
	const bool bHadEnv = FPaths::FileExists(EnvPath);
	FString OriginalEnv;
	if (bHadEnv)
	{
		FFileHelper::LoadFileToString(OriginalEnv, *EnvPath);
	}

	ON_SCOPE_EXIT
	{
		if (bHadEnv)
		{
			FFileHelper::SaveStringToFile(OriginalEnv, *EnvPath);
		}
		else
		{
			IFileManager::Get().Delete(*EnvPath);
		}
		FVazioLogFacade::ReloadConfig();
	};

	WriteEnv(TEXT("LOG_ENABLED=true\nLOG_ENEMIES=true\nLOG_LOOPS=true\n"));
	TestTrue(TEXT("Reload config"), FVazioLogFacade::ReloadConfig());

	TestTrue(TEXT("Enemies enabled"), FVazioLogFacade::IsEnabled(EVazioLogCategory::Enemies));
	TestTrue(TEXT("Loops enabled"), FVazioLogFacade::IsEnabled(EVazioLogCategory::Loops));
	TestFalse(TEXT("UI disabled"), FVazioLogFacade::IsEnabled(EVazioLogCategory::UI));

	TUniquePtr<FSpySink> Spy = MakeUnique<FSpySink>();
	FSpySink* SpyRaw = Spy.Get();
	FVazioLogFacade::SetSink(MoveTemp(Spy));

	LOG_ENEMIES(Info, TEXT("Enemy %d"), 1);
	LOG_UI(Info, TEXT("Should stay silent"));
	TestEqual(TEXT("Only one log emitted"), SpyRaw->Count(), 1);

	WriteEnv(TEXT("LOG_ENABLED=false\nLOG_ENEMIES=true\n"));
	TestTrue(TEXT("Reload disabled config"), FVazioLogFacade::ReloadConfig());

	SpyRaw->Reset();
	LOG_ENEMIES(Info, TEXT("No output expected"));
	TestEqual(TEXT("Global disable silences"), SpyRaw->Count(), 0);

	WriteEnv(TEXT("LOG_ENABLED=true\nLOG_ENEMIES=true\nLOG_LEVEL=ERROR\n"));
	TestTrue(TEXT("Reload error level"), FVazioLogFacade::ReloadConfig());

	SpyRaw->Reset();
	LOG_ENEMIES(Info, TEXT("Filtered"));
	LOG_ENEMIES(Error, TEXT("Raised"));
	TestEqual(TEXT("Level gating"), SpyRaw->Count(), 1);
	TestTrue(TEXT("Error level recorded"), SpyRaw->Last().Contains(TEXT("[ERROR]")));

	SpyRaw->Reset();
	const FName LoopKey(TEXT("LoopKey"));
	for (int32 Index = 0; Index < 10; ++Index)
	{
		LOG_LOOP_EVERY(Debug, LoopKey, 5, TEXT("Loop %d"), Index);
	}
	TestEqual(TEXT("Loop throttle count"), SpyRaw->Count(), 3);

	SpyRaw->Reset();
	for (int32 Index = 0; Index < 3; ++Index)
	{
		LOG_LOOP_THROTTLE(Debug, FName(TEXT("TimedLoop")), 1, 0.5f, TEXT("Tick %d"), Index);
		if (Index == 0)
		{
			TestEqual(TEXT("First timed emit"), SpyRaw->Count(), 1);
		}
		else
		{
			TestEqual(TEXT("No extra emissions before window"), SpyRaw->Count(), 1);
		}
		FPlatformProcess::Sleep(0.2f);
	}
	FPlatformProcess::Sleep(0.6f);
	LOG_LOOP_THROTTLE(Debug, FName(TEXT("TimedLoop")), 1, 0.5f, TEXT("Tick late"));
	TestEqual(TEXT("Emission after window"), SpyRaw->Count(), 2);

	SpyRaw->Reset();
	WriteEnv(TEXT("LOG_ENABLED=true\nLOG_UI=true\n"));
	TestTrue(TEXT("Reload hot config"), FVazioLogFacade::ReloadConfig());
	LOG_UI(Info, TEXT("Hot swap"));
	TestEqual(TEXT("Hot reload takes effect"), SpyRaw->Count(), 1);

	FVazioLogFacade::SetSink(TUniquePtr<IVazioLogSink>());

	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
