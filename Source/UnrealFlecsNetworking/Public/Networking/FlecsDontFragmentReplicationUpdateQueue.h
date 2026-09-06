// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once

#include "Containers/Array.h"

#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsReplicationKey.h"
#include "Networking/Layout/FlecsDontFragmentReplicationSnapshot.h"

/** One deferred client-side DontFragment component update. */
struct FFlecsDontFragmentReplicationQueuedUpdate
{
	FFlecsNetworkId NetworkId;
	FFlecsReplicationKey ReplicationKey;
	FFlecsDontFragmentReplicationSnapshot Snapshot;
	uint32 StateRevision = 0;
	bool bRemove = false;

}; // struct FFlecsDontFragmentReplicationQueuedUpdate

/** Coalesces deferred DontFragment component updates by entity and component key. */
class FFlecsDontFragmentReplicationUpdateQueue
{
public:
	UE_FORCEINLINE_HINT void EnqueueSnapshot(const FFlecsNetworkId& InNetworkId,
		const FFlecsReplicationKey& InReplicationKey,
		const FFlecsDontFragmentReplicationSnapshot& InSnapshot)
	{
		if (FFlecsDontFragmentReplicationQueuedUpdate* ExistingUpdate =
			FindPendingUpdate(InNetworkId, InReplicationKey))
		{
			if (ExistingUpdate->StateRevision > InSnapshot.StateRevision)
			{
				return;
			}

			ExistingUpdate->Snapshot = InSnapshot;
			ExistingUpdate->StateRevision = InSnapshot.StateRevision;
			ExistingUpdate->bRemove = false;
			return;
		}

		FFlecsDontFragmentReplicationQueuedUpdate& Update = Updates.Emplace_GetRef();
		Update.NetworkId = InNetworkId;
		Update.ReplicationKey = InReplicationKey;
		Update.Snapshot = InSnapshot;
		Update.StateRevision = InSnapshot.StateRevision;
		Update.bRemove = false;
	}

	UE_FORCEINLINE_HINT void EnqueueRemoval(const FFlecsNetworkId& InNetworkId,
		const FFlecsReplicationKey& InReplicationKey,
		const uint32 InStateRevision)
	{
		if (FFlecsDontFragmentReplicationQueuedUpdate* ExistingUpdate =
			FindPendingUpdate(InNetworkId, InReplicationKey))
		{
			if (ExistingUpdate->StateRevision > InStateRevision)
			{
				return;
			}

			ExistingUpdate->Snapshot = {};
			ExistingUpdate->StateRevision = InStateRevision;
			ExistingUpdate->bRemove = true;
			return;
		}

		FFlecsDontFragmentReplicationQueuedUpdate& Update = Updates.Emplace_GetRef();
		Update.NetworkId = InNetworkId;
		Update.ReplicationKey = InReplicationKey;
		Update.StateRevision = InStateRevision;
		Update.bRemove = true;
	}

	NO_DISCARD UE_FORCEINLINE_HINT TArray<FFlecsDontFragmentReplicationQueuedUpdate> Drain()
	{
		return MoveTemp(Updates);
	}

	UE_FORCEINLINE_HINT void Reset()
	{
		Updates.Reset();
	}

private:
	NO_DISCARD FFlecsDontFragmentReplicationQueuedUpdate* FindPendingUpdate(
		const FFlecsNetworkId& InNetworkId,
		const FFlecsReplicationKey& InReplicationKey)
	{
		return Updates.FindByPredicate([&InNetworkId, &InReplicationKey](
			const FFlecsDontFragmentReplicationQueuedUpdate& InUpdate)
		{
			return InUpdate.NetworkId == InNetworkId
				&& InUpdate.ReplicationKey == InReplicationKey;
		});
	}

	TArray<FFlecsDontFragmentReplicationQueuedUpdate> Updates;

}; // class FFlecsDontFragmentReplicationUpdateQueue
