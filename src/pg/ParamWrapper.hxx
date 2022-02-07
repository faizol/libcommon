/*
 * Copyright 2007-2021 CM4all GmbH
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

#pragma once

#include "Serial.hxx"
#include "BinaryValue.hxx"
#include "Array.hxx"
#include "util/Compiler.h"

#include <array>
#include <concepts>
#include <list>
#include <cinttypes>
#include <cstdio>
#include <cstddef>
#include <string>
#include <tuple>
#include <type_traits>

#if __cplusplus >= 201703L && !GCC_OLDER_THAN(7,0)
#include <string_view>
#endif

namespace Pg {

template<typename T, typename Enable=void>
struct ParamWrapper {
	ParamWrapper(const T &t) noexcept;
	const char *GetValue() const noexcept;

	/**
	 * Is the buffer returned by GetValue() binary?  If so, the method
	 * GetSize() must return the size of the value.
	 */
	bool IsBinary() const noexcept;

	/**
	 * Returns the size of the value in bytes.  Only applicable if
	 * IsBinary() returns true and the value is non-nullptr.
	 */
	size_t GetSize() const noexcept;
};

template<>
struct ParamWrapper<Serial> {
	char buffer[16];

	ParamWrapper(Serial s) noexcept {
		sprintf(buffer, "%" PRIpgserial, s.get());
	}

	const char *GetValue() const noexcept {
		return buffer;
	}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<>
struct ParamWrapper<BinaryValue> {
	BinaryValue value;

	constexpr ParamWrapper(BinaryValue _value) noexcept
		:value(_value) {}

	constexpr const char *GetValue() const noexcept {
		return (const char *)value.data;
	}

	static constexpr bool IsBinary() noexcept {
		return true;
	}

	constexpr size_t GetSize() const noexcept {
		return value.size;
	}
};

template<>
struct ParamWrapper<const char *> {
	const char *value;

	constexpr ParamWrapper(const char *_value) noexcept
		:value(_value) {}

	constexpr const char *GetValue() const noexcept {
		return value;
	}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<>
struct ParamWrapper<int> {
	char buffer[16];

	ParamWrapper(int i) noexcept {
		sprintf(buffer, "%i", i);
	}

	const char *GetValue() const noexcept {
		return buffer;
	}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<>
struct ParamWrapper<int64_t> {
	char buffer[32];

	ParamWrapper(int64_t i) noexcept {
		sprintf(buffer, "%" PRId64, i);
	}

	const char *GetValue() const noexcept {
		return buffer;
	}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<>
struct ParamWrapper<unsigned> {
	char buffer[16];

	ParamWrapper(unsigned i) noexcept {
		sprintf(buffer, "%u", i);
	}

	const char *GetValue() const noexcept {
		return buffer;
	}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<>
struct ParamWrapper<uint64_t> {
	char buffer[32];

	ParamWrapper(int64_t i) noexcept {
		sprintf(buffer, "%" PRIu64, i);
	}

	const char *GetValue() const noexcept {
		return buffer;
	}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<>
struct ParamWrapper<bool> {
	const char *value;

	constexpr ParamWrapper(bool _value) noexcept
		:value(_value ? "t" : "f") {}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	constexpr const char *GetValue() const noexcept {
		return value;
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

#if __cplusplus >= 201703L && !GCC_OLDER_THAN(7,0)

template<>
struct ParamWrapper<std::string_view> {
	std::string_view value;

	constexpr ParamWrapper(std::string_view _value) noexcept
		:value(_value) {}

	static constexpr bool IsBinary() noexcept {
		/* since std::string_view is not null-terminated, we
		   need to pass it as a binary value */
		return true;
	}

	constexpr const char *GetValue() const noexcept {
		return value.data();
	}

	constexpr size_t GetSize() const noexcept {
		return value.size();
	}
};

#endif

template<>
struct ParamWrapper<std::string> : ParamWrapper<const char *> {
	ParamWrapper(const std::string &_value) noexcept
		:ParamWrapper<const char *>(_value.c_str()) {}
};

/**
 * Specialization for STL container types of std::string instances.
 */
template<typename T>
struct ParamWrapper<T,
		    std::enable_if_t<std::is_same<typename T::value_type,
						  std::string>::value>> {
	std::string value;

	ParamWrapper(const T &list) noexcept
		:value(EncodeArray(list)) {}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	const char *GetValue() const noexcept {
		return value.c_str();
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<>
struct ParamWrapper<const std::list<std::string> *> {
	std::string value;

	ParamWrapper(const std::list<std::string> *list) noexcept
		:value(list != nullptr
		       ? EncodeArray(*list)
		       : std::string()) {}

	static constexpr bool IsBinary() noexcept {
		return false;
	}

	const char *GetValue() const noexcept {
		return value.empty() ? nullptr : value.c_str();
	}

	size_t GetSize() const noexcept {
		/* ignored for text columns */
		return 0;
	}
};

template<typename... Params>
class ParamCollector {
	std::tuple<ParamWrapper<Params>...> wrappers;

public:
	explicit ParamCollector(const Params&... params) noexcept
		:wrappers(params...)
	{
	}

	static constexpr bool HasBinary() noexcept {
		return (ParamWrapper<Params>::IsBinary() || ...);
	}

	static constexpr size_t Count() noexcept {
		return sizeof...(Params);
	}

	template<typename O, typename S, typename F>
	size_t Fill(O output, S size, F format) const noexcept {
		return std::apply([&](const auto &...w){
			return (FillOne(w, output, size, format) + ...);
		}, wrappers);
	}

	template<typename O>
	size_t Fill(O output) const noexcept {
		return std::apply([&](const auto &...w){
			return (FillOne(w, output) + ...);
		}, wrappers);
	}

private:
	template<typename W, typename O, typename S, typename F>
	static std::size_t FillOne(const W &wrapper, O &output,
				   S &size, F &format) noexcept {
		*output++ = wrapper.GetValue();
		*size++ = wrapper.GetSize();
		*format++ = wrapper.IsBinary();
		return 1;
	}

	template<typename W, typename O>
	static std::size_t FillOne(const W &wrapper, O &output) noexcept {
		*output++ = wrapper.GetValue();
		return 1;
	}
};

template<typename... Params>
class TextParamArray {
	ParamCollector<Params...> collector;

	static_assert(!decltype(collector)::HasBinary());

public:
	static constexpr size_t count = decltype(collector)::Count();
	std::array<const char *, count> values;

	explicit TextParamArray(const Params&... params) noexcept
		:collector(params...)
	{
		collector.Fill(values.begin());
	}

	constexpr const int *GetLengths() const noexcept {
		return nullptr;
	}

	constexpr const int *GetFormats() const noexcept {
		return nullptr;
	}
};

template<typename... Params>
class BinaryParamArray {
	ParamCollector<Params...> collector;

public:
	static constexpr size_t count = decltype(collector)::Count();
	std::array<const char *, count> values;
	std::array<int, count> lengths;
	std::array<int, count> formats;

	explicit BinaryParamArray(const Params&... params) noexcept
		:collector(params...)
	{
		collector.Fill(values.begin(),
			       lengths.begin(),
			       formats.begin());
	}

	constexpr const int *GetLengths() const noexcept {
		return lengths.data();
	}

	constexpr const int *GetFormats() const noexcept {
		return formats.data();
	}
};

template<typename... Params>
using AutoParamArray = std::conditional_t<ParamCollector<Params...>::HasBinary(),
					  BinaryParamArray<Params...>,
					  TextParamArray<Params...>>;

template<typename T>
concept ParamArray = requires (const T &t) {
	{ t.count } noexcept -> std::convertible_to<std::size_t>;
	{ t.values.data() } noexcept -> std::same_as<const char *const*>;
	{ t.GetLengths() } noexcept -> std::same_as<const int *>;
	{ t.GetFormats() } noexcept -> std::same_as<const int *>;
};

} /* namespace Pg */
