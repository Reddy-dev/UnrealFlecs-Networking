// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "Networking/FlecsComponentReplicationDescriptor.h"

#include "Misc/SecureHash.h"
#include "UObject/UnrealType.h"

#include "Logs/FlecsCategories.h"
#include "Networking/FlecsNetworkId.h"
#include "Networking/FlecsReplicatedTrait.h"

#include UE_INLINE_GENERATED_CPP_BY_NAME(FlecsComponentReplicationDescriptor)

namespace
{
	using FWorldRegistryMap = TMap<TWeakObjectPtr<const UFlecsWorld>, TUniquePtr<FFlecsComponentReplicationRegistry>>;

	FWorldRegistryMap& GetWorldRegistries()
	{
		static FWorldRegistryMap Registries;
		return Registries;
	}

	void RemoveExpiredWorldRegistries()
	{
		for (auto It = GetWorldRegistries().CreateIterator(); It; ++It)
		{
			if (!It.Key().IsValid())
			{
				It.RemoveCurrent();
			}
		}
	}

	TValueOrError<void, FString> ValidateProperty(const FProperty* Property, TSet<const UStruct*>& Visited)
	{
		if (Property->IsA<FSoftObjectProperty>() || Property->IsA<FSoftClassProperty>())
		{
			return MakeValue();
		}
		
		if (Property->IsA<FObjectPropertyBase>() || Property->IsA<FInterfaceProperty>())
		{
			return MakeError(FString::Printf(TEXT("Raw UObject reference property '%s' is not supported"),
				*Property->GetPathName()));
		}
		
		if (const FArrayProperty* ArrayProperty = CastField<FArrayProperty>(Property))
		{
			return ValidateProperty(ArrayProperty->Inner, Visited);
		}
		
		if (const FSetProperty* SetProperty = CastField<FSetProperty>(Property))
		{
			return ValidateProperty(SetProperty->ElementProp, Visited);
		}
		
		if (const FMapProperty* MapProperty = CastField<FMapProperty>(Property))
		{
			const TValueOrError<void, FString> KeyResult = ValidateProperty(MapProperty->KeyProp, Visited);
			if (KeyResult.HasError())
			{
				return MakeError(KeyResult.GetError());
			}
			
			const TValueOrError<void, FString> ValueResult = ValidateProperty(MapProperty->ValueProp, Visited);
			if (ValueResult.HasError())
			{
				return MakeError(ValueResult.GetError());
			}
		}
		
		if (const FStructProperty* StructProperty = CastField<FStructProperty>(Property))
		{
			if (Visited.Contains(StructProperty->Struct))
			{
				return MakeValue();
			}
			
			Visited.Add(StructProperty->Struct);
			
			for (TFieldIterator<FProperty> It(StructProperty->Struct); It; ++It)
			{
				const TValueOrError<void, FString> Result = ValidateProperty(*It, Visited); 
				if (Result.HasError())
				{
					return MakeError(Result.GetError());
				}
			}
		}
		
		return MakeValue();
	}
	
} // namespace

FFlecsReplicationSchemaId FFlecsReplicationSchemaId::FromStableName(const FString& StableName)
{
	if (StableName.IsEmpty())
	{
		return {};
	}

	const FTCHARToUTF8 Utf8(*StableName);
	
	FMD5 Md5;
	Md5.Update(reinterpret_cast<const uint8*>(Utf8.Get()), Utf8.Length());
	
	FMD5Hash Hash;
	Hash.Set(Md5);
	FGuid Guid = MD5HashToGuid(Hash);
	
	if (!Guid.IsValid())
	{
		Guid.D = 1;
	}
	
	return FFlecsReplicationSchemaId(Guid);
}

TValueOrError<void, FString> FFlecsComponentReplicationDescriptor::Verify() const
{
	if (StableName.IsEmpty() || !SchemaId.IsValid())
	{
		return MakeError(TEXT("Replication stable name/schema ID is missing"));
	}
	
	if (!LocalFlecsId.IsValid())
	{
		return MakeError(TEXT("Local Flecs ID is invalid"));
	}
	
	if (!bIsTag && (Size == 0 || Alignment == 0))
	{
		return MakeError(TEXT("Data component size/alignment is invalid"));
	}
	
	if (!bIsTag && (!Serialize || !Deserialize || !Construct || !Destroy))
	{
		return MakeError(TEXT("Native replication operations are incomplete"));
	}
	
	return MakeValue();
}

FFlecsComponentReplicationRegistry& FFlecsComponentReplicationRegistry::Get(const TSolidNotNull<const UFlecsWorld*> World)
{
	RemoveExpiredWorldRegistries();
	const TWeakObjectPtr<const UFlecsWorld> Key(World);
	
	TUniquePtr<FFlecsComponentReplicationRegistry>& Registry = GetWorldRegistries().FindOrAdd(Key);
	
	if (!Registry)
	{
		Registry = MakeUnique<FFlecsComponentReplicationRegistry>();
	}
	
	return *Registry;
}

void FFlecsComponentReplicationRegistry::RemoveWorld(const UFlecsWorld* World)
{
	if LIKELY_IF(World)
	{
		GetWorldRegistries().Remove(TWeakObjectPtr(World));
	}
}

TValueOrError<void, FString> FFlecsComponentReplicationRegistry::Register(const FFlecsComponentReplicationDescriptor& Descriptor)
{
	const TValueOrError<void, FString> IsValidOutcome = Descriptor.Verify();
	if (IsValidOutcome.HasError())
	{
		return MakeError(IsValidOutcome.GetError());
	}
	
	if (Descriptor.ScriptStruct)
	{
		const TValueOrError<void, FString> ValidationResult = ValidateReflectedType(Descriptor.ScriptStruct);
		
		if (ValidationResult.HasError())
		{
			return MakeError(ValidationResult.GetError());
		}
	}
	
	if (const FFlecsId* ExistingLocal = SchemaToLocalId.Find(Descriptor.SchemaId))
	{
		if (*ExistingLocal == Descriptor.LocalFlecsId)
		{
			return MakeValue();
		}
		
		return MakeError(FString::Printf(TEXT("Duplicate replication schema ID %s for '%s'"),
		                           *Descriptor.SchemaId.ToString(), *Descriptor.StableName));
	}
	
	const FFlecsId LocalId = Descriptor.LocalFlecsId;
	
	SchemaToLocalId.Add(Descriptor.SchemaId, LocalId);
	
	ByLocalId.Add(LocalId, Descriptor);
	DescriptorRegisteredDelegate.Broadcast(ByLocalId.FindChecked(LocalId));
	
	return MakeValue();
}

const FFlecsComponentReplicationDescriptor* FFlecsComponentReplicationRegistry::Find(const FFlecsId LocalId) const
{
	if UNLIKELY_IF(!LocalId.IsValid())
	{
		UE_LOGFMT(LogFlecsWorld, Warning, 
			"Invalid Flecs ID provided for component replication descriptor lookup in world");
		return nullptr;
	}
	
	return ByLocalId.Find(LocalId);
}

const FFlecsComponentReplicationDescriptor* FFlecsComponentReplicationRegistry::Find(const FFlecsReplicationSchemaId& SchemaId) const
{
	const FFlecsId* LocalId = SchemaToLocalId.Find(SchemaId);
	return LocalId ? ByLocalId.Find(*LocalId) : nullptr;
}

TValueOrError<void, FString> FFlecsComponentReplicationRegistry::ValidateReflectedType(
	const TSolidNotNull<const UScriptStruct*> ScriptStruct)
{
	TSet<const UStruct*> Visited;
	Visited.Add(ScriptStruct);
	
	for (TFieldIterator<FProperty> It(ScriptStruct); It; ++It)
	{
		const TValueOrError<void, FString> Result = ValidateProperty(*It, Visited);
		if UNLIKELY_IF(Result.HasError())
		{
			return MakeError(Result.GetError());
		}
	}
	
	return MakeValue();
}

bool FFlecsComponentReplicationRegistry::IsEntityReplicationEligible(const TSolidNotNull<const UFlecsWorld*> World,
	const FFlecsId Id)
{
	if UNLIKELY_IF(!Id.IsValid())
	{
		UE_LOGFMT(LogFlecsWorld, Error, 
			"Invalid Flecs ID provided for entity replication eligibility check in world");
		return false;
	}
	
	const FFlecsEntityHandle EntityHandle = World->GetAlive(Id);
	return EntityHandle.IsValid() && (EntityHandle.Has<FFlecsNetworkId>() || EntityHandle.Has<FFlecsReplicatedTrait>());
}

TValueOrError<void, FString> UE::Flecs::Replication::RegisterComponentDefinition(
	const TSolidNotNull<const UFlecsWorld*> InWorld,
	const FFlecsReplicationComponentDefinition& InDefinition)
{
	FFlecsComponentReplicationDescriptor Descriptor;
	Descriptor.StableName = InDefinition.StableName;
	Descriptor.SchemaId = FFlecsReplicationSchemaId::FromStableName(InDefinition.StableName);
	Descriptor.LocalFlecsId = InDefinition.LocalFlecsId;
	Descriptor.Size = InDefinition.Size;
	Descriptor.Alignment = InDefinition.Alignment;
	Descriptor.bIsTag = InDefinition.bIsTag;
	Descriptor.bDontFragment = InDefinition.bDontFragment;
	Descriptor.ScriptStruct = InDefinition.ScriptStruct;
	Descriptor.Serialize = InDefinition.Serialize;
	Descriptor.Deserialize = InDefinition.Deserialize;
	Descriptor.Construct = InDefinition.Construct;
	Descriptor.Destroy = InDefinition.Destroy;

	TValueOrError<void, FString> RegistrationOutcome
		= FFlecsComponentReplicationRegistry::Get(InWorld).Register(MoveTemp(Descriptor));

	if (RegistrationOutcome.HasError())
	{
		return MakeError(RegistrationOutcome.GetError());
	}

	return MakeValue();
}

void UE::Flecs::Replication::MarkComponentReplicated(const FFlecsComponentHandle& InComponent)
{
	InComponent.GetFlecsWorldChecked()->RegisterComponentType<FFlecsReplicatedTrait>();

	InComponent.Add<FFlecsReplicatedTrait>();
}
