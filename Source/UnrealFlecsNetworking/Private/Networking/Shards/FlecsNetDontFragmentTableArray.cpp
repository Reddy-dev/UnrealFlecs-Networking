// Fill out your copyright notice in the Description page of Project Settings.

#include "Networking/Shards/FlecsNetDontFragmentTableArray.h"

#include "Networking/Shards/FlecsDontFragmentTable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetDontFragmentTableArray)

void FFlecsNetDontFragmentEntityTableArray::SetOwner(const TSolidNotNull<UFlecsDontFragmentTable*> InOwner)
{
	solid_check(IsValid(InOwner));
	Owner = InOwner;
}

bool FFlecsNetDontFragmentEntityTableArray::NetDeltaSerialize(FNetDeltaSerializeInfo& DeltaParms)
{
	return FastArrayDeltaSerialize(Items, DeltaParms, *this);
}

void FFlecsNetDontFragmentEntityTableArray::PreReplicatedRemove(const TArrayView<int32>& RemovedIndices, int32 FinalSize)
{
	UFlecsDontFragmentTable* Table = Owner.Get();
	if UNLIKELY_IF(!Table)
	{
		return;
	}

	for (const int32 RemovedIndex : RemovedIndices)
	{
		if (!Items.IsValidIndex(RemovedIndex))
		{
			continue;
		}

		const FFlecsNetDontFragmentEntityTableItem& Item = Items[RemovedIndex];
		Table->HandleEntityRemoved(Item.NetworkId, Item.Snapshot.StateRevision);
	}
}

void FFlecsNetDontFragmentEntityTableArray::PostReplicatedAdd(const TArrayView<int32>& AddedIndices, int32 FinalSize)
{
	PostReplicatedChange(AddedIndices, FinalSize);
}

void FFlecsNetDontFragmentEntityTableArray::PostReplicatedChange(const TArrayView<int32>& ChangedIndices, int32 FinalSize)
{
	UFlecsDontFragmentTable* Table = Owner.Get();
	if UNLIKELY_IF(!Table)
	{
		return;
	}

	for (const int32 ChangedIndex : ChangedIndices)
	{
		if (!Items.IsValidIndex(ChangedIndex))
		{
			continue;
		}

		const FFlecsNetDontFragmentEntityTableItem& Item = Items[ChangedIndex];
		Table->HandleEntityUpdated(Item.NetworkId, Item.Snapshot);
	}
}
