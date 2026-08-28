// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Worlds/FlecsAbstractWorldSubsystem.h"

#include "FlecsNetworkSubsystemSingletonTestTypes.generated.h"

UCLASS()
class UNREALFLECSNETWORKINGTESTS_API UTestFlecsNetworkSubsystemSingleton_Initialization
	: public UFlecsAbstractWorldSubsystem
{
	GENERATED_BODY()

public:
	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

	virtual void OnFlecsWorldInitialized(const TSolidNotNull<UFlecsWorld*> InWorld) override;

	UPROPERTY()
	bool bWasFlecsWorldInitialized = false;

	UPROPERTY()
	bool bWasNetworkSubsystemSingletonAvailable = false;
}; // class UTestFlecsNetworkSubsystemSingleton_Initialization
