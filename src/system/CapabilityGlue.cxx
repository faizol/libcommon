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

#include "CapabilityGlue.hxx"
#include "CapabilityState.hxx"

bool
IsSysAdmin() noexcept
try {
	const auto s = CapabilityState::Current();
	return s.GetFlag(CAP_SYS_ADMIN, CAP_EFFECTIVE) == CAP_SET;
} catch (...) {
	/* shouldn't happen, but if it does, pretend we're not
	   admin */
	return false;
}

bool
HaveSetuid() noexcept
try {
	const auto s = CapabilityState::Current();
	return s.GetFlag(CAP_SETUID, CAP_EFFECTIVE) == CAP_SET;
} catch (...) {
	/* shouldn't happen */
	return false;
}

bool
HaveNetBindService() noexcept
try {
	const auto s = CapabilityState::Current();
	return s.GetFlag(CAP_NET_BIND_SERVICE, CAP_EFFECTIVE) == CAP_SET;
} catch (...) {
	/* shouldn't happen */
	return false;
}
