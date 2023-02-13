// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <mk@cm4all.com>

#pragma once

struct lua_State;
class EventLoop;

namespace Lua {

void
InitPgStock(lua_State *L) noexcept;

void
NewPgStock(struct lua_State *L, EventLoop &event_loop,
	   const char *conninfo, const char *schema,
	   unsigned limit, unsigned max_idle) noexcept;

} // namespace Lua
