// Fill out your copyright notice in the Description page of Project Settings.

#include "Networking/Shards/FlecsNetDontFragmentTableNetFactory.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetDontFragmentTableNetFactory)

FName UFlecsNetDontFragmentTableNetFactory::GetFactoryName()
{
	static const FName FactoryName(TEXT("FlecsNetDontFragmentTableNetFactory"));
	return FactoryName;
}

UNetObjectFactory::FInstantiateResult UFlecsNetDontFragmentTableNetFactory::InstantiateReplicatedObjectFromHeader(
	const FInstantiateContext& Context, const UE::Net::FNetObjectCreationHeader* Header)
{
	return Super::InstantiateReplicatedObjectFromHeader(Context, Header);
}

void UFlecsNetDontFragmentTableNetFactory::DetachedFromReplication(const FDetachContext& Context,
	const TOptional<FSubObjectDetachContext>& SubObjectContext)
{
	Super::DetachedFromReplication(Context, SubObjectContext);
}

void UFlecsNetDontFragmentTableNetFactory::FillRootObjectReplicationParams(
	const UE::Net::FRootObjectReplicationParamsContext& Context, UE::Net::FRootObjectReplicationParams& OutParams)
{
	Super::FillRootObjectReplicationParams(Context, OutParams);
}
