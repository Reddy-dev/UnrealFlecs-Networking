// Fill out your copyright notice in the Description page of Project Settings.

#include "Networking/Shards/FlecsNetDontFragmentTableNetFactory.h"

#include "Engine/World.h"
#include "Iris/ReplicationSystem/ObjectReplicationBridge.h"

#include "Networking/Bridge/FlecsIrisReplicationWorldResolver.h"
#include "Networking/Shards/FlecsDontFragmentTable.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetDontFragmentTableNetFactory)

FName UFlecsNetDontFragmentTableNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsNetDontFragmentTableNetFactory"));
	return FactoryName;
}

UNetObjectFactory::FInstantiateResult UFlecsNetDontFragmentTableNetFactory::InstantiateReplicatedObjectFromHeader(
	const FInstantiateContext& Context, const UE::Net::FNetObjectCreationHeader* Header)
{
	FInstantiateResult Result = Super::InstantiateReplicatedObjectFromHeader(Context, Header);
	if UNLIKELY_IF(!Result.Instance)
	{
		return Result;
	}

	const TSolidNotNull<UFlecsDontFragmentTable*> Table =
		CastChecked<UFlecsDontFragmentTable>(Result.Instance);
	UWorld* World = UE::Flecs::Replication::GetReplicationBridgeWorld(Bridge);
	if UNLIKELY_IF(!World)
	{
		Result.Instance = nullptr;
		Result.Template = nullptr;
		Result.FailureDiagnosticMessage = TEXT("The receiving replication bridge does not have a valid UWorld");
		return Result;
	}

	Table->SetOwningWorld(World);
	return Result;
}

void UFlecsNetDontFragmentTableNetFactory::DetachedFromReplication(const FDetachContext& Context,
	const TOptional<FSubObjectDetachContext>& SubObjectContext)
{
	if (UFlecsDontFragmentTable* Table = Cast<UFlecsDontFragmentTable>(Context.DetachedInstance))
	{
		Table->HandleReplicationDetached();

		if (Context.Reason != UE::Net::EDetachReason::TornOff)
		{
			Table->SetOwningNetworkWorldSubsystem(nullptr);
		}
	}

	Super::DetachedFromReplication(Context, SubObjectContext);
}

void UFlecsNetDontFragmentTableNetFactory::FillRootObjectReplicationParams(
	const UE::Net::FRootObjectReplicationParamsContext& Context, UE::Net::FRootObjectReplicationParams& OutParams)
{
	Super::FillRootObjectReplicationParams(Context, OutParams);
}
