/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCT.h"

//--- standard modules used ----------------------------------------------------
#include "Threads.h"
#include "Dbg.h"
#include "SysLog.h"
#include "StringStream.h"
#include "DiffTimer.h"

//--- c-library modules used ---------------------------------------------------
#include <limits.h>

//---- SybCT ----------------------------------------------------------------
Mutex SybCT::fgSybaseLocker("SybaseLocker");

CS_RETCODE SybCT_csmsg_handler(CS_CONTEXT *context, CS_CLIENTMSG *errmsg);
CS_RETCODE SybCT_clientmsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg);
CS_RETCODE SybCT_servermsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg);

ColumnData::ColumnData(Allocator *a)
	: indicator(NULL)
	, value(NULL)
	, valuelen(NULL)
	, fAllocator((a) ? a : Storage::Current())
{
}

ColumnData::~ColumnData()
{
	(Storage::Global())->Free(value);
	(Storage::Global())->Free(valuelen);
	(Storage::Global())->Free(indicator);
}

void *ColumnData::operator new[](size_t size, Allocator *a)
{
	if (a) {
		void *mem = a->Calloc(1, size + sizeof(Allocator *));
		((Allocator **)mem)[0L] = a;				// remember address of responsible Allocator
		return (char *)mem + sizeof(Allocator *); // needs cast because of pointer arithmetic
	} else {
		return 0;
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void ColumnData::operator delete[](void *ptr, Allocator *a)
{
	if (ptr) {
		void *realPtr = (char *)ptr - sizeof(Allocator *);
		a->Free(realPtr);
	}
	return;
}
#endif

void ColumnData::operator delete[](void *ptr)
{
	if (ptr) {
		void *realPtr = (char *)ptr - sizeof(Allocator *);
		Allocator *a = ((Allocator **)realPtr)[0L];	// retrieve Allocator
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		ColumnData::operator delete[](ptr, a);
#else
		a->Free(realPtr);
#endif
	}
	return;
}

SybCT::SybCT(CS_CONTEXT *context)
	: fContext(context)
	, fConnection(NULL)
	, fpMessages(NULL)
{
	StartTrace(SybCT.SybCT);
}

SybCT::~SybCT()
{
}

CS_RETCODE SybCT::Init(CS_CONTEXT **context, Anything *pMessages, const String &strInterfacesPathName)
{
	StartTrace(SybCT.Init);
	CS_RETCODE retcode = CS_FAIL;

	Trace("Get a context handle to use.");
	retcode = cs_ctx_alloc(CTLIB_VERSION, context);
	if (retcode != CS_SUCCEED) {
		SysLog::Error("SybCT::Init: cs_ctx_alloc() failed");
		return retcode;
	}

	Trace("Initialize Open Client.");
	retcode = ct_init(*context, CTLIB_VERSION);
	if (retcode != CS_SUCCEED) {
		SysLog::Error("SybCT::Init: ct_init() failed");
		cs_ctx_drop(*context);
		*context = NULL;
		return retcode;
	}

#ifdef EX_API_DEBUG
	// ct_debug stuff. Enable this function right before any call to
	// OC-Lib that is returning failure.
	retcode = ct_debug(*context, NULL, CS_SET_FLAG, CS_DBG_API_STATES, NULL, CS_UNUSED);
	if (retcode != CS_SUCCEED) {
		SysLog::Error("SybCT::Init: ct_debug() failed");
	}
#endif

	Trace("Install anything to catch context messages");
	if (retcode == CS_SUCCEED) {
		retcode = cs_config(*context, CS_SET, CS_USERDATA, &pMessages, CS_SIZEOF(Anything *), (CS_INT *)NULL);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("SybCT::Init: cs_config(userdata) failed");
		}
	}

	Trace("Install cs-library message handler.");
	if (retcode == CS_SUCCEED) {
		retcode = cs_config(*context, CS_SET, CS_MESSAGE_CB, (CS_VOID *)SybCT_csmsg_handler, CS_UNUSED, NULL);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("SybCT::Init: cs_config(csmsg) failed");
		}
	}

	Trace("Install client message handler.");
	if (retcode == CS_SUCCEED) {
		retcode = ct_callback(*context, NULL, CS_SET, CS_CLIENTMSG_CB, (CS_VOID *)SybCT_clientmsg_handler);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("SybCT::Init: ct_callback(clientmsg) failed");
		}
	}

	Trace("Install server message handler.");
	if (retcode == CS_SUCCEED) {
		retcode = ct_callback(*context, NULL, CS_SET, CS_SERVERMSG_CB, (CS_VOID *)SybCT_servermsg_handler);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("SybCT::Init: ct_callback(servermsg) failed");
		}
	}

	// This is a synchronous example so set the input/output type
	// to synchronous (This is the default setting, but show an example anyway).
	Trace("set context to synchronous IO");
	if (retcode == CS_SUCCEED) {
		CS_INT netio_type = CS_SYNC_IO;
		retcode = ct_config(*context, CS_SET, CS_NETIO, &netio_type, CS_UNUSED, NULL);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("SybCT::Init: ct_config(netio) failed");
		}
	}
	if (retcode == CS_SUCCEED) {
		CS_INT maxCons;
		retcode = ct_config(*context, CS_GET, CS_MAX_CONNECT, &maxCons, CS_UNUSED, NULL);
		if (retcode == CS_SUCCEED) {
			Trace("### MAX_CONNECTIONS: " << maxCons << " ###");
			SysLog::Info(String("MAX_CONNECTIONS: ") << (long)maxCons);
		}
	}
	if (retcode == CS_SUCCEED && strInterfacesPathName.Length()) {
		retcode = ct_config(*context, CS_SET, CS_IFILE, (CS_VOID *)(const char *)strInterfacesPathName, strInterfacesPathName.Length(), NULL);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("SybCT::Init: ct_config(cs_ifile) failed");
		}
	}

	if (retcode != CS_SUCCEED) {
		ct_exit(*context, CS_FORCE_EXIT);
		cs_ctx_drop(*context);
		*context = NULL;
	}

	return retcode;
}

CS_RETCODE SybCT::Finis(CS_CONTEXT *context)
{
	StartTrace(SybCT.Finis);
	CS_RETCODE	retcode;

	retcode = ct_exit(context, CS_UNUSED);
	if (retcode != CS_SUCCEED) {
		SysLog::Error("SybCT::Finis: ct_exit() failed");
		return retcode;
	}
	retcode = cs_ctx_drop(context);
	if (retcode != CS_SUCCEED) {
		SysLog::Error("SybCT::Finis: cs_ctx_drop() failed");
		return retcode;
	}
	return retcode;
}

bool SybCT::Open(Anything *pMessages, String user, String password, String server, String appl)
{
	StartTrace(SybCT.Open);
	CS_RETCODE	retcode;

	Assert(pMessages != NULL);
	fpMessages = pMessages;
	fQueryResults = Anything();
	fQueryColNames = Anything();
	// Allocate a connection structure.
	retcode = ct_con_alloc(fContext, &fConnection);
	if (retcode != CS_SUCCEED) {
		Error("Open: ct_con_alloc failed");
		return false;
	}

	// Put this thread number in fConnection user data for later use.
	{
		MutexEntry me(fgSybaseLocker);
		me.Use();
		retcode = ct_con_props(fConnection, CS_SET, CS_USERDATA, &fpMessages, CS_SIZEOF(Anything *), NULL);
	}
	String hexnum;
	hexnum.AppendAsHex((unsigned char *)&fpMessages, 4);
	Trace("Anything set into connection, with addr: " << hexnum);

	if (retcode != CS_SUCCEED) {
		Error("Open: ct_con_props(CS_SET CS_USERDATA) failed");
		return false;
	}

	// If a username is defined, set the CS_USERNAME property.
	if (retcode == CS_SUCCEED && user.Length()) {
		MutexEntry me(fgSybaseLocker);
		me.Use();
		if ((retcode = ct_con_props(fConnection, CS_SET, CS_USERNAME, (char *)(const char *)user, CS_NULLTERM, NULL)) != CS_SUCCEED) {
			Error("Open: ct_con_props(username) failed");
		}
	} else {
		Warning("Open: Username missing");
	}

	// If a password is defined, set the CS_PASSWORD property.
	if (retcode == CS_SUCCEED && password.Length()) {
		MutexEntry me(fgSybaseLocker);
		me.Use();
		if ((retcode = ct_con_props(fConnection, CS_SET, CS_PASSWORD, (char *)(const char *)password, CS_NULLTERM, NULL)) != CS_SUCCEED) {
			Error("Open: ct_con_props(password) failed");
		}
	} else {
		Warning("Open: Password missing");
	}

	// Set the CS_APPNAME property.
	if (retcode == CS_SUCCEED && appl.Length()) {
		MutexEntry me(fgSybaseLocker);
		me.Use();
		if ((retcode = ct_con_props(fConnection, CS_SET, CS_APPNAME, (char *)(const char *)appl, CS_NULLTERM, NULL)) != CS_SUCCEED) {
			Error("Open: ct_con_props(appname) failed");
		}
	} else {
		Warning("Open: Applicationname missing");
	}

	// Open a Server fConnection.
	if (retcode == CS_SUCCEED && server.Length()) {
		MutexEntry me(fgSybaseLocker);
		me.Use();
		retcode = ct_connect(fConnection, (char *)(const char *)server, CS_NULLTERM);
		if (retcode != CS_SUCCEED) {
			Error("Open: ct_connect failed");
		}
	} else {
		Warning("Open: Servername missing");
	}

	if (retcode != CS_SUCCEED) {
		ct_con_drop(fConnection);
		fConnection = NULL;
	}
	return (retcode == CS_SUCCEED);
}

bool SybCT::SqlExec(String query, String resultformat, long lMaxResultSize )
{
	StartTrace(SybCT.SqlExec);
	Trace("dbcmd: [" << query << "]");

	CS_RETCODE	retcode;
	CS_COMMAND	*cmd = NULL;

	// Allocate a command handle to send the query
	if ((retcode = ct_cmd_alloc(fConnection, &cmd)) != CS_SUCCEED) {
		Error("SqlExec: ct_cmd_alloc() failed");
		return false;
	}

	// Initiate the command with the specified query
	retcode = ct_command(cmd, CS_LANG_CMD, (char *)(const char *)query, CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED) {
		Error(String("SqlExec: ct_command(CS_LANG_CMD:") << query << ") failed");
		(void)ct_cmd_drop(cmd);
		return false;
	}

	{
		long lDiff = 0L;
		DiffTimer aTimer;
		// Send the command to the server
		if ((retcode = ct_send(cmd)) != CS_SUCCEED) {
			Error("DoSqlExec: ct_send() failed");
			(void)ct_cmd_drop(cmd);
			return false;
		}
		lDiff = aTimer.Diff();
		Trace("cmd execution: " << lDiff << "ms");
	}

	// Process the results. Loop while ct_results() returns CS_SUCCEED.
	CS_INT res_type;
	CS_RETCODE	query_code = CS_SUCCEED;
	while ((retcode = ct_results(cmd, &res_type)) == CS_SUCCEED) {
		switch ((int)res_type) {
			case CS_CMD_SUCCEED:
				// This means no rows were returned.
			case CS_CMD_DONE:
				// This means we're done with one result set.
				break;
			case CS_CMD_FAIL:
				// This means that the server encountered an error while
				// processing our command.
				Error("SqlExec: ct_results() returned CMD_FAIL");
				query_code = CS_FAIL;
				break;
			case CS_MSG_RESULT: {
				CS_SMALLINT	msg_id;
				// the result was a message
				retcode = ct_res_info(cmd, CS_MSGTYPE, (CS_VOID *)&msg_id, CS_UNUSED, NULL);
				if (retcode != CS_SUCCEED) {
					Error("SqlExec: ct_res_info(msgtype) failed");
					query_code = CS_FAIL;
				} else {
					SysLog::Info(String("SybCT::SqlExec: we got a MessageResult with id:") << (long)msg_id);
				}
				break;
			}
			case CS_STATUS_RESULT:
			case CS_CURSOR_RESULT:
			case CS_ROW_RESULT:
			case CS_PARAM_RESULT:
				// write an Info on which result type we got
				switch ((int)res_type) {
					case  CS_ROW_RESULT:
						Trace("CS_ROW_RESULT");
						break;
					case  CS_CURSOR_RESULT:
						Trace("CS_CURSOR_RESULT");
						break;
					case  CS_PARAM_RESULT:
						Trace("CS_PARAM_RESULT");
						break;
					case  CS_STATUS_RESULT:
						Trace("CS_STATUS_RESULT");
						break;
				}
				{
					long lDiff = 0L;
					{
						DiffTimer aTimer;
						retcode = DoFetchData(cmd, res_type, resultformat, lMaxResultSize);
						lDiff = aTimer.Diff();
					}
					Trace("fetching data: " << lDiff << "ms");
				}
				if (retcode == CS_MEM_ERROR) {
					Warning("SqlExec: rows limited due to MemoryLimit!");
					if (fpMessages) {
						(*fpMessages)["MainMsgErr"] = String("Query aborted! Query would exceed memory limit of ") << lMaxResultSize << " kB!";
						(*fpMessages)["MainMsgErrNumber"] = 49152L;
					}
					query_code = retcode;
				} else if (retcode != CS_SUCCEED) {
					Error("SqlExec: ex_fetch_data() failed");
					query_code = CS_FAIL;
				}
				break;
			default:
				// We got an unexpected result type.
				Error("SqlExec: ct_results() returned unexpected result type");
				query_code = CS_FAIL;
		}
		if (query_code == CS_FAIL || query_code == CS_MEM_ERROR) {
			// Terminate results processing and break out of the results loop
			retcode = ct_cancel(NULL, cmd, CS_CANCEL_ALL);
			if (retcode != CS_SUCCEED) {
				Error("SqlExec: ct_cancel() failed");
			}
			break;
		}
	}

	// We're done processing results. Let's check the return value of ct_results() to see if everything
	// went ok.
	switch ((int)retcode) {
		case CS_END_RESULTS:
			Trace("CS_END_RESULTS");
			// Everything went fine.
			retcode = ct_cmd_drop(cmd);
			if (retcode != CS_SUCCEED) {
				query_code = CS_FAIL;
			}
			break;
		case CS_SUCCEED:
			Trace("CS_SUCCEED");
			// probably set from ct_cancel: used to terminate query early because of MemoryLimit
			retcode = ct_cmd_drop(cmd);
			if (retcode != CS_SUCCEED) {
				Trace("CS_FAIL");
				query_code = CS_FAIL;
			}
			break;
		case CS_FAIL:
			Trace("CS_FAIL");
			// Something terrible happened.
		default:
			Trace("default: retCode:" << (long)retcode);
			(void)ct_cmd_drop(cmd);
			query_code = CS_FAIL;
			// We got an unexpected return value.
			Error("SqlExec: ct_results() failed");
	}

	return (query_code == CS_SUCCEED);
}

CS_RETCODE SybCT::DoFetchData(CS_COMMAND *cmd, const CS_INT res_type, const String &resultformat, const long &lMaxResultSize)
{
	StartTrace(SybCT.DoFetchData);

	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			rows_read;
	CS_DATAFMT		*datafmt;
	EX_COLUMN_DATA	*coldata;
	CS_CONNECTION	*connection;
	CS_INT			rowsize;
	CS_INT			num_rows;	// number of rows to get in a single fetch
	CS_INT			lRowCount = 0;

	connection = (CS_CONNECTION *)NULL;
	rowsize = 0;

	// Get parent connection
	retcode = ct_cmd_props(cmd, CS_GET, CS_PARENT_HANDLE, &connection, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED) {
		Error("DoFetchData: ct_cmd_props(CS_PARENT_HANDLE) failed");
		return retcode;
	}

	// Find out how many columns there are in this result set.
	retcode = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED) {
		Error("DoFetchData: ct_res_info() failed");
		return retcode;
	}

	Trace("we have to process " << (long)num_cols << " columns");
	// Make sure we have at least one column
	if (num_cols <= 0) {
		Error("DoFetchData: ct_res_info() returned zero columns");
		return CS_FAIL;
	}

	// Our program variable, called 'coldata', is an array of
	// EX_COLUMN_DATA structures. Each array element represents
	// one column.  Each array element will re-used for each row set.
	// In coldata, we allocate an array of values and thus use
	// array binding.
	//
	// First, allocate memory for the data element to process.
	coldata = new (Storage::Global()) EX_COLUMN_DATA[num_cols];
	if (coldata == NULL) {
		Error("DoFetchData: (Storage::Global())->Malloc() failed");
		return CS_MEM_ERROR;
	}

	datafmt = (CS_DATAFMT *)(Storage::Global())->Malloc(num_cols * sizeof (CS_DATAFMT));
	if (datafmt == NULL) {
		Error("DoFetchData: (Storage::Global())->Malloc() failed");
		(Storage::Global())->Free(coldata);
		return CS_MEM_ERROR;
	}

	// Loop through the columns getting a description of each one
	// and binding each one to a program variable.
	//
	// We're going to bind each column to a character string;
	// this will show how conversions from server native datatypes
	// to strings can occur via bind.
	//
	// We're going to use the same datafmt structure for both the describe
	// and the subsequent bind.
	//
	// If an error occurs within the for loop, a break is used to get out
	// of the loop and the data that was allocated is free'd before
	// returning.
	for (i = 0; i < num_cols; i++) {
		// Get the column description.  ct_describe() fills the
		// datafmt parameter with a description of the column.
		retcode = ct_describe(cmd, (i + 1), &datafmt[i]);
		if (retcode != CS_SUCCEED) {
			Error("DoFetchData: ct_describe() failed");
			break;
		}

		// update the datafmt structure to indicate that we want the
		// results in a null terminated character string.
		//
		// First, update datafmt.maxlength to contain the maximum
		// possible length of the column. To do this, call
		// ex_display_len() to determine the number of bytes needed
		// for the character string representation, given the
		// datatype described above.  Add one for the null
		// termination character.
		datafmt[i].maxlength = DisplayDlen(&datafmt[i]) + 1;

		// Set datatype and format to tell bind we want things
		// converted to null terminated strings
		datafmt[i].datatype = CS_CHAR_TYPE;
		datafmt[i].format   = CS_FMT_NULLTERM;

		rowsize += MEM_ALIGN_SIZE(datafmt[i].maxlength);
	}
	Trace("total rowsize needed: " << (long)rowsize << " bytes");
	if (retcode != CS_SUCCEED) {
		delete[] coldata;
		(Storage::Global())->Free(datafmt);
		return retcode;
	}

	// adding needed space for valuelen and indicator per column
	rowsize += (MEM_ALIGN_SIZE(sizeof(CS_INT)) + MEM_ALIGN_SIZE(sizeof(CS_SMALLINT))) * num_cols;
	Trace("total rowsize needed after adding additional space: " << (long)rowsize << " bytes");
	// Find max. number of rows that we want to obtain in a single
	// fetch (Why? - just to limit memory being used).
	if (MAX_MEM_BLOCK_SIZE < rowsize) {
		// no row fetchable
		Error(String("DoFetchData: MAX_MEM_BLOCK_SIZE (") << (long)MAX_MEM_BLOCK_SIZE << " bytes) too small for query, needs " << rowsize << " bytes!");
		retcode = CS_CMD_FAIL;
		delete[] coldata;
		(Storage::Global())->Free(datafmt);
		return retcode;
	} else {
		num_rows = MAX_MEM_BLOCK_SIZE / rowsize;
	}
	Trace("max number of rows to fetch at once: " << (long)num_rows);
	// calculate max amount of rows to read without exceeding a given memory limit
	long lMaxRows;
	if (lMaxResultSize == 0) {
		lMaxRows = LONG_MAX;
	} else {
		lMaxRows = (lMaxResultSize * 1024) / rowsize;
		if (lMaxRows < num_rows) {
			num_rows = lMaxRows;
		}
	}
	Trace("max number of rows to totally fetch: " << lMaxRows);

	for (i = 0; i < num_cols; i++) {
		Trace("processing column [" << datafmt[i].name << "]");
		// Allocate memory for the column string
		Trace("value mem: " << (long)num_rows << " rows with maxlength of " << (long)datafmt[i].maxlength << " bytes");
		coldata[i].value = (CS_CHAR *)(Storage::Global())->Malloc(datafmt[i].maxlength * num_rows);
		if (coldata[i].value == NULL) {
			Error("DoFetchData: (Storage::Global())->Malloc() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		// Allocate memory for the valuelen
		Trace("valuelen mem: " << (long)num_rows << " rows with length of " << (long)sizeof(CS_INT) << " bytes");
		coldata[i].valuelen = (CS_INT *)(Storage::Global())->Malloc(sizeof(CS_INT) * num_rows);
		if (coldata[i].valuelen == NULL) {
			Error("DoFetchData: (Storage::Global())->Malloc() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		// Allocate memory for the indicator
		Trace("indicator mem: " << (long)num_rows << " rows with length of " << (long)sizeof(CS_SMALLINT) << " bytes");
		coldata[i].indicator = (CS_SMALLINT *)(Storage::Global())->Malloc(sizeof(CS_SMALLINT) * num_rows);
		if (coldata[i].indicator == NULL) {
			Error("DoFetchData: (Storage::Global())->Malloc() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		// Now do an array bind.
		Trace("binding column");
		datafmt[i].count = num_rows;
		retcode = ct_bind(cmd, (i + 1), &datafmt[i], coldata[i].value, coldata[i].valuelen, coldata[i].indicator);
		if (retcode != CS_SUCCEED) {
			Error("DoFetchData: ct_bind() failed");
			break;
		}
	}
	if (retcode != CS_SUCCEED) {
		delete[] coldata;
		(Storage::Global())->Free(datafmt);
		return retcode;
	}
	// Fetch the rows.  Loop while ct_fetch() returns CS_SUCCEED or  CS_ROW_FAIL
	long lDiff = 0L;
	{
		DiffTimer aTimer;
		retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, &rows_read);
		lDiff = aTimer.Diff();
	}
	while (retcode == CS_SUCCEED || retcode == CS_ROW_FAIL) {
		// Increment our row count by the number of rows just fetched.
		lRowCount += rows_read;

		// Check if we hit a recoverable error.
		if (retcode == CS_ROW_FAIL) {
			Error(String("DoFetchData: Error on row ") << (long)lRowCount);
		}

		DoFillResults(rows_read, num_cols, datafmt, coldata, res_type, resultformat );
		if ((lRowCount + num_rows) > lMaxRows) {
			retcode = CS_MEM_ERROR;
			break;
		} else {
			DiffTimer aTimer;
			retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, &rows_read);
			lDiff += aTimer.Diff();
		}

		Trace(String() << (long)lRowCount << " rows read so far");
	}
	Trace("total fetch execution time: " << lDiff << "ms");

	// Free allocated space.
	delete[] coldata;
	(Storage::Global())->Free(datafmt);

	// We're done processing rows.  Let's check the final return value of ct_fetch().
	switch ((int)retcode) {
		case CS_END_DATA:
			// Everything went fine.
			Trace("All done processing rows - total " << (long)lRowCount);
			retcode = CS_SUCCEED;
			break;

		case CS_MEM_ERROR:
			// early stop due to memory limitations
			Trace("processed only " << (long)lRowCount << " rows");
			break;

		case CS_FAIL:
			// Something terrible happened.
			if ((lRowCount + num_rows) > lMaxRows) {
				Warning(String("DoFetchData: rows limited due to memory limit (") << lMaxResultSize << " kB)");
			} else {
				Error("DoFetchData: ct_fetch() failed");
			}
			// NOTREACHED
			break;

		default:
			// We got an unexpected return value.
			Error("DoFetchData: ct_fetch() returned an unexpected retcode");
			// NOTREACHED
			break;
	}
	return retcode;
}

CS_INT SybCT::DisplayDlen(CS_DATAFMT *column)
{
	CS_INT		len;

	switch ((int) column->datatype) {
		case CS_CHAR_TYPE:
		case CS_VARCHAR_TYPE:
		case CS_TEXT_TYPE:
		case CS_IMAGE_TYPE:
			len = MIN(column->maxlength, MAX_CHAR_BUF);
			break;

		case CS_BINARY_TYPE:
		case CS_VARBINARY_TYPE:
			len = MIN((2 * column->maxlength) + 2, MAX_CHAR_BUF);
			break;

		case CS_BIT_TYPE:
		case CS_TINYINT_TYPE:
			len = 3;
			break;

		case CS_SMALLINT_TYPE:
			len = 6;
			break;

		case CS_INT_TYPE:
			len = 11;
			break;

		case CS_REAL_TYPE:
		case CS_FLOAT_TYPE:
			len = 20;
			break;

		case CS_MONEY_TYPE:
		case CS_MONEY4_TYPE:
			len = 24;
			break;

		case CS_DATETIME_TYPE:
		case CS_DATETIME4_TYPE:
			len = 30;
			break;

		case CS_NUMERIC_TYPE:
		case CS_DECIMAL_TYPE:
			len = (CS_MAX_PREC + 2);
			break;

		default:
			len = 12;
			break;
	}

	return MAX(strlen(column->name) + 1, (unsigned)len);
}

CS_RETCODE SybCT::DoFillResults(CS_INT numrows, CS_INT numcols, CS_DATAFMT *colfmt, EX_COLUMN_DATA *coldata, const CS_INT res_type, const String &resultformat )
{
	StartTrace(SybCT.DoFillResults);
	CS_INT row, col, anyIdx;

	Trace("resultformat [" << resultformat << "]");
	if ( res_type == CS_STATUS_RESULT ) {
		Trace("CS_STATUS_RESULT: stored procedure return code");
		fQueryResults["SP_Retcode"] = EX_GET_COLUMN_VALUE(coldata, 0, 0, colfmt);
	} else if (resultformat.IsEqual("TitlesOnce")) {
		for (col = 0; col < numcols; col++) {
			Trace("colname:[" << colfmt[col].name << "]" );
			fQueryColNames[colfmt[col].name] = (long)col;
		}

		TraceAny(fQueryColNames, "QUERYCOLNAMES is :");

		anyIdx = fQueryResults.GetSize();
		for (row = 0; row < numrows; row++) {
			// We have a row.  Loop through the columns displaying the column values.
			Anything temp = Anything();
			for (col = 0; col < numcols; col++) {
				// the following would be for a NULL value
				// fQueryResults[String() << (long)(row)][colfmt[col].name] = Anything();
				temp.Append( EX_GET_COLUMN_VALUE(coldata, row, col, colfmt) );
			}
			fQueryResults.Append( temp );
		}
		TraceAny(fQueryResults, "QUERYRESULT is :");
	} else {
		anyIdx = fQueryResults.GetSize();
		// need correction when SP_Retcode is defined
		anyIdx -= (fQueryResults.IsDefined("SP_Retcode") ? 1 : 0);
		for (row = 0; row < numrows; row++) {
			// We have a row.  Loop through the columns displaying the column values.
			for (col = 0; col < numcols; col++) {
				// the following would be for a NULL value
				// fQueryResults[String() << (long)(row)][colfmt[col].name] = Anything();
				Trace("colname:[" << colfmt[col].name << "] data:[" << EX_GET_COLUMN_VALUE(coldata, row, col, colfmt) << "]");
				fQueryResults[String() << (long)(anyIdx + row)][colfmt[col].name] = EX_GET_COLUMN_VALUE(coldata, row, col, colfmt);
			}
		}
	}

	return (CS_SUCCEED);
}

bool SybCT::GetResult( Anything &anyResult, Anything &anyTitles )
{
	StartTrace(SybCT.GetResult);
	anyResult = fQueryResults;
	anyTitles = fQueryColNames;
	TraceAny(anyTitles, "Titles are");
	TraceAny(anyResult, "Results are");
	return true;
}

bool SybCT::Close(bool bForce)
{
	StartTrace(SybCT.Close);

	CS_RETCODE retcode = CS_SUCCEED;
	if ( fConnection ) {
		CS_INT	close_option;

		close_option = bForce ? CS_FORCE_CLOSE : CS_UNUSED;
		retcode = ct_close(fConnection, close_option);
		if (retcode != CS_SUCCEED) {
			Error("Close: ct_close() failed");
		} else {
			retcode = ct_con_drop(fConnection);
			if (retcode != CS_SUCCEED) {
				Error("Close: ct_con_drop() failed");
			}
		}
		fConnection = NULL;
	}
	return (retcode == CS_SUCCEED);
}

void SybCT::Warning(String str)
{
	StartTrace(SybCT.Warning);
	String strErr("SybCT::");
	strErr << str;
	SysLog::Warning(strErr);
	if (fpMessages) {
		(*fpMessages)["Messages"].Append(strErr);
	}
}

void SybCT::Error(String str)
{
	StartTrace(SybCT.Error);
	String strErr("SybCT::");
	strErr << str;
	SysLog::Error(strErr);
	if (fpMessages) {
		(*fpMessages)["Messages"].Append(strErr);
	}
}

bool SybCT::GetMessageAny(CS_CONTEXT *context, CS_CONNECTION *connection, Anything **anyMessage)
{
	StartTrace(SybCT.GetMessageAny);
	CS_RETCODE retcode = CS_FAIL;

	if (connection != (CS_CONNECTION *)NULL) {
		retcode = ct_con_props(connection, CS_GET, CS_USERDATA, anyMessage, CS_SIZEOF(Anything *), (CS_INT *)NULL);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("GetMessageAny: ct_con_props(CS_GET CS_USERDATA) failed, MessageAny not available");
		}
	} else {
		retcode = cs_config(context, CS_GET, CS_USERDATA, anyMessage, CS_SIZEOF(Anything *), (CS_INT *)NULL);
		if (retcode != CS_SUCCEED) {
			SysLog::Error("GetMessageAny: cs_config(CS_GET CS_USERDATA) failed");
		}
	}

	return (retcode == CS_SUCCEED);
}

CS_RETCODE SybCT_servermsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg)
{
	StartTrace(SybCT.SybCT_servermsg_handler);

	if ( (long)srvmsg->severity != 0L ) {
		Anything *pAny = NULL;
		if (SybCT::GetMessageAny(context, connection, &pAny)) {
			Anything anyData;
			anyData["severity"] = (long)srvmsg->severity;
			anyData["msgno"] = (long)srvmsg->msgnumber;
			anyData["msgstate"] = (long)srvmsg->state;
			String text = NotNull(srvmsg->text);
			// if a "\n" trails the text, cut it away
			if (text[text.Length()-1L] == '\n') {
				text.Trim(text.Length() - 1);
			}
			anyData["msgtext"] = text;
			anyData["line"] = (long)srvmsg->line;
			if (srvmsg->svrnlen > 0) {
				anyData["servername"] = NotNull(srvmsg->svrname);
			}
			if (srvmsg->proclen > 0) {
				anyData["procname"] = NotNull(srvmsg->proc);
			}
			TraceAny(anyData, "anyData");
			(*pAny)["Messages"].Append(anyData);
			// The msgtext of the first severity unequal 0 must saved
			if ( !(*pAny).IsDefined("MainMsgErr")
				 && (long)srvmsg->msgnumber != 5701L
				 && (long)srvmsg->msgnumber != 0L ) {
				String help;
				help << "[" << anyData["msgtext"].AsString("") << "]";
				(*pAny)["MainMsgErr"] = help;
				(*pAny)["MainMsgErrNumber"] = (long)srvmsg->msgnumber;
			}
		} else {
			SysLog::Error("SybCT_servermsg_handler: could not get Message anything");
		}
	}
	return CS_SUCCEED;
}

CS_RETCODE SybCT_clientmsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg)
{
	StartTrace(SybCT.SybCT_clientmsg_handler);

	// If there is no error, we can spare to write the error-message
	if ( (long)CS_SEVERITY(errmsg->msgnumber) != 0L ) {
		Anything *pAny = NULL;
		if (SybCT::GetMessageAny(context, connection, &pAny)) {
			Anything anyData;
			anyData["severity"] = (long)CS_SEVERITY(errmsg->msgnumber);
			anyData["msgno"] = (long)CS_NUMBER(errmsg->msgnumber);
			anyData["origin"] = (long)CS_ORIGIN(errmsg->msgnumber);
			String text = NotNull(errmsg->msgstring);
			// if a "\n" trails the text, cut it away
			if (text[text.Length()-1L] == '\n') {
				text.Trim(text.Length() - 1);
			}
			anyData["msgtext"] = text;
			anyData["layer"] = (long)CS_LAYER(errmsg->msgnumber);
			if (errmsg->osstringlen > 0) {
				anyData["osstring"] = NotNull(errmsg->osstring);
			}
			TraceAny(anyData, "anyData");
			(*pAny)["Messages"].Append(anyData);
			// The msgtext of the first severity unequal 0 must saved
			if ( !(*pAny).IsDefined("MainMsgErr")
				 && (long)CS_NUMBER(errmsg->msgnumber) != 5701L
				 && (long)CS_NUMBER(errmsg->msgnumber) != 0L ) {
				String help;
				help << "[" << anyData["msgtext"].AsString("") << "]";
				(*pAny)["MainMsgErr"] = help;
				(*pAny)["MainMsgErrNumber"] = (long)CS_NUMBER(errmsg->msgnumber);
			}
		} else {
			SysLog::Error("SybCT_clientmsg_handler: could not get Message anything");
		}
	}
	return CS_SUCCEED;
}

CS_RETCODE SybCT_csmsg_handler(CS_CONTEXT *context, CS_CLIENTMSG *errmsg)
{
	StartTrace(SybCT.SybCT_csmsg_handler);

	// If there is no error, we can spare to write the error-message
	if ( (long)CS_SEVERITY(errmsg->msgnumber) != 0L ) {
		Anything *pAny = NULL;
		if (SybCT::GetMessageAny(context, NULL, &pAny)) {
			Anything anyData;
			anyData["severity"] = (long)CS_SEVERITY(errmsg->msgnumber);
			anyData["msgno"] = (long)CS_NUMBER(errmsg->msgnumber);
			anyData["origin"] = (long)CS_ORIGIN(errmsg->msgnumber);
			String text = NotNull(errmsg->msgstring);
			// if a "\n" trails the text, cut it away
			if (text[text.Length()-1L] == '\n') {
				text.Trim(text.Length() - 1);
			}
			anyData["msgtext"] = text;
			anyData["layer"] = (long)CS_LAYER(errmsg->msgnumber);
			if (errmsg->osstringlen > 0) {
				anyData["osstring"] = NotNull(errmsg->osstring);
			}
			TraceAny(anyData, "anyData");
			(*pAny)["Messages"].Append(anyData);
			// The msgtext of the first severity unequal 0 must saved
			if ( !(*pAny).IsDefined("MainMsgErr")
				 && (long)CS_NUMBER(errmsg->msgnumber) != 5701L
				 && (long)CS_NUMBER(errmsg->msgnumber) != 0L ) {
				String help;
				help << "[" << anyData["msgtext"].AsString("") << "]";
				(*pAny)["MainMsgErr"] = help;
				(*pAny)["MainMsgErrNumber"] = (long)CS_NUMBER(errmsg->msgnumber);
			}
		} else {
			SysLog::Error("SybCT_csmsg_handler: could not get Message anything");
		}
	}
	return CS_SUCCEED;
}
