/*
 * Copyright 2007-2022 CM4all GmbH
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

#include "adata/AssignmentList.hxx"
#include "util/ShallowCopy.hxx"

#include <string_view>

class AllocatorPtr;
class UniqueFileDescriptor;
struct CgroupState;

/**
 * Options for how to configure the cgroups of a process.
 */
struct CgroupOptions {
	/**
	 * The name of the cgroup this process will be moved into.  It
	 * is a name (without slashes) relative to the daemon's scope
	 * cgroup.  For example, it could be an identification of the
	 * hosting account which this process belongs to.
	 */
	const char *name = nullptr;

	/**
	 * Create one child cgroup per session?  Set this to the name
	 * of the session cgroup below the one created by #name above.
	 */
	const char *session = nullptr;

	/**
	 * A list of cgroup extended attributes.  They should usually
	 * be in the "user" namespace.
	 */
	AssignmentList xattr;

	/**
	 * A list of cgroup controller settings.
	 *
	 * The name of the controller setting,
	 * e.g. "cpu.shares".
	 *
	 * The value to be written to the specified setting.
	 */
	AssignmentList set;

	CgroupOptions() = default;
	CgroupOptions(AllocatorPtr alloc, const CgroupOptions &src) noexcept;

	constexpr CgroupOptions(ShallowCopy shallow_copy,
				const CgroupOptions &src) noexcept
		:name(src.name),
		 session(src.session),
		 xattr(shallow_copy, src.xattr),
		 set(shallow_copy, src.set) {}

	CgroupOptions(CgroupOptions &&) = default;
	CgroupOptions &operator=(CgroupOptions &&) = default;

	constexpr bool IsDefined() const noexcept {
		return name != nullptr;
	}

	void SetXattr(AllocatorPtr alloc,
		      std::string_view name, std::string_view value) noexcept;

	void Set(AllocatorPtr alloc,
		 std::string_view name, std::string_view value) noexcept;

	/**
	 * Create a cgroup2 group.  Returns an undefined
	 * #UniqueFileDescriptor if this instance is not enabled or
	 * this is not a cgroup2-only host.
	 *
	 * Throws on error.
	 */
	UniqueFileDescriptor Create2(const CgroupState &state) const;

	/**
	 * Throws std::runtime_error on error.
	 */
	void Apply(const CgroupState &state, unsigned pid) const;

	char *MakeId(char *p) const noexcept;
};
