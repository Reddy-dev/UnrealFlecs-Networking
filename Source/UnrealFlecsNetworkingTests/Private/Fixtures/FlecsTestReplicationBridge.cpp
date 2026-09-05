// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Fixtures/FlecsTestReplicationBridge.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsTestReplicationBridge)

void UFlecsTestReplicationBridge::InitializeBridge()
{
	bInitialized = true;
}

void UFlecsTestReplicationBridge::DeinitializeBridge()
{
	bInitialized = false;
	Peer = nullptr;
}

void UFlecsTestReplicationBridge::PublishEntityLayout(
	const FFlecsReplicationLayoutDefinition& InLayoutDefinition)
{
	PublishedLayouts.Add(InLayoutDefinition);
	Super::PublishEntityLayout(InLayoutDefinition);

	if (Peer)
	{
		Peer->ReceiveEntityLayout(InLayoutDefinition);
	}
}

void UFlecsTestReplicationBridge::PublishNetEntity(
	MAYBE_UNUSED const FFlecsEntityHandle& InEntityHandle,
	const FFlecsNetworkId InNetworkId,
	const FFlecsEntityReplicationSnapshot& InSnapshot)
{
	PublishedSnapshots.Emplace(InNetworkId, InSnapshot);

	if (Peer)
	{
		Peer->ReceiveNetEntity(InNetworkId, InSnapshot);
	}
}

void UFlecsTestReplicationBridge::PublishDontFragmentComponent(const FFlecsNetworkId InNetworkId,
	const FFlecsReplicationKey& InReplicationKey,
	const FFlecsDontFragmentReplicationSnapshot& InSnapshot)
{
	FFlecsTestDontFragmentComponentPublication& Publication =
		PublishedDontFragmentComponents.Emplace_GetRef();
	Publication.NetworkId = InNetworkId;
	Publication.ReplicationKey = InReplicationKey;
	Publication.Snapshot = InSnapshot;
}

void UFlecsTestReplicationBridge::RemoveDontFragmentComponent(const FFlecsNetworkId InNetworkId,
	const FFlecsReplicationKey& InReplicationKey)
{
	FFlecsTestDontFragmentComponentRemoval& Removal =
		RemovedDontFragmentComponents.Emplace_GetRef();
	Removal.NetworkId = InNetworkId;
	Removal.ReplicationKey = InReplicationKey;
}

void UFlecsTestReplicationBridge::SetPeer(UFlecsTestReplicationBridge* InPeer)
{
	Peer = InPeer;
}

void UFlecsTestReplicationBridge::ResetCapturedRecords()
{
	PublishedLayouts.Reset();
	PublishedSnapshots.Reset();
	PublishedDontFragmentComponents.Reset();
	RemovedDontFragmentComponents.Reset();
}
