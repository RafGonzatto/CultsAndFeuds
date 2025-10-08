#pragma once

#include "CoreMinimal.h"
#include "Logging/VazioLogTypes.h"

namespace FVazioLog
{
	struct FVazioLogRuntimeConfig
	{
		bool bGlobalEnabled = false;
		uint32 CategoryMask = 0u;
		EVazioLogLevel MinLevel = EVazioLogLevel::Info;
	};

	class VAZIOSHARED_API FVazioLogConfigLoader
	{
	public:
		FVazioLogConfigLoader();

		bool Load(FVazioLogRuntimeConfig& OutConfig) const;

	private:
		bool TryReadFile(TArray<FString>& OutLines) const;
		static bool ParseBool(const FString& InValue, bool& OutBool);
		static uint32 ApplyCategory(uint32 CurrentMask, EVazioLogCategory Category, bool bEnabled);

	private:
		FString EnvPath;
	};

	uint32 CategoryToBit(EVazioLogCategory Category);
}
