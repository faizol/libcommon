/*
 * Copyright 2017 Content Management AG
 * All rights reserved.
 *
 * author: Max Kellermann <mk@cm4all.com>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * - Redistributions of source code must retain the above copyright
 * notice, this list of conditions and the following disclaimer.
 *
 * - Redistributions in binary form must reproduce the above copyright
 * notice, this list of conditions and the following disclaimer in the
 * documentation and/or other materials provided with the
 * distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED.  IN NO EVENT SHALL THE
 * FOUNDATION OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * Implementation of the Fowler-Noll-Vo hash function.
 */

#ifndef FNV_HASH_HXX
#define FNV_HASH_HXX

#include "util/ConstBuffer.hxx"
#include "Compiler.h"

#include <stddef.h>
#include <stdint.h>

template<typename T>
struct FNVTraits {};

template<>
struct FNVTraits<uint32_t> {
	typedef uint32_t value_type;
	using fast_type = uint_fast32_t;

	static constexpr value_type OFFSET_BASIS = 2166136261u;
	static constexpr value_type PRIME = 16777619u;
};

template<>
struct FNVTraits<uint64_t> {
	using value_type = uint64_t;
	using fast_type = uint_fast64_t;

	static constexpr value_type OFFSET_BASIS = 14695981039346656037u;
	static constexpr value_type PRIME = 1099511628211u;
};

template<typename Traits>
struct FNV1aAlgorithm {
	using value_type = typename Traits::value_type;
	using fast_type = typename Traits::fast_type;

	gcc_hot
	static constexpr fast_type Update(fast_type hash, uint8_t b) noexcept {
		return (hash ^ b) * Traits::PRIME;
	}

	gcc_pure gcc_hot
	static value_type StringHash(const char *s) noexcept {
		using Algorithm = FNV1aAlgorithm<Traits>;

		fast_type hash = Traits::OFFSET_BASIS;
		while (*s)
			hash = Algorithm::Update(hash, *s++);

		return hash;
	}

	gcc_pure gcc_hot
	static constexpr value_type BinaryHash(ConstBuffer<uint8_t> s) noexcept {
		using Algorithm = FNV1aAlgorithm<Traits>;

		fast_type hash = Traits::OFFSET_BASIS;
		for (auto b : s)
			hash = Algorithm::Update(hash, b);

		return hash;
	}

	gcc_pure gcc_hot
	static constexpr value_type BinaryHash(ConstBuffer<void> s) noexcept {
		return BinaryHash(ConstBuffer<uint8_t>::FromVoid(s));
	}
};

gcc_pure gcc_hot
inline uint32_t
FNV1aHash32(const char *s) noexcept
{
	using Traits = FNVTraits<uint32_t>;
	using Algorithm = FNV1aAlgorithm<Traits>;
	return Algorithm::StringHash(s);
}

gcc_pure gcc_hot
constexpr uint32_t
FNV1aHash32(ConstBuffer<void> s) noexcept
{
	using Traits = FNVTraits<uint32_t>;
	using Algorithm = FNV1aAlgorithm<Traits>;
	return Algorithm::BinaryHash(s);
}

gcc_pure gcc_hot
inline uint64_t
FNV1aHash64(const char *s) noexcept
{
	using Traits = FNVTraits<uint64_t>;
	using Algorithm = FNV1aAlgorithm<Traits>;
	return Algorithm::StringHash(s);
}

gcc_pure gcc_hot
constexpr uint64_t
FNV1aHash64(ConstBuffer<void> s) noexcept
{
	using Traits = FNVTraits<uint64_t>;
	using Algorithm = FNV1aAlgorithm<Traits>;
	return Algorithm::BinaryHash(s);
}

gcc_pure gcc_hot
inline uint32_t
FNV1aHashFold32(const char *s) noexcept
{
	const uint64_t h64 = FNV1aHash64(s);

	/* XOR folding */
	const uint_fast32_t lo(h64);
	const uint_fast32_t hi(h64 >> 32);
	return lo ^ hi;
}

#endif
