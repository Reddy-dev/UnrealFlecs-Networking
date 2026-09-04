// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Iris/ReplicationState/IrisFastArraySerializer.h"
#include "Net/Serialization/FastArraySerializer.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/Layout/FlecsDontFragmentReplicationSnapshot.h"
#include "Networking/Layout/FlecsReplicationSnapshot.h"

#include "FlecsNetDontFragmentTableArray.generated.h"

class UFlecsDontFragmentTable;

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsNetDontFragmentEntityTableItem : public FFastArraySerializerItem
{
	GENERATED_BODY()

public:
	UPROPERTY()
	FFlecsNetworkId NetworkId;

	UPROPERTY()
	FFlecsDontFragmentReplicationSnapshot Snapshot;

}; // struct FFlecsNetDontFragmentEntityTableItem

USTRUCT()
struct UNREALFLECSNETWORKING_API FFlecsNetDontFragmentEntityTableArray : public FIrisFastArraySerializer
{
	GENERATED_BODY()

public:
	FFlecsNetDontFragmentEntityTableArray()
		: Owner(nullptr)
	{
	}

	void SetOwner(const TSolidNotNull<UFlecsDontFragmentTable*> InOwner);

	UPROPERTY()
	TArray<FFlecsNetDontFragmentEntityTableItem> Items;

	UPROPERTY(Transient, NotReplicated)
	TWeakObjectPtr<UFlecsDontFragmentTable> Owner;

	bool NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms);

	void PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize);
	void PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize);
	void PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize);

}; // struct FFlecsNetDontFragmentEntityTableArray

template<>
struct TStructOpsTypeTraits<FFlecsNetDontFragmentEntityTableArray> : public TStructOpsTypeTraitsBase2<FFlecsNetDontFragmentEntityTableArray>
{
	enum
	{
		WithNetDeltaSerializer = true
	};
	
}; // struct TStructOpsTypeTraits<FFlecsNetDontFragmentEntityTableArray>