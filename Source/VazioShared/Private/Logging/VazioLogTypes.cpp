#include "Logging/VazioLogTypes.h"

namespace
{
	constexpr const TCHAR* LevelNames[] = {
		TEXT("TRACE"),
		TEXT("DEBUG"),
		TEXT("INFO"),
		TEXT("WARN"),
		TEXT("ERROR")
	};

	constexpr const TCHAR* CategoryNames[] = {
		TEXT("UI"),
		TEXT("ENEMIES"),
		TEXT("WEAPONS"),
		TEXT("UPGRADES"),
		TEXT("MASS"),
		TEXT("SYSTEMS"),
		TEXT("NETWORK"),
		TEXT("LOOPS")
	};
}

namespace FVazioLog
{
	const TCHAR* ToString(EVazioLogLevel Level)
	{
		const int32 Index = static_cast<int32>(Level);
		return LevelNames[FMath::Clamp(Index, 0, UE_ARRAY_COUNT(LevelNames) - 1)];
	}

	const TCHAR* ToString(EVazioLogCategory Category)
	{
		const int32 Index = static_cast<int32>(Category);
		return CategoryNames[FMath::Clamp(Index, 0, UE_ARRAY_COUNT(CategoryNames) - 1)];
	}

	EVazioLogLevel LevelFromString(const FString& InValue, EVazioLogLevel DefaultLevel)
	{
		const FString UpperValue = InValue.ToUpper();
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(LevelNames); ++Index)
		{
			if (UpperValue == LevelNames[Index])
			{
				return static_cast<EVazioLogLevel>(Index);
			}
		}
		return DefaultLevel;
	}
}
