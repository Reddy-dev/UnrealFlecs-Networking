// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "FlecsNetShardBase.h"
#include "FlecsNetDontFragmentTableArray.h"
#include "Networking/FlecsDontFragmentReplicationUpdateQueue.h"
#include "Networking/Layout/FlecsDontFragmentReplicationSnapshot.h"

#include "FlecsDontFragmentTable.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsDontFragmentTable : public UFlecsNetShardBase
{
	GENERATED_BODY()
	
public:
	virtual void PostInitProperties() override;
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ConfigureObjectSettings(UE::Net::FRootObjectSettings& OutSettings) const override;
	
	void InitializeDontFragmentTable(const FFlecsReplicationKey& InReplicationKey);
	
	void HandleReplicationDetached();
	void HandleEntityRemoved(const FFlecsNetworkId InNetworkId, uint32 InStateRevision);
	void HandleEntityUpdated(const FFlecsNetworkId InNetworkId, const FFlecsDontFragmentReplicationSnapshot& InSnapshot);
	
	virtual bool CanAcceptNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot) const override
	{
		return true;
	}
	
	// Use the custom function instead
	virtual void PublishNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsEntityReplicationSnapshot& InSnapshot) override
	{
		unimplemented();
	}
	
	void PublishDontFragmentNetEntity(const FFlecsNetworkId& InNetworkId, const FFlecsDontFragmentReplicationSnapshot& InSnapshot);
	
	virtual void RemoveNetEntity(const FFlecsNetworkId& InNetworkId, const bool bInIsBeingDestroyed) override;
	
	virtual bool IsEmpty() const override;
	
	NO_DISCARD bool HasEntity(const FFlecsNetworkId InNetworkId) const;
	
protected:
	virtual void FlushPendingReplicationUpdates() override;
	
	UPROPERTY(ReplicatedUsing = OnRep_DontFragmentKey)
	FFlecsReplicationKey DontFragmentKey;

	UFUNCTION()
	void OnRep_DontFragmentKey();
	
	UPROPERTY(Replicated)
	FFlecsNetDontFragmentEntityTableArray DontFragmentTable;

	FFlecsDontFragmentReplicationUpdateQueue PendingDontFragmentReplicationUpdateQueue;
	
}; // class UFlecsDontFragmentTable
