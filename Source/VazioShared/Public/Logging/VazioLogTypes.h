#pragma once

#include "CoreMinimal.h"

namespace FVazioLog
{
	enum class EVazioLogLevel : uint8
	{
		Trace = 0,
		Debug,
		Info,
		Warn,
		Error
	};

	enum class EVazioLogCategory : uint8
	{
		UI = 0,
		Enemies,
		Weapons,
		Upgrades,
		Mass,
		Systems,
		Network,
		Loops,
		Count
	};

	VAZIOSHARED_API const TCHAR* ToString(EVazioLogLevel Level);
	VAZIOSHARED_API const TCHAR* ToString(EVazioLogCategory Category);
	VAZIOSHARED_API EVazioLogLevel LevelFromString(const FString& InValue, EVazioLogLevel DefaultLevel = EVazioLogLevel::Info);
}

using EVazioLogLevel = FVazioLog::EVazioLogLevel;
using EVazioLogCategory = FVazioLog::EVazioLogCategory;

namespace FVazioLog
{
	namespace LevelTokens
	{
		constexpr EVazioLogLevel Trace = EVazioLogLevel::Trace;
		constexpr EVazioLogLevel Debug = EVazioLogLevel::Debug;
		constexpr EVazioLogLevel Info = EVazioLogLevel::Info;
		constexpr EVazioLogLevel Warn = EVazioLogLevel::Warn;
		constexpr EVazioLogLevel Error = EVazioLogLevel::Error;
		constexpr EVazioLogLevel Verbose = EVazioLogLevel::Debug;
		constexpr EVazioLogLevel VeryVerbose = EVazioLogLevel::Trace;
		constexpr EVazioLogLevel Warning = EVazioLogLevel::Warn;
		constexpr EVazioLogLevel Display = EVazioLogLevel::Info;
	}
}
