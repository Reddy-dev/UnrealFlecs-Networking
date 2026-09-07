// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "Properties/FlecsComponentProperties.h"

#include "FlecsNetAlwaysRelevantTag.generated.h"

// @TODO: Currently does nothing
USTRUCT()
struct FFlecsNetAlwaysRelevantTag
{
	GENERATED_BODY()
	
	static constexpr flecs::on_instantiate OnInstantiate = flecs::on_instantiate::inherit;
	
}; // struct FFlecsNetAlwaysRelevantTag

template <>
struct TFlecsComponentTraits<FFlecsNetAlwaysRelevantTag> : public TFlecsComponentTraitsBase<FFlecsNetAlwaysRelevantTag>
{
	static constexpr EFlecsOnInstantiate OnInstantiate = EFlecsOnInstantiate::Inherit;
}; // struct TFlecsComponentTraits<FFlecsNetAlwaysRelevantTag>

