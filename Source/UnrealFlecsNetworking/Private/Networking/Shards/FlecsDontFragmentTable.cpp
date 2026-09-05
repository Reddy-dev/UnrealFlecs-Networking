// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsDontFragmentTable.h"

#include "Net/UnrealNetwork.h"
#include "Networking/Shards/FlecsNetDontFragmentTableNetFactory.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDontFragmentTable)

void UFlecsDontFragmentTable::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsDontFragmentTable, DontFragmentKey, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsDontFragmentTable, DontFragmentTable, SharedParams);
}

void UFlecsDontFragmentTable::ConfigureObjectSettings(UE::Net::FRootObjectSettings& OutSettings) const
{
	Super::ConfigureObjectSettings(OutSettings);
	
	OutSettings.FactoryName = UFlecsNetDontFragmentTableNetFactory::GetFactoryName();
	OutSettings.bIsAlwaysRelevant = true; // Tables must always be relevant
	OutSettings.bIsNotRouted = false;
}

void UFlecsDontFragmentTable::InitializeDontFragmentTable(const FFlecsReplicationKey& InReplicationKey)
{
	InitializeShard(FFlecsEntityView::GetNullHandle());
	
	DontFragmentKey = InReplicationKey;
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsDontFragmentTable, DontFragmentKey, this);
}

// @TODO: tf we do here?
void UFlecsDontFragmentTable::HandleReplicationDetached()
{
}

void UFlecsDontFragmentTable::HandleEntityRemoved(const FFlecsNetworkId InNetworkId, uint32 InStateRevision)
{
	
}

void UFlecsDontFragmentTable::HandleEntityUpdated(const FFlecsNetworkId InNetworkId,
	const FFlecsDontFragmentReplicationSnapshot& InSnapshot)
{
	
}

void UFlecsDontFragmentTable::PublishDontFragmentNetEntity(const FFlecsNetworkId& InNetworkId,
	const FFlecsDontFragmentReplicationSnapshot& InSnapshot)
{
	const int32 ExistingIndex = DontFragmentTable.Items
		.IndexOfByPredicate([&InNetworkId](const FFlecsNetDontFragmentEntityTableItem& InItem)
		{
			return InItem.NetworkId == InNetworkId;
		});
	
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsDontFragmentTable, DontFragmentTable, this);
	
	if (ExistingIndex != INDEX_NONE)
	{
		DontFragmentTable.Items[ExistingIndex].Snapshot = InSnapshot;
	}
	else
	{
		FFlecsNetDontFragmentEntityTableItem& Item = DontFragmentTable.Items.Add_GetRef(FFlecsNetDontFragmentEntityTableItem());
		Item.NetworkId = InNetworkId;
		Item.Snapshot = InSnapshot;
		
		DontFragmentTable.MarkArrayDirty();
	}
}

void UFlecsDontFragmentTable::RemoveNetEntity(const FFlecsNetworkId& InNetworkId, const bool bInIsBeingDestroyed)
{
	DontFragmentTable.Items.RemoveAll([&InNetworkId](const FFlecsNetDontFragmentEntityTableItem& InItem)
	{
		return InItem.NetworkId == InNetworkId;
	});
	
	DontFragmentTable.MarkArrayDirty();
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsDontFragmentTable, DontFragmentTable, this);
}

bool UFlecsDontFragmentTable::IsEmpty() const
{
	return DontFragmentTable.Items.IsEmpty();
}

bool UFlecsDontFragmentTable::HasEntity(const FFlecsNetworkId InNetworkId) const
{
	return DontFragmentTable.Items.ContainsByPredicate([&InNetworkId](const FFlecsNetDontFragmentEntityTableItem& InItem)
	{
		return InItem.NetworkId == InNetworkId;
	});
}
