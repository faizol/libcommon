// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <mk@cm4all.com>

/*
 * Implementation of D. J. Bernstein's cdb hash function.
 * http://cr.yp.to/cdb/cdb.txt
 */

#pragma once

#include <cstdint>
#include <span>

[[gnu::pure]]
uint32_t
djb_hash(std::span<const std::byte> src) noexcept;

[[gnu::pure]]
uint32_t
djb_hash_string(const char *p) noexcept;
