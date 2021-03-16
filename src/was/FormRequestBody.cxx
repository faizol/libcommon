/*
 * Copyright 2020-2021 CM4all GmbH
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

#include "FormRequestBody.hxx"
#include "StringRequestBody.hxx"
#include "ExceptionResponse.hxx"
#include "uri/MapQueryString.hxx"
#include "util/MimeType.hxx"
#include "util/StringView.hxx"

#include <was/simple.h>

namespace Was {

[[gnu::pure]]
static bool
IsContentType(was_simple *w, const std::string_view expected) noexcept
{
	const char *content_type = was_simple_get_header(w, "content-type");
	return content_type != nullptr &&
		GetMimeTypeBase(content_type).compare(expected) == 0;
}

std::multimap<std::string, std::string>
FormRequestBodyToMap(was_simple *w, std::string::size_type limit)
{
	if (!was_simple_has_body(w))
		throw BadRequest{"No request body\n"};

	if (!IsContentType(w, "application/x-www-form-urlencoded"))
		throw BadRequest{"Wrong request body type\n"};

	const auto raw = RequestBodyToString(w, limit);
	const std::string_view v(raw);
	return MapQueryString(v);
}

} // namespace Was