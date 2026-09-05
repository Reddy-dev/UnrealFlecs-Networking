// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Networking/Bridge/FlecsReplicationBridgeBase.h"
#include "Networking/FlecsReplicationKey.h"
#include "Networking/Layout/FlecsDontFragmentReplicationSnapshot.h"

#include "FlecsTestReplicationBridge.generated.h"

struct FFlecsTestDontFragmentComponentPublication
{
	FFlecsNetworkId NetworkId;
	FFlecsReplicationKey ReplicationKey;
	FFlecsDontFragmentReplicationSnapshot Snapshot;
}; // struct FFlecsTestDontFragmentComponentPublication

struct FFlecsTestDontFragmentComponentRemoval
{
	FFlecsNetworkId NetworkId;
	FFlecsReplicationKey ReplicationKey;
}; // struct FFlecsTestDontFragmentComponentRemoval

UCLASS()
class UNREALFLECSNETWORKINGTESTS_API UFlecsTestReplicationBridge : public UFlecsReplicationBridgeBase
{
	GENERATED_BODY()

public:
	virtual void InitializeBridge() override;
	virtual void DeinitializeBridge() override;

	virtual void PublishEntityLayout(const FFlecsReplicationLayoutDefinition& InLayoutDefinition) override;
	virtual void PublishNetEntity(
		const FFlecsEntityHandle& InEntityHandle,
		const FFlecsNetworkId InNetworkId,
		const FFlecsEntityReplicationSnapshot& InSnapshot) override;
	virtual void PublishDontFragmentComponent(const FFlecsNetworkId InNetworkId,
		const FFlecsReplicationKey& InReplicationKey,
		const FFlecsDontFragmentReplicationSnapshot& InSnapshot) override;
	virtual void RemoveDontFragmentComponent(const FFlecsNetworkId InNetworkId,
		const FFlecsReplicationKey& InReplicationKey) override;
	virtual NO_DISCARD UFlecsNetShardBase* ResolveShard(
		const FFlecsEntityHandle&,
		const FFlecsNetworkId,
		const FFlecsEntityReplicationSnapshot&) override
	{
		return nullptr;
	}

	void SetPeer(UFlecsTestReplicationBridge* InPeer);
	void ResetCapturedRecords();

	NO_DISCARD bool IsInitialized() const
	{
		return bInitialized;
	}

	NO_DISCARD const TArray<FFlecsReplicationLayoutDefinition>& GetPublishedLayouts() const
	{
		return PublishedLayouts;
	}

	NO_DISCARD const TArray<TPair<FFlecsNetworkId, FFlecsEntityReplicationSnapshot>>& GetPublishedSnapshots() const
	{
		return PublishedSnapshots;
	}

	NO_DISCARD const TArray<FFlecsTestDontFragmentComponentPublication>& GetPublishedDontFragmentComponents() const
	{
		return PublishedDontFragmentComponents;
	}

	NO_DISCARD const TArray<FFlecsTestDontFragmentComponentRemoval>& GetRemovedDontFragmentComponents() const
	{
		return RemovedDontFragmentComponents;
	}

private:
	UPROPERTY(Transient)
	TObjectPtr<UFlecsTestReplicationBridge> Peer = nullptr;

	UPROPERTY(Transient)
	TArray<FFlecsReplicationLayoutDefinition> PublishedLayouts;

	TArray<TPair<FFlecsNetworkId, FFlecsEntityReplicationSnapshot>> PublishedSnapshots;
	TArray<FFlecsTestDontFragmentComponentPublication> PublishedDontFragmentComponents;
	TArray<FFlecsTestDontFragmentComponentRemoval> RemovedDontFragmentComponents;

	bool bInitialized = false;
}; // class UFlecsTestReplicationBridge
