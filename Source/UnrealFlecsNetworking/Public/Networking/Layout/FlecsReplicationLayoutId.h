// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "SolidMacros/Macros.h"

#include "FlecsReplicationLayoutId.generated.h"

/** Deterministic identity of a complete replicated component/pair composition. */
USTRUCT(BlueprintType)
struct UNREALFLECSNETWORKING_API FFlecsReplicationLayoutId
{
	GENERATED_BODY()
	
	friend uint32 GetTypeHash(const FFlecsReplicationLayoutId& Id)
	{
		return GetTypeHash(Id.Value);
	}
	
public:

	FFlecsReplicationLayoutId() = default;
	explicit FFlecsReplicationLayoutId(const FGuid& InValue) 
		: Value(InValue)
	{
	}
	
	NO_DISCARD bool IsValid() const
	{
		return Value.IsValid();
	}
	
	NO_DISCARD FString ToString() const
	{
		return Value.ToString(EGuidFormats::DigitsWithHyphensLower);
	}
	
	NO_DISCARD FORCEINLINE bool UEOpEquals(const FFlecsReplicationLayoutId& Other) const
	{
		return Value == Other.Value;
	}

	UPROPERTY()
	FGuid Value;
	
}; // struct FFlecsReplicationLayoutId