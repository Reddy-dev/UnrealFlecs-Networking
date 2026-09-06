// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "UnrealFlecsConfigMacros.h"
#include "Fixtures/FlecsReplicationFixture.h"

#if WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS

#include "Iris/ReplicationSystem/NetObjectFactoryRegistry.h"
#include "UObject/UObjectGlobals.h"

#include "Networking/FlecsNetDirtyTag.h"
#include "Networking/FlecsComponentReplicationDescriptor.h"
#include "Networking/Profiles/FlecsReplicationProfile.h"
#include "Networking/Profiles/FlecsReplicationProfileDataAsset.h"
#include "Networking/FlecsReplicationShardSelection.h"
#include "Networking/FlecsReplicationUpdateQueue.h"
#include "Networking/FlecsNetworkingModuleSettings.h"
#include "Networking/FlecsReplicatedEntityComponent.h"
#include "Networking/Layout/FlecsLayoutReplicatorFastArray.h"
#include "Networking/Layout/FlecsReplicationLayoutRegistry.h"
#include "Networking/Profiles/FlecsReplicationProfileParamTypes.h"
#include "Networking/Shards/FlecsNetEntityTable.h"
#include "Networking/Shards/FlecsNetEntityTableNetFactory.h"
#include "Networking/Shards/FlecsNetEntityProxy.h"
#include "Networking/Shards/FlecsNetEntityProxyNetFactory.h"
#include "Networking/Shards/FlecsNetShardBase.h"

FLECS_REPLICATION_TEST_CLASS_WITH_FLAGS_AND_TAGS(FlecsReplicationBridgeTests,
	"UnrealFlecs.Networking.Replication.FakeBridge",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::ProductFilter,
	"[Flecs][Networking][Replication][FakeBridge]")
{
	TEST_METHOD(Fixture_InstallsAndInitializesFakeBridge)
	{
		ASSERT_THAT(IsNotNull(TestBridge()));
		ASSERT_THAT(IsTrue(TestBridge()->IsInitialized()));
		ASSERT_THAT(IsTrue(NetworkSubsystem()->GetReplicationBridge() == TestBridge()));
	}

	TEST_METHOD(DontFragmentReplicatedComponents_RegisterDescriptors)
	{
		const FFlecsComponentReplicationRegistry& Registry =
			FFlecsComponentReplicationRegistry::Get(World());

		const FFlecsId ValueComponentId =
			World()->RegisterComponentType<FFlecsReplicationTestDontFragmentValue>().GetFlecsId();
		const FFlecsComponentReplicationDescriptor* ValueDescriptor = Registry.Find(ValueComponentId);
		ASSERT_THAT(IsNotNull(ValueDescriptor));
		if (!ValueDescriptor)
		{
			return;
		}

		ASSERT_THAT(IsTrue(ValueDescriptor->IsDontFragment()));
		ASSERT_THAT(IsFalse(ValueDescriptor->IsTag()));
		ASSERT_THAT(IsTrue(ValueDescriptor->IsStorageEligible()));
		ASSERT_THAT(IsTrue(Registry.Find(ValueDescriptor->GetSchemaId()) == ValueDescriptor));

		const FFlecsId TagComponentId =
			World()->RegisterComponentType<FFlecsReplicationTestDontFragmentTag>().GetFlecsId();
		const FFlecsComponentReplicationDescriptor* TagDescriptor = Registry.Find(TagComponentId);
		ASSERT_THAT(IsNotNull(TagDescriptor));
		if (!TagDescriptor)
		{
			return;
		}

		ASSERT_THAT(IsTrue(TagDescriptor->IsDontFragment()));
		ASSERT_THAT(IsTrue(TagDescriptor->IsTag()));
		ASSERT_THAT(IsFalse(TagDescriptor->IsStorageEligible()));
		ASSERT_THAT(IsTrue(Registry.Find(TagDescriptor->GetSchemaId()) == TagDescriptor));
	}

	TEST_METHOD(DontFragmentReplicationPair_ResolvesPrimaryStorageDescriptor)
	{
		const FFlecsId DontFragmentValueId =
			World()->RegisterComponentType<FFlecsReplicationTestDontFragmentValue>().GetFlecsId();
		const FFlecsId RelationshipId =
			World()->RegisterComponentType<FFlecsReplicationTestRelationship>().GetFlecsId();
		const FFlecsId PairId = FFlecsId::MakePair(DontFragmentValueId, RelationshipId);

		const TValueOrError<FFlecsReplicationKey, FString> ReplicationKeyResult =
			FFlecsReplicationKey::BuildKey(World(), PairId);
		ASSERT_THAT(IsFalse(ReplicationKeyResult.HasError()));
		if (ReplicationKeyResult.HasError())
		{
			return;
		}

		const FFlecsReplicationKey& ReplicationKey = ReplicationKeyResult.GetValue();
		const FFlecsComponentReplicationDescriptor* Descriptor =
			ReplicationKey.TryGetStorageDescriptor(World());

		ASSERT_THAT(IsTrue(ReplicationKey.Kind == EFlecsReplicationKeyKind::Pair));
		ASSERT_THAT(IsTrue(ReplicationKey.StorageKind == EFlecsReplicationKeyStorageKind::Primary));
		ASSERT_THAT(IsTrue(ReplicationKey.Primary.Kind == EFlecsReplicationPairTargetKind::Schema));
		ASSERT_THAT(IsNotNull(Descriptor));
		if (Descriptor)
		{
			ASSERT_THAT(IsTrue(Descriptor->GetLocalFlecsId() == DontFragmentValueId));
			ASSERT_THAT(IsTrue(Descriptor->IsDontFragment()));
		}
	}

	TEST_METHOD(DontFragmentReplicationPair_ResolvesSecondaryStorageDescriptor)
	{
		const FFlecsId RelationshipId =
			World()->RegisterComponentType<FFlecsReplicationTestRelationship>().GetFlecsId();
		const FFlecsId DontFragmentValueId =
			World()->RegisterComponentType<FFlecsReplicationTestDontFragmentValue>().GetFlecsId();
		const FFlecsId PairId = FFlecsId::MakePair(RelationshipId, DontFragmentValueId);

		const TValueOrError<FFlecsReplicationKey, FString> ReplicationKeyResult =
			FFlecsReplicationKey::BuildKey(World(), PairId);
		ASSERT_THAT(IsFalse(ReplicationKeyResult.HasError()));
		if (ReplicationKeyResult.HasError())
		{
			return;
		}

		const FFlecsReplicationKey& ReplicationKey = ReplicationKeyResult.GetValue();
		const FFlecsComponentReplicationDescriptor* Descriptor =
			ReplicationKey.TryGetStorageDescriptor(World());

		ASSERT_THAT(IsTrue(ReplicationKey.Kind == EFlecsReplicationKeyKind::Pair));
		ASSERT_THAT(IsTrue(ReplicationKey.StorageKind == EFlecsReplicationKeyStorageKind::Secondary));
		ASSERT_THAT(IsTrue(ReplicationKey.Secondary.Kind == EFlecsReplicationPairTargetKind::Schema));
		ASSERT_THAT(IsNotNull(Descriptor));
		if (Descriptor)
		{
			ASSERT_THAT(IsTrue(Descriptor->GetLocalFlecsId() == DontFragmentValueId));
			ASSERT_THAT(IsTrue(Descriptor->IsDontFragment()));
		}
	}

	TEST_METHOD(DontFragmentComponent_CapturesPublicationRemovalAndResetThroughFakeBridge)
	{
		const FFlecsId ComponentId =
			World()->RegisterComponentType<FFlecsReplicationTestDontFragmentValue>().GetFlecsId();
		const TValueOrError<FFlecsReplicationKey, FString> ReplicationKeyResult =
			FFlecsReplicationKey::BuildKey(World(), ComponentId, true);
		ASSERT_THAT(IsFalse(ReplicationKeyResult.HasError()));
		if (ReplicationKeyResult.HasError())
		{
			return;
		}

		const FFlecsReplicationKey ReplicationKey = ReplicationKeyResult.GetValue();
		const FFlecsNetworkId NetworkId(37, 1);
		FFlecsDontFragmentReplicationSnapshot InitialSnapshot;
		InitialSnapshot.SnapshotData = { 3, 7, 11 };
		InitialSnapshot.StateRevision = 1;
		const TArray<uint8> InitialSnapshotData = InitialSnapshot.SnapshotData;

		TestBridge()->PublishDontFragmentComponent(NetworkId, ReplicationKey, InitialSnapshot);

		const TArray<FFlecsTestDontFragmentComponentPublication>& Publications =
			TestBridge()->GetPublishedDontFragmentComponents();
		ASSERT_THAT(AreEqual(1, Publications.Num()));
		if (Publications.Num() != 1)
		{
			return;
		}

		const FFlecsTestDontFragmentComponentPublication& InitialPublication = Publications[0];
		ASSERT_THAT(IsTrue(InitialPublication.NetworkId == NetworkId));
		ASSERT_THAT(IsTrue(
			InitialPublication.ReplicationKey.Kind == EFlecsReplicationKeyKind::Component));
		ASSERT_THAT(IsTrue(
			InitialPublication.ReplicationKey.StorageKind == EFlecsReplicationKeyStorageKind::Primary));
		const FFlecsReplicationKey InitialReplicationKey = InitialPublication.ReplicationKey;
		ASSERT_THAT(IsTrue(InitialPublication.Snapshot.SnapshotData == InitialSnapshotData));
		ASSERT_THAT(AreEqual(1u, InitialPublication.Snapshot.StateRevision));

		InitialSnapshot.SnapshotData[0] = 99;
		InitialSnapshot.StateRevision = 99;
		ASSERT_THAT(IsTrue(InitialPublication.Snapshot.SnapshotData == InitialSnapshotData));
		ASSERT_THAT(AreEqual(1u, InitialPublication.Snapshot.StateRevision));

		FFlecsDontFragmentReplicationSnapshot UpdatedSnapshot;
		UpdatedSnapshot.SnapshotData = { 13, 17 };
		UpdatedSnapshot.StateRevision = 2;
		TestBridge()->PublishDontFragmentComponent(NetworkId, ReplicationKey, UpdatedSnapshot);
		ASSERT_THAT(AreEqual(2, Publications.Num()));
		if (Publications.Num() != 2)
		{
			return;
		}

		const FFlecsTestDontFragmentComponentPublication& UpdatedPublication = Publications[1];
		ASSERT_THAT(IsTrue(UpdatedPublication.NetworkId == NetworkId));
		ASSERT_THAT(IsTrue(UpdatedPublication.ReplicationKey == InitialReplicationKey));
		ASSERT_THAT(IsTrue(UpdatedPublication.Snapshot.SnapshotData == UpdatedSnapshot.SnapshotData));
		ASSERT_THAT(AreEqual(2u, UpdatedPublication.Snapshot.StateRevision));

		TestBridge()->RemoveDontFragmentComponent(NetworkId, ReplicationKey);
		const TArray<FFlecsTestDontFragmentComponentRemoval>& Removals =
			TestBridge()->GetRemovedDontFragmentComponents();
		ASSERT_THAT(AreEqual(1, Removals.Num()));
		if (Removals.Num() != 1)
		{
			return;
		}

		ASSERT_THAT(IsTrue(Removals[0].NetworkId == NetworkId));
		ASSERT_THAT(IsTrue(Removals[0].ReplicationKey == ReplicationKey));

		TestBridge()->ResetCapturedRecords();
		ASSERT_THAT(AreEqual(0, Publications.Num()));
		ASSERT_THAT(AreEqual(0, Removals.Num()));
	}

	TEST_METHOD(ReplicationQueue_CoalescesByLatestStateRevision)
	{
		FFlecsReplicationUpdateQueue Queue;
		const FFlecsNetworkId NetworkId(21, 1);

		FFlecsEntityReplicationSnapshot OlderSnapshot;
		OlderSnapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		OlderSnapshot.StateRevision = 4;

		FFlecsEntityReplicationSnapshot NewerSnapshot = OlderSnapshot;
		NewerSnapshot.StateRevision = 5;

		Queue.EnqueueSnapshot(NetworkId, NewerSnapshot);
		Queue.EnqueueSnapshot(NetworkId, OlderSnapshot);

		const TArray<FFlecsReplicationQueuedUpdate> Updates = Queue.Drain();
		ASSERT_THAT(AreEqual(1, Updates.Num()));
		if (Updates.Num() != 1)
		{
			return;
		}

		ASSERT_THAT(IsFalse(Updates[0].bRemove));
		ASSERT_THAT(AreEqual(5u, Updates[0].StateRevision));
		ASSERT_THAT(AreEqual(5u, Updates[0].Snapshot.StateRevision));
	}

	TEST_METHOD(ReplicationQueue_OrdersRemovalsByStateRevision)
	{
		FFlecsReplicationUpdateQueue Queue;
		const FFlecsNetworkId NetworkId(22, 1);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 7;
		FFlecsEntityReplicationSnapshot OlderSnapshot = Snapshot;
		OlderSnapshot.StateRevision = 6;

		Queue.EnqueueSnapshot(NetworkId, Snapshot);
		Queue.EnqueueRemoval(NetworkId, 7);
		Queue.EnqueueSnapshot(NetworkId, OlderSnapshot);

		const TArray<FFlecsReplicationQueuedUpdate> Updates = Queue.Drain();
		ASSERT_THAT(AreEqual(1, Updates.Num()));
		if (Updates.Num() != 1)
		{
			return;
		}

		ASSERT_THAT(IsTrue(Updates[0].bRemove));
		ASSERT_THAT(AreEqual(7u, Updates[0].StateRevision));
	}

	TEST_METHOD(ReplicationProfile_AssetCreatesFlecsPrefab)
	{
		UFlecsReplicationProfileDataAsset* Asset = NewObject<UFlecsReplicationProfileDataAsset>(NetworkSubsystem());
		Asset->ProfileName = FName(TEXT("TestProfile"));
		
		Asset->Definition.AddParam<FFlecsReplicationProfileNetFilter>(FName(TEXT("TestFilter")));

		Asset->Definition.AddParam<FFlecsReplicationProfileObjectPrioritizer>(FName(TEXT("TestPrioritizer")));

		Asset->Definition.AddParam<FFlecsReplicationProfileNetShardSelector>(FName(TEXT("Proxy")));

		const FFlecsEntityHandle ProfilePrefab = NetworkSubsystem()->RegisterReplicationProfileAsset(Asset);
		ASSERT_THAT(IsTrue(ProfilePrefab.IsValid()));
		ASSERT_THAT(IsTrue(ProfilePrefab.IsPrefab()));
		ASSERT_THAT(IsTrue(ProfilePrefab.Has<FFlecsReplicationProfileTag>()));

		const FFlecsEntityHandle Entity = World()->CreateEntity().AddPrefab(ProfilePrefab.GetFlecsId());
		
		auto ResolvedProfileOutcome = NetworkSubsystem()->ResolveReplicationProfile(Entity);
		ASSERT_THAT(IsTrue(ResolvedProfileOutcome.IsValid()));
		ASSERT_THAT(IsTrue(ResolvedProfileOutcome.GetValue().Get<FFlecsReplicationProfileDefinition>() == Asset->Definition));
	}

	TEST_METHOD(ReplicationProfile_SetProfileReplacesRegisteredIsAProfile)
	{
		FFlecsReplicationProfileDefinition FirstDefinition;
		FirstDefinition.AddParam<FFlecsReplicationProfileNetFilter>(FName(TEXT("FirstFilter")));

		const FFlecsEntityHandle FirstPrefab = NetworkSubsystem()->RegisterReplicationProfileDefinition(
			FName(TEXT("FirstProfile")), FirstDefinition);

		FFlecsReplicationProfileDefinition SecondDefinition;
		SecondDefinition.AddParam<FFlecsReplicationProfileNetFilter>(FName(TEXT("SecondFilter")));
		const FFlecsEntityHandle SecondPrefab = NetworkSubsystem()->RegisterReplicationProfileDefinition(
			FName(TEXT("SecondProfile")), SecondDefinition);

		const FFlecsEntityHandle Entity = World()->CreateEntity()
			.Add<FFlecsReplicatedEntityComponent>()
			.AddPrefab(FirstPrefab.GetFlecsId());
		ASSERT_THAT(IsTrue(NetworkSubsystem()->SetReplicationProfile(Entity, SecondPrefab)));

		const FFlecsReplicatedEntityComponent* ReplicatedEntity = Entity.TryGet<FFlecsReplicatedEntityComponent>();
		ASSERT_THAT(IsNotNull(ReplicatedEntity));
		if (!ReplicatedEntity)
		{
			return;
		}

		ASSERT_THAT(IsTrue(ReplicatedEntity->ProfileId == FName(TEXT("SecondProfile"))));

		FFlecsEntityView ResolvedProfile;
		auto ResolvedProfileOutcome = NetworkSubsystem()->ResolveReplicationProfile(Entity);
		ASSERT_THAT(IsTrue(ResolvedProfileOutcome.IsValid()));
		ASSERT_THAT(IsTrue(ResolvedProfileOutcome.GetValue().Get<FFlecsReplicationProfileDefinition>() == SecondDefinition));
	}

	TEST_METHOD(ReplicationShardSelector_UsesRegisteredImplementation)
	{
		bool bSelectorCalled = false;
		ASSERT_THAT(IsTrue(NetworkSubsystem()->RegisterReplicationShardSelector(
			FName(TEXT("TestSelector")),
			[&bSelectorCalled](const FFlecsEntityHandle&, const FFlecsNetworkId&,
				const FFlecsEntityView&, FFlecsReplicationShardSelection& OutSelection)
			{
				bSelectorCalled = true;
				OutSelection.ShardClass = UFlecsNetEntityProxy::StaticClass();
				OutSelection.ShardGroupKey = FName(TEXT("TestShardGroup"));
				return true;
			})));

		FFlecsReplicationProfileDefinition ProfileDefinition;
		ProfileDefinition.AddParam<FFlecsReplicationProfileNetShardSelector>(FName(TEXT("TestSelector")));
		FFlecsEntityView Profile = NetworkSubsystem()->RegisterReplicationProfileDefinition("NewProfile", ProfileDefinition);
		
		
		const FFlecsNetworkId NetworkId(44, 1);
		const auto SelectionOutcome = NetworkSubsystem()
			->SelectReplicationShard(World()->CreateEntity(), NetworkId, Profile);

		ASSERT_THAT(IsTrue(SelectionOutcome.IsValid()));
		ASSERT_THAT(IsTrue(bSelectorCalled));
		ASSERT_THAT(IsTrue(SelectionOutcome.GetValue().ShardClass == UFlecsNetEntityProxy::StaticClass()));
		ASSERT_THAT(IsTrue(SelectionOutcome.GetValue().ShardGroupKey == FName(TEXT("TestShardGroup"))));
	}

	TEST_METHOD(EntityTableAndEntityProxy_AreShardStorageTypes)
	{
		ASSERT_THAT(IsTrue(
			UFlecsNetEntityTable::StaticClass()->IsChildOf(UFlecsNetShardBase::StaticClass())));
		ASSERT_THAT(IsTrue(
			UFlecsNetEntityProxy::StaticClass()->IsChildOf(UFlecsNetShardBase::StaticClass())));
	}

	/*TEST_METHOD(EntityProxy_UsesAlwaysRelevantRegisteredFactory)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		UE::Net::FRootObjectSettings Settings;
		Proxy->ConfigureObjectSettings(Settings);

		ASSERT_THAT(IsTrue(Settings.bIsAlwaysRelevant));
		ASSERT_THAT(IsFalse(Settings.bIsNotRouted));
		ASSERT_THAT(IsTrue(
			Settings.FactoryName == UFlecsNetEntityProxyNetFactory::GetFactoryName()));
		ASSERT_THAT(IsTrue(
			UE::Net::FNetObjectFactoryRegistry::GetFactoryIdFromName(Settings.FactoryName)
				!= UE::Net::InvalidNetObjectFactoryId));
	}*/

	TEST_METHOD(EntityProxy_UsesAssignedWorldWithoutWorldOuter)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(GetTransientPackage());
		ASSERT_THAT(IsTrue(Proxy->GetOuter() == GetTransientPackage()));

		Proxy->SetOwningWorld(NetworkSubsystem()->GetWorld());
		ASSERT_THAT(IsTrue(Proxy->GetWorld() == NetworkSubsystem()->GetWorld()));
	}

	TEST_METHOD(EntityProxy_QueuesUpdateUntilWorldIsAssigned)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(GetTransientPackage());
		const FFlecsNetworkId NetworkId(16, 3);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 1;

		Proxy->NetworkId = NetworkId;
		Proxy->Snapshot = Snapshot;
		Proxy->OnRep_Snapshot();
		ASSERT_THAT(AreEqual(0, NetworkSubsystem()->GetQueuedReplicationUpdateCount()));

		Proxy->SetOwningWorld(NetworkSubsystem()->GetWorld());
		ASSERT_THAT(AreEqual(1, NetworkSubsystem()->GetQueuedReplicationUpdateCount()));
	}

	/*TEST_METHOD(EntityProxy_PublishNetEntity_CopiesIdentityAndSnapshot)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		const FFlecsNetworkId NetworkId(17, 3);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 9;

		FFlecsReplicatedValue Value;
		Value.KeyIndex = 2;
		Value.Bytes = { 4, 8, 15, 16, 23, 42 };
		Snapshot.Values.Add(Value);

		Proxy->PublishNetEntity(NetworkId, Snapshot);

		ASSERT_THAT(IsTrue(Proxy->NetworkId == NetworkId));
		ASSERT_THAT(IsTrue(Proxy->Snapshot.LayoutId == Snapshot.LayoutId));
		ASSERT_THAT(AreEqual(Snapshot.StateRevision, Proxy->Snapshot.StateRevision));
		ASSERT_THAT(AreEqual(Snapshot.Values.Num(), Proxy->Snapshot.Values.Num()));
		ASSERT_THAT(IsTrue(Proxy->Snapshot.Values[0].Bytes == Value.Bytes));
	}*/

	TEST_METHOD(EntityProxy_RemovingEntityDoesNotRequireShardTeardown)
	{
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		const FFlecsNetworkId NetworkId(18, 3);

		FFlecsEntityReplicationSnapshot Snapshot;
		Snapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		Snapshot.StateRevision = 1;

		Proxy->PublishNetEntity(NetworkId, Snapshot);
		ASSERT_THAT(IsFalse(Proxy->IsEmpty()));
		ASSERT_THAT(IsTrue(Proxy->CanAcceptNetEntity(NetworkId, Snapshot)));
		ASSERT_THAT(IsFalse(Proxy->CanAcceptNetEntity(FFlecsNetworkId(19, 3), Snapshot)));

		Proxy->RemoveNetEntity(NetworkId, false);
		ASSERT_THAT(IsTrue(Proxy->IsEmpty()));
	}

	// @TODO: Fix
	/*TEST_METHOD(EntityTable_UsesRegisteredFactoryAndUpsertsByNetworkId)
	{
		UFlecsNetEntityTable* Table = NewObject<UFlecsNetEntityTable>(NetworkSubsystem());
		UE::Net::FRootObjectSettings Settings;
		Table->ConfigureObjectSettings(Settings);

		ASSERT_THAT(IsTrue(Settings.FactoryName == UFlecsNetEntityTableNetFactory::GetFactoryName()));
		ASSERT_THAT(IsTrue(
			UE::Net::FNetObjectFactoryRegistry::GetFactoryIdFromName(Settings.FactoryName)
				!= UE::Net::InvalidNetObjectFactoryId));

		const FFlecsNetworkId NetworkId(20, 3);
		FFlecsEntityReplicationSnapshot InitialSnapshot;
		InitialSnapshot.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());
		InitialSnapshot.StateRevision = 1;

		Table->PublishNetEntity(NetworkId, InitialSnapshot);
		ASSERT_THAT(AreEqual(1, Table->EntityTable.Items.Num()));
		ASSERT_THAT(IsFalse(Table->IsEmpty()));

		FFlecsEntityReplicationSnapshot UpdatedSnapshot = InitialSnapshot;
		UpdatedSnapshot.StateRevision = 2;
		Table->PublishNetEntity(NetworkId, UpdatedSnapshot);
		ASSERT_THAT(AreEqual(1, Table->EntityTable.Items.Num()));
		ASSERT_THAT(AreEqual(2u, Table->EntityTable.Items[0].Snapshot.StateRevision));

		Table->RemoveNetEntity(NetworkId);
		ASSERT_THAT(IsTrue(Table->IsEmpty()));
	}*/

	TEST_METHOD(EntityProxy_AppliesUpdatesAndRejectsStaleRevisions)
	{
		const FFlecsEntityHandle SourceEntity = World()->CreateEntity()
			.Set<FFlecsReplicationTestValue>({ 17 });

		bool bCreatedNewLayout = false;
		const TValueOrError<const FFlecsReplicationLayoutDefinition*, FString> LayoutResult =
			NetworkSubsystem()->GetLayoutRegistry().BuildForEntity(
				World(), SourceEntity, bCreatedNewLayout);

		ASSERT_THAT(IsFalse(LayoutResult.HasError()));
		if (LayoutResult.HasError())
		{
			return;
		}

		const FFlecsReplicationLayoutDefinition* LayoutDefinition = LayoutResult.GetValue();
		ASSERT_THAT(IsNotNull(LayoutDefinition));
		if (!LayoutDefinition)
		{
			return;
		}

		FFlecsEntityReplicationSnapshot InitialSnapshot;
		InitialSnapshot.LayoutId = LayoutDefinition->LayoutId;
		InitialSnapshot.FillFromEntity(SourceEntity, NetworkSubsystem()->GetLayoutRegistry());

		const FFlecsNetworkId NetworkId(31, 1);
		UFlecsNetEntityProxy* Proxy = NewObject<UFlecsNetEntityProxy>(NetworkSubsystem());
		Proxy->SetOwningNetworkWorldSubsystem(NetworkSubsystem());
		Proxy->NetworkId = NetworkId;
		Proxy->Snapshot = InitialSnapshot;
		Proxy->OnRep_Snapshot();
		NetworkSubsystem()->ApplyQueuedReplicationUpdates(World());

		TOptional<FFlecsEntityHandle> ReceivedEntity = NetworkSubsystem()->GetEntityFromNetworkId(NetworkId);
		ASSERT_THAT(IsTrue(ReceivedEntity.IsSet()));
		if (!ReceivedEntity.IsSet())
		{
			return;
		}

		ASSERT_THAT(AreEqual(17, ReceivedEntity.GetValue().Get<FFlecsReplicationTestValue>().Value));

		SourceEntity.Set<FFlecsReplicationTestValue>({ 91 });
		FFlecsEntityReplicationSnapshot UpdatedSnapshot = InitialSnapshot;
		UpdatedSnapshot.FillFromEntity(SourceEntity, NetworkSubsystem()->GetLayoutRegistry());

		Proxy->Snapshot = UpdatedSnapshot;
		Proxy->OnRep_Snapshot();
		NetworkSubsystem()->ApplyQueuedReplicationUpdates(World());
		ASSERT_THAT(AreEqual(91, ReceivedEntity.GetValue().Get<FFlecsReplicationTestValue>().Value));

		Proxy->Snapshot = InitialSnapshot;
		Proxy->OnRep_Snapshot();
		NetworkSubsystem()->ApplyQueuedReplicationUpdates(World());
		ASSERT_THAT(AreEqual(91, ReceivedEntity.GetValue().Get<FFlecsReplicationTestValue>().Value));
	}

	TEST_METHOD(LayoutFastArray_AddsIdempotently)
	{
		FFlecsReplicationLayoutDefinition Layout;
		Layout.LayoutId = FFlecsReplicationLayoutId(FGuid::NewGuid());

		FFlecsReplicatorFastArray Layouts;

		ASSERT_THAT(IsTrue(Layouts.AddLayout(Layout)));
		ASSERT_THAT(IsFalse(Layouts.AddLayout(Layout)));
		ASSERT_THAT(IsTrue(Layouts.Items.Num() == 1));
		ASSERT_THAT(IsTrue(Layouts.Items[0].ReplicationID != INDEX_NONE));
		ASSERT_THAT(IsTrue(Layouts.FindLayout(Layout.LayoutId) != nullptr));
	}

	
}; // FlecsReplicationBridgeTests

#endif // WITH_AUTOMATION_TESTS && ENABLE_UNREAL_FLECS_TESTS
