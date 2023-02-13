// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <mk@cm4all.com>

#pragma once

struct lua_State;
class EventLoop;

namespace Lua {

void
InitPg(lua_State *L, EventLoop &event_loop);

} // namespace Lua
