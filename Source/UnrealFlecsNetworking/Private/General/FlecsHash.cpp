// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "General/FlecsHash.h"

#include "Hash/xxhash.h"

namespace UE::Flecs
{
	struct FHash128Builder::FImpl
	{
		FXxHash128Builder Builder;
	}; // struct FHash128Builder::FImpl

	NO_DISCARD FGuid FHash128::ToGuid() const
	{
		FGuid Guid(
			static_cast<uint32>(High >> 32),
			static_cast<uint32>(High),
			static_cast<uint32>(Low >> 32),
			static_cast<uint32>(Low));

		if (!Guid.IsValid())
		{
			Guid.D = 1;
		}

		return Guid;
	}

	FHash128Builder::FHash128Builder()
		: Impl(MakeUnique<FImpl>())
	{
	}

	FHash128Builder::~FHash128Builder() = default;

	FHash128Builder::FHash128Builder(FHash128Builder&& InOther) noexcept = default;

	FHash128Builder& FHash128Builder::operator=(FHash128Builder&& InOther) noexcept = default;

	void FHash128Builder::Update(const void* InData, const uint64 InSize) const
	{
		solid_check(Impl.Get() != nullptr);
		Impl->Builder.Update(InData, InSize);
	}

	NO_DISCARD FHash128 FHash128Builder::Finalize() const
	{
		solid_check(Impl.Get() != nullptr);

		const auto [HashLow, HashHigh] = Impl->Builder.Finalize();
		return { .Low = HashLow, .High = HashHigh };
	}

} // namespace UE::Flecs
