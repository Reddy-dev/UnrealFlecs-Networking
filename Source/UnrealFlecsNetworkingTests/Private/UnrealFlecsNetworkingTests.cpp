// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsNetworkingTests.h"

#include "General/FlecsModuleRegistry.h"

void FUnrealFlecsNetworkingTestsModule::StartupModule()
{
	UE::Flecs::FFlecsModuleRegistry::Get().RegisterUnrealFlecsModule("UnrealFlecsNetworkingTests");
}

void FUnrealFlecsNetworkingTestsModule::ShutdownModule()
{
}

IMPLEMENT_MODULE(FUnrealFlecsNetworkingTestsModule, UnrealFlecsNetworkingTests)
