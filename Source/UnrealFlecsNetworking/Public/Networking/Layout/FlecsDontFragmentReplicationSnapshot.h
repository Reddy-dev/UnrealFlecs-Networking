// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "FlecsDontFragmentReplicationSnapshot.generated.h"

USTRUCT()
struct FFlecsDontFragmentReplicationSnapshot
{
	GENERATED_BODY()
	
public:
	UPROPERTY()
	TArray<uint8> SnapshotData;
	
	UPROPERTY()
	uint32 StateRevision = 0;
	
}; // struct FFlecsDontFragmentReplicationSnapshot