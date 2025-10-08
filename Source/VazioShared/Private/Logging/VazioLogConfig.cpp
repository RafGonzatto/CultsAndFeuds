#include "Logging/VazioLogConfig.h"

#include "Misc/FileHelper.h"
#include "Misc/Paths.h"

namespace FVazioLog
{
	namespace
	{
		constexpr TCHAR LOG_ENABLED_KEY[] = TEXT("LOG_ENABLED");
		constexpr TCHAR LOG_LEVEL_KEY[] = TEXT("LOG_LEVEL");

		const TPair<const TCHAR*, EVazioLogCategory> CategoryPairs[] = {
			{ TEXT("LOG_UI"), EVazioLogCategory::UI },
			{ TEXT("LOG_ENEMIES"), EVazioLogCategory::Enemies },
			{ TEXT("LOG_WEAPONS"), EVazioLogCategory::Weapons },
			{ TEXT("LOG_UPGRADES"), EVazioLogCategory::Upgrades },
			{ TEXT("LOG_MASS"), EVazioLogCategory::Mass },
			{ TEXT("LOG_SYSTEMS"), EVazioLogCategory::Systems },
			{ TEXT("LOG_NETWORK"), EVazioLogCategory::Network },
			{ TEXT("LOG_LOOPS"), EVazioLogCategory::Loops }
		};

		FString ExtractKey(const FString& Line)
		{
			FString Key;
			FString Value;
			if (Line.Split(TEXT("="), &Key, &Value))
			{
				return Key.TrimStartAndEnd();
			}
			return FString();
		}
	}

	FVazioLogConfigLoader::FVazioLogConfigLoader()
	{
		EnvPath = FPaths::Combine(FPaths::ProjectDir(), TEXT(".env"));
	}

	bool FVazioLogConfigLoader::Load(FVazioLogRuntimeConfig& OutConfig) const
	{
		OutConfig = FVazioLogRuntimeConfig{};

		TArray<FString> Lines;
		if (!TryReadFile(Lines))
		{
			return true; // Treat missing file as "all disabled"
		}

		bool bGlobalSet = false;
		for (const FString& RawLine : Lines)
		{
			const FString Line = RawLine.TrimStartAndEnd();
			if (Line.IsEmpty() || Line.StartsWith(TEXT("#")))
			{
				continue;
			}

			FString Key;
			FString Value;
			if (!Line.Split(TEXT("="), &Key, &Value))
			{
				continue;
			}

			Key = Key.TrimStartAndEnd();
			Value = Value.TrimStartAndEnd();

			if (Key.Equals(LOG_ENABLED_KEY, ESearchCase::IgnoreCase))
			{
				bool bEnabled = false;
				if (ParseBool(Value, bEnabled))
				{
					OutConfig.bGlobalEnabled = bEnabled;
					bGlobalSet = true;
				}
				continue;
			}

			if (Key.Equals(LOG_LEVEL_KEY, ESearchCase::IgnoreCase))
			{
				OutConfig.MinLevel = LevelFromString(Value, EVazioLogLevel::Info);
				continue;
			}

			for (const TPair<const TCHAR*, EVazioLogCategory>& Pair : CategoryPairs)
			{
				if (Key.Equals(Pair.Key, ESearchCase::IgnoreCase))
				{
					bool bFlag = false;
					if (ParseBool(Value, bFlag))
					{
						OutConfig.CategoryMask = ApplyCategory(OutConfig.CategoryMask, Pair.Value, bFlag);
					}
					break;
				}
			}
		}

		if (!bGlobalSet)
		{
			OutConfig.bGlobalEnabled = false;
		}

		return true;
	}

	bool FVazioLogConfigLoader::TryReadFile(TArray<FString>& OutLines) const
	{
		if (!FPaths::FileExists(EnvPath))
		{
			return false;
		}

		return FFileHelper::LoadFileToStringArray(OutLines, *EnvPath);
	}

	bool FVazioLogConfigLoader::ParseBool(const FString& InValue, bool& OutBool)
	{
		const FString Normalized = InValue.ToLower();
		if (Normalized == TEXT("true") || Normalized == TEXT("1") || Normalized == TEXT("yes") || Normalized == TEXT("on"))
		{
			OutBool = true;
			return true;
		}

		if (Normalized == TEXT("false") || Normalized == TEXT("0") || Normalized == TEXT("no") || Normalized == TEXT("off"))
		{
			OutBool = false;
			return true;
		}

		return false;
	}

	uint32 CategoryToBit(EVazioLogCategory Category)
	{
		return 1u << static_cast<uint8>(Category);
	}

	uint32 FVazioLogConfigLoader::ApplyCategory(uint32 CurrentMask, EVazioLogCategory Category, bool bEnabled)
	{
		const uint32 Bit = CategoryToBit(Category);
		return bEnabled ? (CurrentMask | Bit) : (CurrentMask & ~Bit);
	}
}
