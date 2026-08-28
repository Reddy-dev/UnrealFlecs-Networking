// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "UnrealFlecsTests/Fixtures/FlecsWorldFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "FlecsNetworkSubsystemSingletonTestTypes.h"

FLECS_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsNetworkSubsystemSingletonTests,
	"UnrealFlecs.Networking.Subsystems",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter
		| EAutomationTestFlags::CriticalPriority, "[Flecs][Networking]")
{
	TEST_METHOD(NetworkSubsystemSingleton_IsAvailableWhenAbstractSubsystemsAreInitialized)
	{
		const UTestFlecsNetworkSubsystemSingleton_Initialization* WorldSubsystem =
			World()->GetWorld()->GetSubsystem<UTestFlecsNetworkSubsystemSingleton_Initialization>();
		ASSERT_THAT(IsTrue(IsValid(WorldSubsystem)));

		ASSERT_THAT(IsTrue(WorldSubsystem->bWasFlecsWorldInitialized));
		ASSERT_THAT(IsFalse(WorldSubsystem->bWasNetworkSubsystemSingletonAvailable));
	}
}; // FlecsNetworkSubsystemSingletonTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
