// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Properties/FlecsComponentProperties.h"
#include "Serialization/Archive.h"
#include "UObject/Object.h"

#include "FlecsNetworkingTestTypes.generated.h"

USTRUCT()
struct FFlecsReplicationTestValue
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestValue> : TFlecsComponentTraitsBase<FFlecsReplicationTestValue>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool Replicate = true;
};

USTRUCT()
struct FFlecsReplicationTestDontFragmentValue
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestDontFragmentValue>
	: TFlecsComponentTraitsBase<FFlecsReplicationTestDontFragmentValue>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool DontFragment = true;
	static constexpr bool Replicate = true;
}; // struct TFlecsComponentTraits<FFlecsReplicationTestDontFragmentValue>

struct FFlecsReplicationTestNativeValue
{
	int32 Value = 0;
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestNativeValue> : TFlecsComponentTraitsBase<FFlecsReplicationTestNativeValue>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool Replicate = true;
};

template <>
struct TFlecsReplicationTraits<FFlecsReplicationTestNativeValue>
{
	static FString StableSymbolName()
	{
		return TEXT("FFlecsReplicationTestNativeValue");
	}

	static bool Serialize(FArchive& Archive, FFlecsReplicationTestNativeValue& Value)
	{
		Archive << Value.Value;
		return !Archive.IsError();
	}
};

USTRUCT()
struct FFlecsReplicationTestTag
{
	GENERATED_BODY()
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestTag> : TFlecsComponentTraitsBase<FFlecsReplicationTestTag>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool Replicate = true;
};

USTRUCT()
struct FFlecsReplicationTestRequiredTag
{
	GENERATED_BODY()
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestRequiredTag> : TFlecsComponentTraitsBase<FFlecsReplicationTestRequiredTag>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool Replicate = true;
};

USTRUCT()
struct FFlecsReplicationTestWithValue
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestWithValue> : TFlecsComponentTraitsBase<FFlecsReplicationTestWithValue>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool Replicate = true;
	using WithTypes = TTuple<FFlecsReplicationTestRequiredTag>;
};

USTRUCT()
struct FFlecsReplicationTestRelationship
{
	GENERATED_BODY()
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestRelationship> : TFlecsComponentTraitsBase<FFlecsReplicationTestRelationship>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool Replicate = true;
	static constexpr bool Relationship = true;
};

USTRUCT()
struct FFlecsReplicationTestValueRelationship
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Value = 0;
};

template <>
struct TFlecsComponentTraits<FFlecsReplicationTestValueRelationship>
	: TFlecsComponentTraitsBase<FFlecsReplicationTestValueRelationship>
{
	static constexpr bool AutoRegister = false;
	static constexpr bool Replicate = true;
	static constexpr bool Relationship = true;
};
