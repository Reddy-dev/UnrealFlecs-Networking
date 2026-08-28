// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "FlecsNetworkSubsystemSingletonTestTypes.h"

#include "Engine/World.h"
#include "Networking/Subsystem/FlecsNetworkSubsystemSingleton.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsNetworkSubsystemSingletonTestTypes)

bool UTestFlecsNetworkSubsystemSingleton_Initialization::ShouldCreateSubsystem(UObject* Outer) const
{
	if (!GIsAutomationTesting)
	{
		return false;
	}

	return Super::ShouldCreateSubsystem(Outer);
}

void UTestFlecsNetworkSubsystemSingleton_Initialization::OnFlecsWorldInitialized(
	const TSolidNotNull<UFlecsWorld*> InWorld)
{
	Super::OnFlecsWorldInitialized(InWorld);

	bWasFlecsWorldInitialized = true;
	bWasNetworkSubsystemSingletonAvailable = InWorld->Has<FFlecsNetworkSubsystemSingleton>();
}
