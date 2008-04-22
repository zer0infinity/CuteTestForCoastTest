/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SybCTnewDA.h"

//--- project modules used -----------------------------------------------------
#include "oserror.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SysLog.h"
#include "StringStream.h"
#include "Timers.h"
#include "System.h"
#include "TimeStamp.h"

#if defined(ONLY_STD_IOSTREAM)
#include <algorithm>
#endif

//--- c-library modules used ---------------------------------------------------
#include <climits>
#include <cstring>

//---- SybCTnewDA ----------------------------------------------------------------
SimpleMutex SybCTnewDA::fgSybaseLocker("SybaseLocker", Storage::Global());

CS_RETCODE SybCTnewDA_csmsg_handler(CS_CONTEXT *context, CS_CLIENTMSG *errmsg);
CS_RETCODE SybCTnewDA_clientmsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg);
CS_RETCODE SybCTnewDA_servermsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg);

#define fooVALUE2STRING(typ) { (int)typ, #typ },

struct tValue2Strings {
	int   nValue;
	const char *pName;
};

static tValue2Strings fCsRetCode2String[] = {
	fooVALUE2STRING(CS_SUCCEED)
	fooVALUE2STRING(CS_FAIL)
	fooVALUE2STRING(CS_MEM_ERROR)
	fooVALUE2STRING(CS_PENDING)
	fooVALUE2STRING(CS_QUIET)
	fooVALUE2STRING(CS_BUSY)
	fooVALUE2STRING(CS_INTERRUPT)
	fooVALUE2STRING(CS_BLK_HAS_TEXT)
	fooVALUE2STRING(CS_CONTINUE)
	fooVALUE2STRING(CS_FATAL)
	fooVALUE2STRING(CS_RET_HAFAILOVER)
	fooVALUE2STRING(CS_CONV_ERR)
	fooVALUE2STRING(CS_EXTERNAL_ERR)
	fooVALUE2STRING(CS_INTERNAL_ERR)
	fooVALUE2STRING(CS_COMN_ERR)
	fooVALUE2STRING(CS_CANCELED)
	fooVALUE2STRING(CS_ROW_FAIL)
	fooVALUE2STRING(CS_END_DATA)
	fooVALUE2STRING(CS_END_RESULTS)
	fooVALUE2STRING(CS_END_ITEM)
	fooVALUE2STRING(CS_NOMSG)
	fooVALUE2STRING(CS_TIMED_OUT)
	fooVALUE2STRING(CS_PASSTHRU_EOM)
	fooVALUE2STRING(CS_PASSTHRU_MORE)
	fooVALUE2STRING(CS_TRYING)
	fooVALUE2STRING(CS_EBADPARAM)
	fooVALUE2STRING(CS_EBADLEN)
	fooVALUE2STRING(CS_ENOCNVRT)
	fooVALUE2STRING(CS_EOVERFLOW)
	fooVALUE2STRING(CS_EUNDERFLOW)
	fooVALUE2STRING(CS_EPRECISION)
	fooVALUE2STRING(CS_ESCALE)
	fooVALUE2STRING(CS_ESYNTAX)
	fooVALUE2STRING(CS_EFORMAT)
	fooVALUE2STRING(CS_EDOMAIN)
	fooVALUE2STRING(CS_EDIVZERO)
	fooVALUE2STRING(CS_ERESOURCE)
	fooVALUE2STRING(CS_ENULLNOIND)
	fooVALUE2STRING(CS_ETRUNCNOIND)
	fooVALUE2STRING(CS_ENOBIND)
	fooVALUE2STRING(CS_TRUNCATED)
	fooVALUE2STRING(CS_ESTYLE)
	fooVALUE2STRING(CS_EBADXLT)
	fooVALUE2STRING(CS_ENOXLT)
	fooVALUE2STRING(CS_USEREP)
};

// calculate the size of the array
static int nValue2Strings = sizeof(fCsRetCode2String) / sizeof(tValue2Strings);

SybCTnewDA::ColumnData::ColumnData(Allocator *a)
	: indicator(NULL)
	, value(NULL)
	, valuelen(NULL)
	, fAllocator((a) ? a : Storage::Current())
{
}

SybCTnewDA::ColumnData::~ColumnData()
{
	fAllocator->Free(value);
	fAllocator->Free(valuelen);
	fAllocator->Free(indicator);
}

CS_RETCODE SybCTnewDA::ColumnData::AllocateValue(size_t lSize)
{
	StartTrace(SybCTnewDA.AllocateValue);
	value = (CS_CHAR *)fAllocator->Malloc(lSize);
	if (value == NULL) {
		return CS_MEM_ERROR;
	}
	return CS_SUCCEED;
}

CS_RETCODE SybCTnewDA::ColumnData::AllocateValuelen(CS_INT num_rows)
{
	StartTrace(SybCTnewDA.AllocateValuelen);
	valuelen = (CS_INT *)fAllocator->Malloc(sizeof(CS_INT) * num_rows);
	if (valuelen == NULL) {
		return CS_MEM_ERROR;
	}
	return CS_SUCCEED;
}

CS_RETCODE SybCTnewDA::ColumnData::AllocateIndicator(CS_INT num_rows)
{
	StartTrace(SybCTnewDA.AllocateIndicator);
	indicator = (CS_SMALLINT *)fAllocator->Malloc(sizeof(CS_SMALLINT) * num_rows);
	if (indicator == NULL) {
		return CS_MEM_ERROR;
	}
	return CS_SUCCEED;
}

void *SybCTnewDA::ColumnData::operator new[](size_t size, Allocator *a) throw()
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
void SybCTnewDA::ColumnData::operator delete[](void *ptr, Allocator *a)
{
	if (ptr) {
		void *realPtr = (char *)ptr - sizeof(Allocator *);
		a->Free(realPtr);
	}
	return;
}
#endif

void SybCTnewDA::ColumnData::operator delete[](void *ptr)
{
	if (ptr) {
		void *realPtr = (char *)ptr - sizeof(Allocator *);
		Allocator *a = ((Allocator **)realPtr)[0L];	// retrieve Allocator
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		SybCTnewDA::ColumnData::operator delete[](ptr, a);
#else
		a->Free(realPtr);
#endif
	}
	return;
}

SybCTnewDA::SybCTnewDA(CS_CONTEXT *context)
	: fContext(context)
	, fConnection(NULL)
{
	StartTrace(SybCTnewDA.SybCTnewDA);
}

SybCTnewDA::~SybCTnewDA()
{
	StartTrace(SybCTnewDA.~SybCTnewDA);
	if ( fConnection ) {
		Close(true);
	}
}

String SybCTnewDA::GetStringFromRetCode(CS_RETCODE retcode)
{
	for ( int iIdx = 0; iIdx < nValue2Strings; ++iIdx) {
		if ( fCsRetCode2String[iIdx].nValue == retcode ) {
			return String(fCsRetCode2String[iIdx].pName);
		}
	}
	return String().Append((long)retcode);
}

CS_RETCODE SybCTnewDA::Init(CS_CONTEXT **context, Anything *pMessages, const String &strInterfacesPathName, CS_INT iNumberOfConns)
{
	StartTrace(SybCTnewDA.Init);
	CS_RETCODE retcode = CS_FAIL;

	Trace("Get a context handle to use.");
	retcode = cs_ctx_alloc(CTLIB_VERSION, context);
	if (retcode != CS_SUCCEED) {
		SYSERROR("cs_ctx_alloc() failed (" << GetStringFromRetCode(retcode) << ")");
		return retcode;
	}

	Trace("Initialize Open Client.");
	retcode = ct_init(*context, CTLIB_VERSION);
	if (retcode != CS_SUCCEED) {
		SYSERROR("ct_init() failed (" << GetStringFromRetCode(retcode) << ")");
		cs_ctx_drop(*context);
		*context = NULL;
		return retcode;
	}

#ifdef EX_API_DEBUG
	// ct_debug stuff. Enable this function right before any call to
	// OC-Lib that is returning failure.
	retcode = ct_debug(*context, NULL, CS_SET_FLAG, CS_DBG_API_STATES, NULL, CS_UNUSED);
	if (retcode != CS_SUCCEED) {
		SYSERROR("ct_debug() failed");
	}
#endif

	Trace("Install anything to catch context messages");
	if (retcode == CS_SUCCEED) {
		retcode = cs_config(*context, CS_SET, CS_USERDATA, &pMessages, CS_SIZEOF(Anything *), (CS_INT *)NULL);
		if (retcode != CS_SUCCEED) {
			SYSERROR("cs_config(userdata) failed (" << GetStringFromRetCode(retcode) << ")");
		}
	}

	Trace("Install cs-library message handler.");
	if (retcode == CS_SUCCEED) {
		retcode = cs_config(*context, CS_SET, CS_MESSAGE_CB, (CS_VOID *)SybCTnewDA_csmsg_handler, CS_UNUSED, NULL);
		if (retcode != CS_SUCCEED) {
			SYSERROR("cs_config(csmsg) failed (" << GetStringFromRetCode(retcode) << ")");
		}
	}

	Trace("Install client message handler.");
	if (retcode == CS_SUCCEED) {
		retcode = ct_callback(*context, NULL, CS_SET, CS_CLIENTMSG_CB, (CS_VOID *)SybCTnewDA_clientmsg_handler);
		if (retcode != CS_SUCCEED) {
			SYSERROR("ct_callback(clientmsg) failed (" << GetStringFromRetCode(retcode) << ")");
		}
	}

	Trace("Install server message handler.");
	if (retcode == CS_SUCCEED) {
		retcode = ct_callback(*context, NULL, CS_SET, CS_SERVERMSG_CB, (CS_VOID *)SybCTnewDA_servermsg_handler);
		if (retcode != CS_SUCCEED) {
			SYSERROR("ct_callback(servermsg) failed (" << GetStringFromRetCode(retcode) << ")");
		}
	}

	// This is a synchronous connection so set the input/output type
	// to synchronous (This is the default setting, but show an example anyway).
	Trace("set context to synchronous IO");
	if (retcode == CS_SUCCEED) {
		CS_INT netio_type = CS_SYNC_IO;
		retcode = ct_config(*context, CS_SET, CS_NETIO, &netio_type, CS_UNUSED, NULL);
		if (retcode != CS_SUCCEED) {
			SYSERROR("ct_config(netio) failed (" << GetStringFromRetCode(retcode) << ")");
		}
	}
	if ( retcode == CS_SUCCEED && iNumberOfConns > 0 ) {
		Trace("Try set Max Connections to [" << (long)iNumberOfConns << "]");
		retcode = ct_config(*context, CS_SET, CS_MAX_CONNECT, &iNumberOfConns, CS_UNUSED, NULL);
		if (retcode != CS_SUCCEED) {
			SYSWARNING("ct_config(CS_MAX_CONNECT) to " << (long)iNumberOfConns << " failed (" << GetStringFromRetCode(retcode) << ")");
			retcode = CS_SUCCEED;
		}
	}
	if ( retcode == CS_SUCCEED ) {
		CS_INT maxCons;
		retcode = ct_config(*context, CS_GET, CS_MAX_CONNECT, &maxCons, CS_UNUSED, NULL);
		if (retcode == CS_SUCCEED) {
			String strMsg("MAX_CONNECTIONS of SybCTnewDA: ");
			strMsg << maxCons;
			Trace(strMsg);
			SysLog::Info(strMsg);
		}
	}
	if (retcode == CS_SUCCEED && strInterfacesPathName.Length()) {
		retcode = ct_config(*context, CS_SET, CS_IFILE, (CS_VOID *)(const char *)strInterfacesPathName, strInterfacesPathName.Length(), NULL);
		if (retcode != CS_SUCCEED) {
			SYSERROR("ct_config(cs_ifile) failed (" << GetStringFromRetCode(retcode) << ")");
		}
	}

	if (retcode != CS_SUCCEED) {
		ct_exit(*context, CS_FORCE_EXIT);
		cs_ctx_drop(*context);
		*context = NULL;
	}

	return retcode;
}

CS_RETCODE SybCTnewDA::Finis(CS_CONTEXT *context)
{
	StartTrace(SybCTnewDA.Finis);
	CS_RETCODE	retcode;

	retcode = ct_exit(context, CS_UNUSED);
	if (retcode != CS_SUCCEED) {
		SYSERROR("ct_exit() failed (" << GetStringFromRetCode(retcode) << ")");
		return retcode;
	}
	retcode = cs_ctx_drop(context);
	if (retcode != CS_SUCCEED) {
		SYSERROR("cs_ctx_drop() failed (" << GetStringFromRetCode(retcode) << ")");
		return retcode;
	}
	return retcode;
}

bool SybCTnewDA::Open(DaParams &params, String user, String password, String server, String appl)
{
	StartTrace(SybCTnewDA.Open);
	DAAccessTimer(SybCTnewDAImpl.Open, *(params.fpDAName), *(params.fpContext));
	CS_RETCODE	retcode;

	// Allocate a connection structure.
	if ( (retcode = ct_con_alloc(fContext, &fConnection)) == CS_SUCCEED ) {
		// set param structure containing Context and Mappers used to return results and messages
		if ( ( retcode = SetConProps(CS_USERDATA, (CS_VOID *)&params, CS_SIZEOF(DaParams)) ) == CS_SUCCEED ) {
			// If a username is defined, set the CS_USERNAME property.
			if ( user.Length() ) {
				if ((retcode = SetConProps(CS_USERNAME, (CS_VOID *)(char *)(const char *)user, CS_NULLTERM)) != CS_SUCCEED) {
					Error(params, "Open: ct_con_props(username) failed");
				}
			} else {
				Warning(params, "Open: Username missing");
			}

			if ( retcode == CS_SUCCEED ) {
				// If a password is defined, set the CS_PASSWORD property.
				if ( password.Length() ) {
					if ((retcode = SetConProps(CS_PASSWORD, (CS_VOID *)(char *)(const char *)password, CS_NULLTERM)) != CS_SUCCEED) {
						Error(params, "Open: ct_con_props(password) failed");
					}
				} else {
					Warning(params, "Open: Password missing");
				}

				if ( retcode == CS_SUCCEED ) {
					// Set the CS_APPNAME property.
					if ( appl.Length() ) {
						if ((retcode = SetConProps(CS_APPNAME, (CS_VOID *)(char *)(const char *)appl, CS_NULLTERM)) != CS_SUCCEED) {
							Error(params, "Open: ct_con_props(appname) failed");
						}
					} else {
						Warning(params, "Open: Applicationname missing");
					}

					// Set Host-Name
					String hostName;
					if ( (retcode == CS_SUCCEED) && System::HostName( hostName ) ) {
						Trace("setting hostname to [" << hostName << "]");
						if ( (retcode = ct_con_props(fConnection, CS_SET, CS_HOSTNAME, (char *)(const char *)hostName, CS_NULLTERM, NULL)) != CS_SUCCEED ) {
							Warning(params, "Open: setting hostname failed");
						}
					}

					// Set LoginTimeout if specified
					long lLoginTimeout = CS_NO_LIMIT;
					if ( (retcode == CS_SUCCEED) && (params.fpIn)->Get( "LoginTimeout", lLoginTimeout, *(params.fpContext)) ) {
						CS_INT lOldTimeout = CS_NO_LIMIT;
						if ( ( retcode = ct_config(fContext, CS_GET, CS_LOGIN_TIMEOUT, &lOldTimeout, CS_UNUSED, NULL) ) != CS_SUCCEED ) {
							Warning(params, "Open: retrieval of CS_LOGIN_TIMEOUT failed");
						}
						if ( lOldTimeout == CS_NO_LIMIT ) {
							Trace("old CS_LOGIN_TIMEOUT value: <unlimited>");
						} else {
							Trace("old CS_LOGIN_TIMEOUT value: " << (long)lOldTimeout << "s");
						}
						if ( ( lLoginTimeout > 0L || lLoginTimeout == CS_NO_LIMIT ) && ( lOldTimeout != (CS_INT)lLoginTimeout ) ) {
							Trace("setting CS_LOGIN_TIMEOUT to " << lLoginTimeout << "s");
							CS_INT iLoginTimeout = (CS_INT)lLoginTimeout;
							if ( ( retcode = ct_config(fContext, CS_SET, CS_LOGIN_TIMEOUT, (CS_VOID *)&iLoginTimeout, CS_UNUSED, NULL) ) != CS_SUCCEED ) {
								Warning(params, "Open: setting CS_LOGIN_TIMEOUT failed");
							}
						}
					}

					// Set ResultTimeout if specified
					long lTimeout = CS_NO_LIMIT;
					if ( (retcode == CS_SUCCEED) && (params.fpIn)->Get( "ResultTimeout", lTimeout, *(params.fpContext)) ) {
						CS_INT lOldTimeout = CS_NO_LIMIT;
						if ( ( retcode = ct_config(fContext, CS_GET, CS_TIMEOUT, &lOldTimeout, CS_UNUSED, NULL) ) != CS_SUCCEED ) {
							Warning(params, "Open: retrieval of CS_TIMEOUT failed");
						}
						if ( lOldTimeout == CS_NO_LIMIT ) {
							Trace("old CS_TIMEOUT value: <unlimited>");
						} else {
							Trace("old CS_TIMEOUT value: " << (long)lOldTimeout << "s");
						}
						if ( ( lTimeout > 0L || lTimeout == CS_NO_LIMIT ) && ( lOldTimeout != (CS_INT)lTimeout ) ) {
							Trace("setting CS_TIMEOUT to " << lTimeout << "s");
							CS_INT iTimeout = (CS_INT)lTimeout;
							if ( ( retcode = ct_config(fContext, CS_SET, CS_TIMEOUT, (CS_VOID *)&iTimeout, CS_UNUSED, NULL) ) != CS_SUCCEED ) {
								Warning(params, "Open: setting CS_TIMEOUT failed");
							}
						}
					}

					// Open a Server fConnection.
					if ( retcode == CS_SUCCEED ) {
						if ( server.Length() ) {
							{
								LockUnlockEntry me(fgSybaseLocker);
								if ( ( retcode = ct_connect(fConnection, (char *)(const char *)server, CS_NULLTERM) ) == CS_SUCCEED ) {
									return true;
								}
							}
							Error(params, String("Open: ct_connect failed (") << GetStringFromRetCode(retcode) << ")");
						} else {
							Error(params, "Open: Servername missing");
						}
					}
				}
			}
		} else {
			Error(params, "Open: ct_con_props(CS_SET CS_USERDATA) failed");
		}
		ct_con_drop(fConnection);
		fConnection = NULL;
	} else {
		Error(params, "Open: ct_con_alloc failed");
	}
	return false;
}

bool SybCTnewDA::SqlExec(DaParams &params, String query, String resultformat, const long lMaxResultSize, const long lMaxRows)
{
	StartTrace(SybCTnewDA.SqlExec);
	DAAccessTimer(SybCTnewDAImpl.SqlExec, *(params.fpDAName), *(params.fpContext));
	Trace("dbcmd: [" << query << "]");

	CS_RETCODE	retcode;
	CS_COMMAND	*cmd = NULL;

	// set param structure containing Context and Mappers used to return results and messages
	if ( (retcode = SetConProps(CS_USERDATA, (CS_VOID *)&params, CS_SIZEOF(DaParams))) != CS_SUCCEED) {
		Error(params, String("SqlExec: SetConProps(params) failed (") << GetStringFromRetCode(retcode) << ")");
		return false;
	}

	// Allocate a command handle to send the query
	if ((retcode = ct_cmd_alloc(fConnection, &cmd)) != CS_SUCCEED) {
		Error(params, String("SqlExec: ct_cmd_alloc() failed (") << GetStringFromRetCode(retcode) << ")");
		return false;
	}

	params.fpOut->Put("Query", query, *(params.fpContext));

	// Initiate the command with the specified query
	retcode = ct_command(cmd, CS_LANG_CMD, (char *)(const char *)query, CS_NULLTERM, CS_UNUSED);
	if (retcode != CS_SUCCEED) {
		Error(params, String("SqlExec: ct_command(CS_LANG_CMD:") << query << ") failed (" << GetStringFromRetCode(retcode) << ")");
		(void)ct_cmd_drop(cmd);
		return false;
	}

	// Send the command to the server
	if ((retcode = ct_send(cmd)) != CS_SUCCEED) {
		Error(params, String("DoSqlExec: ct_send() failed (") << GetStringFromRetCode(retcode) << ")");
		(void)ct_cmd_drop(cmd);
		return false;
	}

	// Process the results. Loop while ct_results() returns CS_SUCCEED.
	CS_INT res_type;
	CS_RETCODE	query_code = CS_SUCCEED;
	long lRowsReceived( 0L );

	retcode = ct_results(cmd, &res_type);
	while ( retcode == CS_SUCCEED ) {
		switch ((int)res_type) {
			case CS_CMD_SUCCEED:
				// This means no rows were returned.
			case CS_CMD_DONE:
				// This means we're done with one result set.
				break;
			case CS_CMD_FAIL:
				// This means that the server encountered an error while
				// processing our command.
				Error(params, "SqlExec: ct_results() returned CMD_FAIL");
				query_code = CS_FAIL;
				break;
			case CS_MSG_RESULT: {
				CS_SMALLINT	msg_id;
				// the result was a message
				retcode = ct_res_info(cmd, CS_MSGTYPE, (CS_VOID *)&msg_id, CS_UNUSED, NULL);
				if (retcode != CS_SUCCEED) {
					Error(params, String("SqlExec: ct_res_info(msgtype) failed (") << GetStringFromRetCode(retcode) << ")");
					query_code = CS_FAIL;
				} else {
					SysLog::Info(String("SybCTnewDA::SqlExec: we got a MessageResult with id:") << (long)msg_id);
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
				retcode = DoFetchData(params, cmd, res_type, resultformat, lMaxResultSize, lMaxRows, lRowsReceived);
				// abortion due to reached memory limit or given row count will be signalled using CS_MEM_ERROR
				// catch this but do not result in failure because it was a programmers wish
				if ( ( retcode == CS_MEM_ERROR ) && ( ( lMaxResultSize != 0L ) || ( lMaxRows != -1L ) ) ) {
					String strMsg("SqlExec[");
					strMsg << *(params.fpDAName) << "]:";
					if ( lMaxResultSize != 0L && lMaxRows != -1L ) {
						Warning(params, strMsg.Append(" rows limited due to SybDBMaxResultSize(").Append(lMaxResultSize).Append("kB) or SybDBMaxRows(").Append(lMaxRows).Append(")!"));
					} else if ( lMaxResultSize != 0L ) {
						Warning(params, strMsg.Append(" rows limited due to SybDBMaxResultSize(").Append(lMaxResultSize).Append("kB)!"));
					} else if ( lMaxRows != -1L ) {
						Warning(params, strMsg.Append(" rows limited due to SybDBMaxRows(").Append(lMaxRows).Append(")!"));
					}
					// need to pass CS_MEM_ERROR as query_code to force abortion of still running query
					query_code = retcode;
				} else if (retcode != CS_SUCCEED) {
					Error(params, String("SqlExec: ex_fetch_data() failed (") << GetStringFromRetCode(retcode) << ")");
					query_code = CS_FAIL;
				}
				break;
			default:
				// We got an unexpected result type.
				Error(params, "SqlExec: ct_results() returned unexpected result type");
				query_code = CS_FAIL;
		}
		if (query_code == CS_FAIL || query_code == CS_MEM_ERROR) {
			// Terminate results processing and break out of the results loop
			retcode = ct_cancel(NULL, cmd, CS_CANCEL_ALL);
			if (retcode != CS_SUCCEED) {
				Error(params, String("SqlExec: ct_cancel() failed (") << GetStringFromRetCode(retcode) << ")");
			}
			// must adjust query_code again to force successful termination of method
			if ( ( query_code == CS_MEM_ERROR ) && ( ( lMaxResultSize != 0L ) || ( lMaxRows != -1L ) ) ) {
				query_code = CS_SUCCEED;
			}
			break;
		}
		retcode = ct_results(cmd, &res_type);
	}

	// We're done processing results. Let's check the return value of ct_results() to see if everything
	// went ok.
	Trace(GetStringFromRetCode(retcode));
	switch ((int)retcode) {
		case CS_END_RESULTS:
			// Everything went fine.
			retcode = ct_cmd_drop(cmd);
			if (retcode != CS_SUCCEED) {
				query_code = CS_FAIL;
			}
			break;
		case CS_SUCCEED:
			// probably set from ct_cancel: used to terminate query early because of MemoryLimit
			retcode = ct_cmd_drop(cmd);
			if (retcode != CS_SUCCEED) {
				Trace("CS_FAIL");
				query_code = CS_FAIL;
			}
			break;
		case CS_CANCELED:
			Trace("command has been CS_CANCELED");
			ct_cmd_drop(cmd);
			query_code = CS_FAIL;
			break;
		case CS_FAIL:
			// Something terrible happened.
		default:
			(void)ct_cmd_drop(cmd);
			query_code = CS_FAIL;
			// We got an unexpected return value.
			Error(params, "SqlExec: ct_results() failed");
	}
	return (query_code == CS_SUCCEED);
}

CS_RETCODE SybCTnewDA::DoFetchData(DaParams &params, CS_COMMAND *cmd, const CS_INT res_type, const String &resultformat, const long lMaxResultSize, const long lParMaxRows, long &lRowsReceived)
{
	StartTrace(SybCTnewDA.DoFetchData);
	DAAccessTimer(SybCTnewDAImpl.DoFetchData, *(params.fpDAName), *(params.fpContext));

	CS_RETCODE		retcode;
	CS_INT			num_cols;
	CS_INT			i;
	CS_INT			rows_read;
	CS_DATAFMT		*datafmt;
	EX_COLUMN_DATA	*coldata;
	CS_CONNECTION	*connection;
	CS_INT			rowsize;
	CS_INT			num_rows;	// number of rows to get in a single fetch
	long lMaxRows( 0L );

	connection = (CS_CONNECTION *)NULL;
	rowsize = 0;

	// Get parent connection
	retcode = ct_cmd_props(cmd, CS_GET, CS_PARENT_HANDLE, &connection, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED) {
		Error(params, String("DoFetchData: ct_cmd_props(CS_PARENT_HANDLE) failed (") << GetStringFromRetCode(retcode) << ")");
		return retcode;
	}

	// Find out how many columns there are in this result set.
	retcode = ct_res_info(cmd, CS_NUMDATA, &num_cols, CS_UNUSED, NULL);
	if (retcode != CS_SUCCEED) {
		Error(params, String("DoFetchData: ct_res_info() failed (") << GetStringFromRetCode(retcode) << ")");
		return retcode;
	}

	Trace("we have to process " << (long)num_cols << " columns");
	// Make sure we have at least one column
	if (num_cols <= 0) {
		Error(params, "DoFetchData: ct_res_info() returned zero columns");
		return CS_FAIL;
	}

	// Our program variable, called 'coldata', is an array of
	// EX_COLUMN_DATA structures. Each array element represents
	// one column.  Each array element will re-used for each row set.
	// In coldata, we allocate an array of values and thus use
	// array binding.
	//
	// First, allocate memory for the data element to process.
	coldata = new (Storage::Current()) EX_COLUMN_DATA[num_cols];
	if (coldata == NULL) {
		Error(params, "DoFetchData: new EX_COLUMN_DATA failed");
		return CS_MEM_ERROR;
	}

	datafmt = (CS_DATAFMT *)(Storage::Current())->Malloc(num_cols * sizeof (CS_DATAFMT));
	if (datafmt == NULL) {
		Error(params, "DoFetchData: Malloc of datafmt failed");
		delete[] coldata;
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
	for (i = 0; i < num_cols; ++i) {
		// Get the column description.  ct_describe() fills the
		// datafmt parameter with a description of the column.
		retcode = ct_describe(cmd, (i + 1), &datafmt[i]);
		if (retcode != CS_SUCCEED) {
			Error(params, String("DoFetchData: ct_describe() failed (") << GetStringFromRetCode(retcode) << ")");
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
		(Storage::Current())->Free(datafmt);
		return retcode;
	}

	// adding needed space for valuelen and indicator per column
	rowsize += (MEM_ALIGN_SIZE(sizeof(CS_INT)) + MEM_ALIGN_SIZE(sizeof(CS_SMALLINT))) * num_cols;
	Trace("total rowsize needed after adding additional space: " << (long)rowsize << " bytes");
	// Find max. number of rows that we want to obtain in a single
	// fetch (Why? - just to limit memory being used).
	if (MAX_MEM_BLOCK_SIZE < rowsize) {
		// no row fetchable
		Error(params, String("DoFetchData: MAX_MEM_BLOCK_SIZE (") << (long)MAX_MEM_BLOCK_SIZE << " bytes) too small for query, needs " << rowsize << " bytes!");
		retcode = CS_CMD_FAIL;
		delete[] coldata;
		(Storage::Current())->Free(datafmt);
		return retcode;
	} else {
		num_rows = MAX_MEM_BLOCK_SIZE / rowsize;
	}
	// calculate max amount of rows to read without exceeding a given memory limit
	if (lMaxResultSize == 0) {
		lMaxRows = LONG_MAX;
	} else {
		lMaxRows = (lMaxResultSize * 1024) / rowsize;
	}
	if ( lParMaxRows != -1L ) {
		lMaxRows = std::min(lMaxRows, lParMaxRows);
	}
	num_rows = std::min(lMaxRows, num_rows);
	Trace("max number of rows to fetch at once: " << (long)num_rows);
	Trace("max number of rows to totally fetch: " << lMaxRows);

	for (i = 0; i < num_cols; ++i) {
		Trace("processing column [" << datafmt[i].name << "]");
		// Allocate memory for the column string
		Trace("value mem: " << (long)num_rows << " rows with maxlength of " << (long)datafmt[i].maxlength << " bytes");
		if (coldata[i].AllocateValue(datafmt[i].maxlength * num_rows) == CS_MEM_ERROR) {
			Error(params, "DoFetchData: AllocateValue() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		// Allocate memory for the valuelen
		Trace("valuelen mem: " << (long)num_rows << " rows with length of " << (long)sizeof(CS_INT) << " bytes");
		if (coldata[i].AllocateValuelen(num_rows) == CS_MEM_ERROR) {
			Error(params, "DoFetchData: AllocateValuelen() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		// Allocate memory for the indicator
		Trace("indicator mem: " << (long)num_rows << " rows with length of " << (long)sizeof(CS_SMALLINT) << " bytes");
		if (coldata[i].AllocateIndicator(num_rows) == CS_MEM_ERROR) {
			Error(params, "DoFetchData: AllocateIndicator() failed");
			retcode = CS_MEM_ERROR;
			break;
		}

		// Now do an array bind.
		Trace("binding column");
		datafmt[i].count = num_rows;
		retcode = ct_bind(cmd, (i + 1), &datafmt[i], coldata[i].value, coldata[i].valuelen, coldata[i].indicator);
		if (retcode != CS_SUCCEED) {
			Error(params, String("DoFetchData: ct_bind() failed (") << GetStringFromRetCode(retcode) << ")");
			break;
		}
	}
	if (retcode != CS_SUCCEED) {
		delete[] coldata;
		(Storage::Current())->Free(datafmt);
		return retcode;
	}

	// Fetch the rows.  Loop while ct_fetch() returns CS_SUCCEED or  CS_ROW_FAIL
	retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, &rows_read);

	Trace("number of rows fetched: " << rows_read);
	if ( retcode == CS_SUCCEED || retcode == CS_ROW_FAIL ) {
		bool bTitlesOnce = resultformat.IsEqual("TitlesOnce");
		if ( ( res_type != CS_STATUS_RESULT ) && bTitlesOnce ) {
			// put column name information
			Anything temp;
			for (long col = 0; col < num_cols; ++col) {
				Trace("colname@" << col << " [" << datafmt[col].name << "]" );
				temp[datafmt[col].name] = (long)col;
			}
			params.fpOut->Put("QueryTitles", temp, *(params.fpContext));
		}

		bool bGoOn = true;
		// process each set of rows
		while ( bGoOn && ( retcode == CS_SUCCEED || retcode == CS_ROW_FAIL ) ) {
			// Check if we hit a recoverable error.
			if (retcode == CS_ROW_FAIL) {
				Error(params, String("DoFetchData: Error on row ") << (long)lRowsReceived << " (" << GetStringFromRetCode(retcode) << ")");
			}

			if ( res_type == CS_STATUS_RESULT ) {
				String strRetcode;
				strRetcode << EX_GET_COLUMN_VALUE(coldata, 0, 0, datafmt);
				Trace("CS_STATUS_RESULT: SP_Retcode [" << strRetcode << "]");
				params.fpOut->Put("SP_Retcode", strRetcode, *(params.fpContext));
			} else {
				// process all rows retrieved so far
				bGoOn = DoFillResults(params, lRowsReceived, rows_read, num_cols, datafmt, coldata, bTitlesOnce );

				// Increment our row count by the number of rows fetched and processed
				lRowsReceived += rows_read;
			}

			// do we need to fetch more rows yet?
			if ( ( (lRowsReceived + num_rows) > lMaxRows ) || !bGoOn ) {
				retcode = CS_MEM_ERROR;
				break;
			} else {
				retcode = ct_fetch(cmd, CS_UNUSED, CS_UNUSED, CS_UNUSED, &rows_read);
				Trace("number of rows fetched again: " << rows_read);
			}
			Trace(String() << (long)lRowsReceived << " rows read so far");
		}
		if ( res_type != CS_STATUS_RESULT ) {
			bool bShowRowCount( true );
			params.fpIn->Get("ShowQueryCount", bShowRowCount, *(params.fpContext));
			if ( bShowRowCount ) {
				params.fpOut->Put("QueryCount", lRowsReceived, *(params.fpContext));
			}
		}
	}

	// Free allocated space.
	delete[] coldata;
	(Storage::Current())->Free(datafmt);

	// We're done processing rows.  Let's check the final return value of ct_fetch().
	switch ((int)retcode) {
		case CS_END_DATA:
			// Everything went fine.
			Trace("All done processing rows - total " << (long)lRowsReceived);
			retcode = CS_SUCCEED;
			break;

		case CS_MEM_ERROR:
			// early stop due to memory limitations
			Trace("processed only " << (long)lRowsReceived << " rows");
			break;

		case CS_FAIL:
			// Something terrible happened.
			if ((lRowsReceived + num_rows) > lMaxRows) {
				Warning(params, String("DoFetchData: rows limited due to memory limit (") << lMaxResultSize << " kB)");
				break;
			}

		default:
			// We got an unexpected return value.
			Error(params, String("DoFetchData: Error after ct_fetch() (") << GetStringFromRetCode(retcode) << ")");
			break;
	}
	return retcode;
}

CS_INT SybCTnewDA::DisplayDlen(CS_DATAFMT *column)
{
	CS_INT		len;

	switch ((int) column->datatype) {
		case CS_CHAR_TYPE:
		case CS_VARCHAR_TYPE:
		case CS_TEXT_TYPE:
		case CS_IMAGE_TYPE:
			len = std::min<CS_INT>(column->maxlength, MAX_CHAR_BUF);
			break;

		case CS_BINARY_TYPE:
		case CS_VARBINARY_TYPE:
			len = std::min<CS_INT>((2 * column->maxlength) + 2, MAX_CHAR_BUF);
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

	return std::max(strlen(column->name) + 1, (unsigned)len);
}

bool SybCTnewDA::DoFillResults(DaParams &params, CS_INT totalrows, CS_INT numrows, CS_INT numcols, CS_DATAFMT *colfmt, EX_COLUMN_DATA *coldata, bool bTitlesOnce )
{
	StartTrace(SybCTnewDA.DoFillResults);
	ResultMapper *pResultMapper = params.fpOut;
	Context &aContext = *(params.fpContext);
	bool bRet = true;
	DAAccessTimer(SybCTnewDAImpl.DoFillResults, *(params.fpDAName), *(params.fpContext));

	Trace("resultformat [" << (bTitlesOnce ? "TitlesOnce" : "TitlesAlways") << "]");
	for (CS_INT row = 0; bRet && row < numrows; ++row) {
		// We have a row.  Loop through the columns displaying the column values.
		Anything temp;
		for (CS_INT col = 0; col < numcols; ++col) {
			if ( bTitlesOnce ) {
				temp.Append( EX_GET_COLUMN_VALUE(coldata, row, col, colfmt) );
			} else {
				temp[colfmt[col].name] = EX_GET_COLUMN_VALUE(coldata, row, col, colfmt);
			}
		}
		TraceAny(temp, "putting into QueryResult for row " << (long)(totalrows + row));
		bRet = pResultMapper->Put("QueryResult", temp, aContext);
	}

	return bRet;
}

bool SybCTnewDA::Close(bool bForce)
{
	StartTrace(SybCTnewDA.Close);

	CS_RETCODE retcode = CS_SUCCEED;
	if ( fConnection ) {
		CS_INT	close_option;

		close_option = bForce ? CS_FORCE_CLOSE : CS_UNUSED;
		retcode = ct_close(fConnection, close_option);
		if ( retcode != CS_SUCCEED ) {
			SYSERROR("SybCTnewDA::Close: ct_close() failed (" << GetStringFromRetCode(retcode) << ")");
			if ( !bForce ) {
				SYSERROR("SybCTnewDA::Close: force closing now");
				if ( ( retcode = ct_close(fConnection, CS_FORCE_CLOSE) ) != CS_SUCCEED ) {
					SYSERROR("SybCTnewDA::Close: force closing failed!? (" << GetStringFromRetCode(retcode) << ")");
				}
			}
		}

		retcode = ct_con_drop(fConnection);
		if (retcode != CS_SUCCEED) {
			SYSERROR("SybCTnewDA::Close: ct_con_drop() failed (" << GetStringFromRetCode(retcode) << ")");
		}
		fConnection = NULL;
	}
	return (retcode == CS_SUCCEED);
}

void SybCTnewDA::Warning(DaParams &params, String str)
{
	StartTrace(SybCTnewDA.Warning);
	String strErr("SybCTnewDA::");
	strErr.Append(str);
	SysLog::Warning( TimeStamp::Now().AsStringWithZ().Append(' ').Append(strErr) );
	ResultMapper *pResultMapper = params.fpOut;
	Context &aContext( *(params.fpContext) );
	if ( pResultMapper ) {
		pResultMapper->Put("Messages", strErr, aContext);
	}
}

void SybCTnewDA::Error(DaParams &params, String str)
{
	StartTrace(SybCTnewDA.Error);
	String strErr("SybCTnewDA::");
	strErr.Append(str);
	SysLog::Error( TimeStamp::Now().AsStringWithZ().Append(' ').Append(strErr) );
	ResultMapper *pResultMapper( params.fpOut );
	Context &aContext( *(params.fpContext) );
	if ( pResultMapper ) {
		pResultMapper->Put("Messages", strErr, aContext);
	}
}

bool SybCTnewDA::GetDaParams(DaParams &params, CS_CONNECTION *connection)
{
	StartTrace(SybCTnewDA.GetDaParams);
	return SybCTnewDA::IntGetConProps(connection, CS_USERDATA, (CS_VOID **)&params, CS_SIZEOF(SybCTnewDA::DaParams)) == CS_SUCCEED;
}

CS_RETCODE SybCTnewDA::SetConProps(CS_INT property, CS_VOID *buffer, CS_INT buflen)
{
	StartTrace(SybCTnewDA.SetConProps);
//	LockUnlockEntry me(fgSybaseLocker);
	return ct_con_props(fConnection, CS_SET, property, buffer, buflen, NULL);
}

bool SybCTnewDA::GetConProps(CS_INT property, CS_VOID **propvalue, CS_INT propsize)
{
	StartTrace(SybCTnewDA.GetConProps);
	return IntGetConProps(fConnection, property, propvalue, propsize);
}

bool SybCTnewDA::IntGetConProps(CS_CONNECTION *connection, CS_INT property, CS_VOID **propvalue, CS_INT propsize)
{
	StartTrace(SybCTnewDA.IntGetConProps);
	CS_RETCODE retcode = CS_FAIL;

	if (connection != (CS_CONNECTION *)NULL) {
		retcode = ct_con_props(connection, CS_GET, property, propvalue, propsize, (CS_INT *)NULL);
		if (retcode != CS_SUCCEED) {
			SYSERROR("ct_con_props(CS_GET, " << (long)property << ") failed (" << GetStringFromRetCode(retcode) << ")");
		}
	}
	return (retcode == CS_SUCCEED);
}

bool SybCTnewDA::GetMessageAny(CS_CONTEXT *context, Anything **anyMessage)
{
	StartTrace(SybCTnewDA.GetMessageAny);
	CS_RETCODE retcode = cs_config(context, CS_GET, CS_USERDATA, anyMessage, CS_SIZEOF(Anything *), (CS_INT *)NULL);
	if (retcode != CS_SUCCEED) {
		SYSERROR("cs_config(CS_GET CS_USERDATA) failed (" << GetStringFromRetCode(retcode) << ")");
	}
	return (retcode == CS_SUCCEED);
}

bool SybCTnewDA::PutMessages(DaParams &daParams, Anything &anyMessages)
{
	StartTrace(SybCTnewDA.PutMessages);
	bool bRet = false;
	if ( daParams.fpOut != NULL && daParams.fpContext != NULL ) {
		for (long lIdx = 0; lIdx < anyMessages.GetSize(); ++lIdx) {
			String strSlotname = anyMessages.SlotName(lIdx);
			TraceAny(anyMessages[lIdx], "Putting slot [" << strSlotname << "]");
			(daParams.fpOut)->Put(strSlotname, anyMessages[lIdx], *(daParams.fpContext));
		}
		bRet = true;
	}
	return bRet;
}

CS_RETCODE SybCTnewDA_servermsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_SERVERMSG *srvmsg)
{
	StartTrace(SybCTnewDA.SybCTnewDA_servermsg_handler);

	if ( (long)srvmsg->severity != 0L ) {
		SybCTnewDA::DaParams daParams;
		bool bFuncCode = false;
		{
//			LockUnlockEntry me(SybCTnewDA::fgSybaseLocker);
			bFuncCode = SybCTnewDA::GetDaParams(daParams, connection);
		}
		if ( bFuncCode ) {
			Anything anyData, anyRef;
			anyData["Messages"] = MetaThing();
			anyRef = anyData["Messages"];
			CS_MSGNUM lMsgNumber = srvmsg->msgnumber;
			anyRef["MsgNumber"] = (long)lMsgNumber;
			anyRef["State"] = (long)srvmsg->state;
			// it only makes sense to decode the msgnumber if the severity is in decodable range
			// currently, only codes up to 7 are in range
			if ( CS_SEVERITY(lMsgNumber) <= CS_SV_FATAL ) {
				anyRef["Severity"] = CS_SEVERITY(lMsgNumber);
				anyRef["Number"] = CS_NUMBER(lMsgNumber);
				anyRef["Origin"] = CS_ORIGIN(lMsgNumber);
				anyRef["Layer"] = CS_LAYER(lMsgNumber);
			} else {
				anyRef["Severity"] = (long)srvmsg->severity;
			}
			String text = NotNull(srvmsg->text);
			// if a "\n" trails the text, cut it away
			if (text[text.Length()-1L] == '\n') {
				text.Trim(text.Length() - 1);
			}
			anyRef["msgtext"] = text;
			anyRef["line"] = (long)srvmsg->line;
			if (srvmsg->svrnlen > 0) {
				anyRef["servername"] = NotNull(srvmsg->svrname);
			}
			if (srvmsg->proclen > 0) {
				anyRef["procname"] = NotNull(srvmsg->proc);
			}
			// The msgtext of the first severity unequal 0 must saved
			if ( (long)srvmsg->msgnumber != SQLSRV_ENVDB && (long)srvmsg->msgnumber != 0L ) {
				String help;
				help << "[" << anyRef["msgtext"].AsString("") << "]";
				anyData["MainMsgErr"] = help;
				anyData["MainMsgErrNumber"] = (long)srvmsg->msgnumber;
			}
			TraceAny(anyData, "anyData");
			{
//				LockUnlockEntry me(SybCTnewDA::fgSybaseLocker);
				bFuncCode = SybCTnewDA::PutMessages(daParams, anyData);
			}
			if ( !bFuncCode ) {
				SysLog::Error("SybCTnewDA_servermsg_handler: could not put messages using Mapper");
			}
		} else {
			SysLog::Error("SybCTnewDA_servermsg_handler: could not get Message anything");
		}
	}
	return CS_SUCCEED;
}

CS_RETCODE SybCTnewDA_clientmsg_handler(CS_CONTEXT *context, CS_CONNECTION *connection, CS_CLIENTMSG *errmsg)
{
	StartTrace(SybCTnewDA.SybCTnewDA_clientmsg_handler);

	// If there is no error, we can spare to write the error-message
	if ( (long)CS_SEVERITY(errmsg->msgnumber) != 0L ) {
		SybCTnewDA::DaParams daParams;
		bool bFuncCode = false;
		{
//			LockUnlockEntry me(SybCTnewDA::fgSybaseLocker);
			bFuncCode = SybCTnewDA::GetDaParams(daParams, connection);
		}
		if ( bFuncCode ) {
			Anything anyData, anyRef;
			anyData["Messages"] = MetaThing();
			anyRef = anyData["Messages"];
			CS_MSGNUM lMsgNumber = errmsg->msgnumber;
			anyRef["MsgNumber"] = (long)lMsgNumber;
			// it only makes sense to decode the msgnumber if the severity is in decodable range
			// currently, only codes up to 7 are in range
			if ( CS_SEVERITY(lMsgNumber) <= CS_SV_FATAL ) {
				anyRef["Severity"] = CS_SEVERITY(lMsgNumber);
				anyRef["Number"] = CS_NUMBER(lMsgNumber);
				anyRef["Origin"] = CS_ORIGIN(lMsgNumber);
				anyRef["Layer"] = CS_LAYER(lMsgNumber);
			} else {
				anyRef["Severity"] = (long)errmsg->severity;
			}

			String text = NotNull(errmsg->msgstring);
			// if a "\n" trails the text, cut it away
			if (text[text.Length()-1L] == '\n') {
				text.Trim(text.Length() - 1);
			}
			anyRef["msgtext"] = text;
			if (errmsg->osstringlen > 0) {
				anyRef["osstring"] = NotNull(errmsg->osstring);
			}
			// The msgtext of the first severity unequal 0 must saved
			if ( (long)errmsg->msgnumber != SQLSRV_ENVDB && (long)CS_NUMBER(errmsg->msgnumber) != 0L ) {
				String help;
				help << "[" << anyRef["msgtext"].AsString("") << "]";
				anyData["MainMsgErr"] = help;
				anyData["MainMsgErrNumber"] = (long)CS_NUMBER(errmsg->msgnumber);
			}
			TraceAny(anyData, "anyData");
			{
//				LockUnlockEntry me(SybCTnewDA::fgSybaseLocker);
				bFuncCode = SybCTnewDA::PutMessages(daParams, anyData);
			}
			if ( !bFuncCode ) {
				SysLog::Error("SybCTnewDA_clientmsg_handler: could not put messages using Mapper");
			}

#define ERROR_SNOL(e, s, n, o, l) \
	( (CS_SEVERITY(e) == s) && (CS_NUMBER(e) == n) \
	&& (CS_ORIGIN(e) == o) && (CS_LAYER(e) == l ) )

			if ( ERROR_SNOL(errmsg->msgnumber, CS_SV_RETRY_FAIL, 63, 2, 1) ) {
				Trace("a kind of timeout error occured");
				// Read from server timed out. Timeouts happen on synchronous
				// connections only, and you must have set one or both of the
				// following context properties to see them:
				//  CS_TIMEOUT for results timeouts
				//  CS_LOGIN_TIMEOUT for login-attempt timeouts

				// If we return CS_FAIL, the connection is marked as dead and
				// unrecoverable.  If we return CS_SUCCEED, the timeout
				// continues for another quantum.

				// We kill the connection for login timeouts, and send a
				// cancel for results timeouts. We determine which case we
				// have via the CS_LOGIN_STATUS property.
				CS_INT status = 0;
				if ( ct_con_props(connection, CS_GET, CS_LOGIN_STATUS, (CS_VOID *)&status, CS_UNUSED, NULL) != CS_SUCCEED ) {
					SysLog::Error("SybCTnewDA_clientmsg_handler: could not retrieve CS_LOGIN_STATUS");
					return CS_FAIL;
				}

				if (status) {
					// Results timeout
					SysLog::Warning("SybCTnewDA_clientmsg_handler: cancelling the query due to a result timeout...");
					(CS_VOID)ct_cancel(connection, (CS_COMMAND *)NULL, CS_CANCEL_ATTN);
				} else {
					// Login timeout
					SysLog::Warning("SybCTnewDA_clientmsg_handler: aborting connection(login) attempt...");
					return CS_FAIL;
				}
			}
		} else {
			SysLog::Error("SybCTnewDA_clientmsg_handler: could not get Message anything");
		}
	}
	return CS_SUCCEED;
}

CS_RETCODE SybCTnewDA_csmsg_handler(CS_CONTEXT *context, CS_CLIENTMSG *errmsg)
{
	StartTrace(SybCTnewDA.SybCTnewDA_csmsg_handler);

	// If there is no error, we can spare to write the error-message
	if ( (long)CS_SEVERITY(errmsg->msgnumber) != 0L ) {
		Anything *pAny = NULL;
		bool bFuncCode = false;
		{
			LockUnlockEntry me(SybCTnewDA::fgSybaseLocker);
			bFuncCode = SybCTnewDA::GetMessageAny(context, &pAny);
		}
		if ( bFuncCode ) {
			Anything anyData;
			CS_MSGNUM lMsgNumber = errmsg->msgnumber;
			anyData["MsgNumber"] = (long)lMsgNumber;
			// it only makes sense to decode the msgnumber if the severity is in decodable range
			// currently, only codes up to 7 are in range
			if ( CS_SEVERITY(lMsgNumber) <= CS_SV_FATAL ) {
				anyData["Severity"] = CS_SEVERITY(lMsgNumber);
				anyData["Number"] = CS_NUMBER(lMsgNumber);
				anyData["Origin"] = CS_ORIGIN(lMsgNumber);
				anyData["Layer"] = CS_LAYER(lMsgNumber);
			} else {
				anyData["Severity"] = (long)errmsg->severity;
			}

			String text = NotNull(errmsg->msgstring);
			// if a "\n" trails the text, cut it away
			if (text[text.Length()-1L] == '\n') {
				text.Trim(text.Length() - 1);
			}
			anyData["msgtext"] = text;
			if (errmsg->osstringlen > 0) {
				anyData["osstring"] = NotNull(errmsg->osstring);
			}
			TraceAny(anyData, "anyData");
			{
				// needs lock because other connections could also log errors
				LockUnlockEntry me(SybCTnewDA::fgSybaseLocker);
				(*pAny)["Messages"].Append(anyData);
				// The msgtext of the first severity unequal 0 must saved
				if ( !(*pAny).IsDefined("MainMsgErr")
					 && (long)errmsg->msgnumber != SQLSRV_ENVDB
					 && (long)CS_NUMBER(errmsg->msgnumber) != 0L ) {
					String help;
					help << "[" << anyData["msgtext"].AsString("") << "]";
					(*pAny)["MainMsgErr"] = help;
					(*pAny)["MainMsgErrNumber"] = (long)CS_NUMBER(errmsg->msgnumber);
				}
			}
		} else {
			SysLog::Error("SybCTnewDA_csmsg_handler: could not get Message anything");
		}
	}
	return CS_SUCCEED;
}
