// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/Shards/FlecsDontFragmentTable.h"

#include "Net/UnrealNetwork.h"

#include "Networking/Shards/FlecsNetDontFragmentTableNetFactory.h"
#include "Networking/Subsystem/FlecsNetworkWorldSubsystem.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsDontFragmentTable)

void UFlecsDontFragmentTable::PostInitProperties()
{
	Super::PostInitProperties();

	DontFragmentTable.SetOwner(this);
}

void UFlecsDontFragmentTable::GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const
{
	Super::GetLifetimeReplicatedProps(OutLifetimeProps);
	
	FDoRepLifetimeParams SharedParams;
	SharedParams.bIsPushBased = true;
	SharedParams.RepNotifyCondition = REPNOTIFY_Always;
	
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsDontFragmentTable, DontFragmentKey, SharedParams);
	DOREPLIFETIME_WITH_PARAMS_FAST(UFlecsDontFragmentTable, DontFragmentTable, SharedParams);
}

void UFlecsDontFragmentTable::ConfigureObjectSettings(UE::Net::FRootObjectSettings& OutSettings) const
{
	//Super::ConfigureObjectSettings(OutSettings);
	
	OutSettings.FactoryName = UFlecsNetDontFragmentTableNetFactory::GetFactoryName();
	OutSettings.bIsAlwaysRelevant = true; // Tables must always be relevant
	OutSettings.bIsNotRouted = false;
}

void UFlecsDontFragmentTable::InitializeDontFragmentTable(const FFlecsReplicationKey& InReplicationKey)
{
	bShouldUseReplicationProfile = false;
	
	InitializeShard(FFlecsEntityView::GetNullHandle());
	
	DontFragmentKey = InReplicationKey;
	MARK_PROPERTY_DIRTY_FROM_NAME(UFlecsDontFragmentTable, DontFragmentKey, this);
}

void UFlecsDontFragmentTable::HandleReplicationDetached()
{
	for (const FFlecsNetDontFragmentEntityTableItem& Item : DontFragmentTable.Items)
	{
		HandleEntityRemoved(Item.NetworkId, Item.Snapshot.StateRevision);
	}
}

void UFlecsDontFragmentTable::HandleEntityRemoved(const FFlecsNetworkId InNetworkId, uint32 InStateRevision)
{
	if (!InNetworkId.IsValid())
	{
		return;
	}
	
	ResolveOwningNetworkWorldSubsystem();
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	
	if (!NetworkSubsystem)
	{
		PendingDontFragmentReplicationUpdateQueue.EnqueueRemoval(
			InNetworkId, DontFragmentKey, InStateRevision);
		return;
	}

	NetworkSubsystem->RemoveReceivedNetworkDontFragmentComponent(
		InNetworkId, DontFragmentKey, InStateRevision);
}

void UFlecsDontFragmentTable::HandleEntityUpdated(const FFlecsNetworkId InNetworkId,
	const FFlecsDontFragmentReplicationSnapshot& InSnapshot)
{
	if (!InNetworkId.IsValid())
	{
		return;
	}

	ResolveOwningNetworkWorldSubsystem();
	
	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	if UNLIKELY_IF(!NetworkSubsystem)
	{
		PendingDontFragmentReplicationUpdateQueue.EnqueueSnapshot(
			InNetworkId, DontFragmentKey, InSnapshot);
		return;
	}

	NetworkSubsystem->ReceiveNetworkDontFragmentSnapshot(InNetworkId, DontFragmentKey, InSnapshot);
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
		DontFragmentTable.MarkItemDirty(DontFragmentTable.Items[ExistingIndex]);
	}
	else
	{
		FFlecsNetDontFragmentEntityTableItem& Item = DontFragmentTable.Items.Add_GetRef(FFlecsNetDontFragmentEntityTableItem());
		Item.NetworkId = InNetworkId;
		Item.Snapshot = InSnapshot;
		
		DontFragmentTable.MarkArrayDirty();
	}
}

void UFlecsDontFragmentTable::OnRep_DontFragmentKey()
{
	for (const FFlecsNetDontFragmentEntityTableItem& Item : DontFragmentTable.Items)
	{
		HandleEntityUpdated(Item.NetworkId, Item.Snapshot);
	}
}

void UFlecsDontFragmentTable::FlushPendingReplicationUpdates()
{
	Super::FlushPendingReplicationUpdates();

	UFlecsNetworkWorldSubsystem* NetworkSubsystem = GetOwningNetworkWorldSubsystem();
	if UNLIKELY_IF(!NetworkSubsystem)
	{
		return;
	}

	const TArray<FFlecsDontFragmentReplicationQueuedUpdate> Updates =
		PendingDontFragmentReplicationUpdateQueue.Drain();
	for (const FFlecsDontFragmentReplicationQueuedUpdate& Update : Updates)
	{
		if (Update.bRemove)
		{
			NetworkSubsystem->RemoveReceivedNetworkDontFragmentComponent(
				Update.NetworkId, Update.ReplicationKey, Update.StateRevision);
		}
		else
		{
			NetworkSubsystem->ReceiveNetworkDontFragmentSnapshot(
				Update.NetworkId, Update.ReplicationKey, Update.Snapshot);
		}
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
