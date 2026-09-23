// Elie Wiese-Namir © 2026. All Rights Reserved.

#pragma once


#include "CoreTypes.h"
#include "Misc/Guid.h"
#include "SolidMacros/Macros.h"
#include "Templates/UniquePtr.h"

namespace UE::Flecs
{
	/** A non-cryptographic 128-bit hash value. */
	struct UNREALFLECSNETWORKING_API FHash128
	{
		uint64 Low = 0;
		uint64 High = 0;

		/** Converts this hash to a valid GUID for APIs that use FGuid as an identity. */
		NO_DISCARD FGuid ToGuid() const;
	};

	/** Incrementally hashes bytes without exposing the selected hash implementation. */
	class UNREALFLECSNETWORKING_API FHash128Builder
	{
	public:
		FHash128Builder();
		~FHash128Builder();

		FHash128Builder(const FHash128Builder&) = delete;
		FHash128Builder& operator=(const FHash128Builder&) = delete;

		FHash128Builder(FHash128Builder&& InOther) noexcept;
		FHash128Builder& operator=(FHash128Builder&& InOther) noexcept;

		void Update(const void* InData, uint64 InSize);

		NO_DISCARD FHash128 Finalize() const;

	private:
		struct FImpl;
		TUniquePtr<FImpl> Impl;
	};

} // namespace UE::Flecs
