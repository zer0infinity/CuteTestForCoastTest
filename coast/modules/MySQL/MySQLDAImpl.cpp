/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MySQLDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "SysLog.h"
#include "Dbg.h"

//--- MySQLDAImpl -----------------------------------------------------
RegisterDataAccessImpl(MySQLDAImpl);

MySQLDAImpl::MySQLDAImpl(const char *name) : DataAccessImpl(name)
{
}

MySQLDAImpl::~MySQLDAImpl()
{
}

IFAObject *MySQLDAImpl::Clone() const
{
	return new MySQLDAImpl(fName);
}

bool MySQLDAImpl::Exec( Context &context, InputMapper *in, OutputMapper *out)
{
	StartTrace(MySQLDAImpl.Exec);

	MYSQL mysql, *sock;
	MYSQL_RES *res;

	mysql_init(&mysql);

	// make connection
	// mysql, localhost or other host, loginname,
	String dataBase;
	in->Get("Database", dataBase, context);

	String user("No user specified!");
	in->Get("MySQLUser", user, context);

	String pw("No password specified!");
	in->Get("MySQLPW", pw, context);

	TraceAny(fConfig, "config");
	String host("localhost");
	in->Get("Host", host, context);
	long port = 3306L;
	in->Get("Port", port, context);

	Trace("trying connect to DB:[" << dataBase << "], with user@host:port [" << user << "@" << host << ":" << port << "], and pass:[" << pw << "]");
	if (!(sock = mysql_real_connect(&mysql, host, user, pw, dataBase, port, NULL, 0))) {
		SetErrorMsg("Couldn't connect to mySQL engine!", &mysql, context, out);
		return false;
	}

	// first part of transaction, assemble query from arguments
	String theQuery;
	OStringStream os(theQuery);
	in->Get("SQL", os, context);
	os.flush();
	SubTrace (Query, "QUERY IS:" << theQuery );

	bool result = true;
	if (mysql_query(sock, theQuery)) {
		SetErrorMsg("Query failed", sock, context, out);
		result = false;
	} else {
		if (!(res = mysql_store_result(sock))) {
			// No results -- was INSERT or UPDATE etc
			Anything queryResults;
			out->Put("QueryResult", queryResults, context);
		} else {
			// process result- res fields are placed into anything...
			long num_fields = mysql_num_fields(res);
			Trace("Number of fields:" << num_fields);
			MYSQL_FIELD *fields = mysql_fetch_fields(res);

			// fill rows
			Anything theSet;
			MYSQL_ROW myRow;
			long rowNum = 0;
			while ( (myRow = mysql_fetch_row(res) ) ) {
				Anything newRow;

				for (long i = 0; i < num_fields; i++) {
					String fieldName = fields[i].name;
					newRow[fieldName] = myRow[i];					// assume only char *
				}
				TraceAny(newRow, "Row");
				Anything rowNumber = rowNum++;
				theSet[rowNumber.AsString("X")] = newRow;
			}
			TraceAny(theSet, "The Set");

			if (!Lookup("NoQueryCount", 0L)) {
				out->Put("QueryCount", rowNum, context);
			}
			out->Put("QueryResult", theSet, context);
		}
		mysql_free_result(res);
	}

	mysql_close(sock);
	if (mysql_errno(sock)) {
		SetErrorMsg("close failed !", sock, context, out);
		result = false;
	}

	return result;
}

void MySQLDAImpl::SetErrorMsg(const char *msg, MYSQL *mysql, Context &context, OutputMapper *out )
{
	StartTrace(MySQLDAImpl.SetErrorMsg);

	String errorMsg(msg);
	errorMsg << " " << mysql_error(mysql);

	out->Put("Error", errorMsg, context);
	SysLog::Error(errorMsg);
	Trace("MySQL Error >" << errorMsg);
}
