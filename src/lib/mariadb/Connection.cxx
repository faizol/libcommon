// SPDX-License-Identifier: BSD-2-Clause
// Copyright CM4all GmbH
// author: Max Kellermann <mk@cm4all.com>

#include "Connection.hxx"
#include "Statement.hxx"
#include "util/RuntimeError.hxx"

#include <stdexcept>

void
MysqlConnection::Connect(const char *host,
			 const char *user, const char *passwd,
			 const char *db, unsigned int port,
			 const char *unix_socket,
			 unsigned long clientflag)
{
	if (!mysql_real_connect(&mysql, host, user, passwd, db, port,
				unix_socket, clientflag))
		throw std::runtime_error{mysql_error(&mysql)};
}

void
MysqlConnection::Query(std::string_view sql)
{
	if (mysql_real_query(&mysql, sql.data(), sql.size()) != 0)
		throw FormatRuntimeError("mysql_real_query() failed: %s",
					 mysql_error(&mysql));
}

MysqlResult
MysqlConnection::StoreResult()
{
	auto *r = mysql_store_result(&mysql);
	if (r == nullptr && mysql_errno(&mysql) != 0)
		throw FormatRuntimeError("mysql_store_result() failed: %s",
					 mysql_error(&mysql));

	return MysqlResult{r};
}

bool
MysqlConnection::NextResult()
{
	int r = mysql_next_result(&mysql);
	if (r > 0)
		throw FormatRuntimeError("mysql_next_result() failed: %s",
					 mysql_error(&mysql));

	return r == 0;
}

MysqlStatement
MysqlConnection::Prepare(std::string_view sql)
{
	MysqlStatement stmt{mysql};
	stmt.Prepare(sql);
	return stmt;
}
