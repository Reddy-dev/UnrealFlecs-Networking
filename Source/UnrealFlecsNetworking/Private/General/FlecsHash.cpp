// Elie Wiese-Namir © 2026. All Rights Reserved.

#include "General/FlecsHash.h"

#include "Hash/xxhash.h"

namespace UE::Flecs
{
	struct FHash128Builder::FImpl
	{
		FXxHash128Builder Builder;
	};

	FGuid FHash128::ToGuid() const
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

	void FHash128Builder::Update(const void* InData, const uint64 InSize)
	{
		check(Impl.Get() != nullptr);
		Impl->Builder.Update(InData, InSize);
	}

	FHash128 FHash128Builder::Finalize() const
	{
		check(Impl.Get() != nullptr);

		const FXxHash128 Hash = Impl->Builder.Finalize();
		return { Hash.HashLow, Hash.HashHigh };
	}

} // namespace UE::Flecs
