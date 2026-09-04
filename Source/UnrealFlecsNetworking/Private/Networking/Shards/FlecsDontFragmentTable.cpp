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
