// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "FlecsNetShardBase.h"
#include "FlecsNetDontFragmentTableArray.h"
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
	virtual void GetLifetimeReplicatedProps(TArray<class FLifetimeProperty>& OutLifetimeProps) const override;
	virtual void ConfigureObjectSettings(UE::Net::FRootObjectSettings& OutSettings) const override;
	
	void HandleReplicationDetached();
	void HandleEntityRemoved(const FFlecsNetworkId InNetworkId, uint32 InStateRevision);
	void HandleEntityUpdated(const FFlecsNetworkId InNetworkId, const FFlecsDontFragmentReplicationSnapshot& InSnapshot);
	
protected:
	
	UPROPERTY(Replicated)
	FFlecsReplicationKey DontFragmentKey;
	
	UPROPERTY(Replicated)
	FFlecsNetDontFragmentEntityTableArray DontFragmentTable;
	
}; // class UFlecsDontFragmentTable
