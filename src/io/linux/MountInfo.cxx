/*
 * Copyright 2014-2022 CM4all GmbH
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

#include "MountInfo.hxx"
#include "lib/fmt/ToBuffer.hxx"
#include "lib/fmt/SystemError.hxx"
#include "util/ScopeExit.hxx"
#include "util/IterableSplitString.hxx"

#include <array>

using std::string_view_literals::operator""sv;

template<std::size_t N>
static void
SplitFill(std::array<std::string_view, N> &dest, std::string_view s, char separator)
{
	std::size_t i = 0;
	for (auto value : IterableSplitString(s, separator)) {
		dest[i++] = value;
		if (i >= dest.size())
			return;
	}

	std::fill(std::next(dest.begin(), i), dest.end(), std::string_view{});
}

static FILE *
Open(const char *path, const char *mode)
{
	FILE *file = fopen(path, mode);
	if (file == nullptr)
		throw FmtErrno("Failed to open {}", path);

	return file;
}

static FILE *
OpenMountInfo(unsigned pid)
{
	return pid > 0
		? Open(FmtBuffer<64>("/proc/{}/mountinfo", pid), "r")
		: Open("/proc/self/mountinfo", "r");
}

struct MountInfoView {
	std::string_view root;
	std::string_view mount_point;
	std::string_view filesystem;
	std::string_view source;

	operator MountInfo() const noexcept {
		return {
			std::string{root},
			std::string{filesystem},
			std::string{source},
		};
	}
};

class MountInfoReader {
	FILE *const file;

	char line[4096];

public:
	explicit MountInfoReader(unsigned pid)
		:file(OpenMountInfo(pid)) {}

	bool ReadLine() noexcept {
		return fgets(line, sizeof(line), file) != nullptr;
	}

	MountInfoView GetLine() const noexcept {
		std::array<std::string_view, 10> columns;
		SplitFill(columns, line, ' ');

		/* skip the optional tagged fields */
		size_t i = 6;
		while (i < columns.size() && columns[i] != "-"sv)
			++i;

		if (i + 2 >= columns.size())
			return {};

		return {
			columns[3],
			columns[4],
			columns[i + 1],
			columns[i + 2],
		};
	}

	class iterator {
		MountInfoReader &reader;

		bool more;

	public:
		iterator(MountInfoReader &_reader, bool _more) noexcept
			:reader(_reader), more(_more) {}

		auto &operator++() noexcept {
			more = reader.ReadLine();
			return *this;
		}

		auto operator*() const noexcept {
			return reader.GetLine();
		}

		bool operator!=(const iterator &other) const noexcept {
			return more || other.more;
		}
	};

	iterator begin() noexcept {
		return {*this, ReadLine()};
	}

	iterator end() noexcept {
		return {*this, false};
	}
};

MountInfo
ReadProcessMount(unsigned pid, const char *_mountpoint)
{
	FILE *const file = OpenMountInfo(pid);
	AtScopeExit(file) { fclose(file); };

	const std::string_view mountpoint(_mountpoint);

	for (const auto &i : MountInfoReader{pid})
		if (i.mount_point == mountpoint)
			return i;

	/* not found: return empty string */
	return {{}, {}, {}};
}