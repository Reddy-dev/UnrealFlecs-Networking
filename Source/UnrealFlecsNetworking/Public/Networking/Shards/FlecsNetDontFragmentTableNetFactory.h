// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Net/Iris/ReplicationSystem/NetRootObjectFactory.h"

#include "FlecsNetDontFragmentTableNetFactory.generated.h"

/**
 * 
 */
UCLASS()
class UNREALFLECSNETWORKING_API UFlecsNetDontFragmentTableNetFactory : public UNetRootObjectFactory
{
	GENERATED_BODY()

public:
	static FName GetFactoryName();
	
protected:
	virtual FInstantiateResult InstantiateReplicatedObjectFromHeader(
		const FInstantiateContext& Context,
		const UE::Net::FNetObjectCreationHeader* Header) override;

	virtual void DetachedFromReplication(
		const FDetachContext& Context,
		const TOptional<FSubObjectDetachContext>& SubObjectContext) override;
	
	virtual void FillRootObjectReplicationParams(const UE::Net::FRootObjectReplicationParamsContext& Context,
		UE::Net::FRootObjectReplicationParams& OutParams) override;
	
}; // class UFlecsNetDontFragmentTableNetFactory
