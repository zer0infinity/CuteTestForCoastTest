/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"
#include "Dbg.h"
#include "SysLog.h"
#include "StringStream.h"

//--- interface include --------------------------------------------------------
#include "SybDB.h"

int SybDB_err_handler(
	DBPROCESS    *dbproc,
	int          severity,
	int          dberr,
	int          oserr,
	char         *dberrstr,
	char         *oserrstr);

int SybDB_msg_handler(
	DBPROCESS       *dbproc,
	DBINT           msgno,
	int             msgstate,
	int             severity,
	char            *msgtext,
	char            *servername,
	char            *procname,
	int			    line);

//---- SybDB ----------------------------------------------------------------

SimpleMutex SybDB::fgMessagesMutex("SybDB::fgMessagesMutex");
Anything SybDB::fgMessages;

SybDB::SybDB()
{
}

SybDB::~SybDB()
{
}

bool SybDB::Init(void)
{
	StartTrace(SybDB.Init);
	if (dbinit() == FAIL) {
		Error("Init: Failed to initialize SyBase");
		return false;
	} else {
		// Install user-supplied error-handling and message-handling routines.
		dberrhandle(SybDB_err_handler);
		dbmsghandle(SybDB_msg_handler);
		return true;
	}
}

bool SybDB::Open(String user, String password, String server, String appl)
{
	StartTrace(SybDB.Open);

	// Get a LOGINREC
	flogin = dblogin();
	if (!flogin) {
		Error("Open: failed to allocate login structure");
		return false;
	}
	if (!user.Length()) {
		Warning("Open: Username missing");
	} else {
		if ( DBSETLUSER(flogin, (char *)(const char *)user) ) {
			// All right:  just to avoid negative logic (as HUA wishes)
		} else {
			Error("DBSETLUSER: failed to set the user into the login structure");
			return false;
		}
	}
	if (!password.Length()) {
		Warning("Open: Password missing");
	} else {
		if ( DBSETLPWD(flogin, (char *)(const char *)password) ) {
			// All right:  just to avoid negative logic (as HUA wishes)
		} else {
			Error("DBSETLPWD: failed to set the password into the login structure");
			return false;
		}
	}
	if (!server.Length()) {
		Warning("Open: Servername missing");
	}
	if (!appl.Length()) {
		Warning("Open: Applicationname missing");
	} else {
		if ( DBSETLAPP(flogin, (char *)(const char *)appl) ) {
			// All right:  just to avoid negative logic (as HUA wishes)
		} else {
			Error("DBSETLAPP: failed to set the application into the login structure");
			return false;
		}
	}
	// Get a DBPROCESS structure for communication with SQL Server.
	Trace("opening DB");
	fdbproc = dbopen(flogin, (char *)(const char *)server);
	// if no subsidiary dbopen call follows using the same fdbLogin structure we can free it here
	Trace("freeing loginbuffer");
	dbloginfree(flogin);
	if ( fdbproc == NULL ) {
		Error("Open: failed to open DB");
		return false;
	}

	// Specify fMessages as the conatiner for warnings and error-nessages
	dbsetuserdata( fdbproc, (BYTE *)&fMessages );

	return true;
}

bool SybDB::SqlExec(String cmd)
{
	StartTrace(SybDB.SqlExec);
	Trace("dbcmd: [" << cmd << "]");
	// First, put the command into the command buffer.
	if ( (fret_code = dbcmd(fdbproc, (char *)(const char *)cmd)) == SUCCEED ) {
		Trace("dbcmd successful");
		Trace("dbsqlexec");
		// Send the command to SQL Server and start execution
		if ( (fret_code = dbsqlexec(fdbproc) ) == SUCCEED ) {
			Trace("dbsqlexec successful");
			return true;
		} else {
			Trace("dbsqlexec not successful");
			Error("dbcmd: failed to send the command to SQL Server");
			return false;
		}
	} else {
		Trace("dbcmd not successful");
		Error("dbcmd: failed to put the command into the command buffer");
		return false;
	}
}

bool SybDB::DoGetResult( Anything &anyResult )
{
	StartTrace(SybDB.DoGetResult);
	bool boRet = false;
	bool bStoredPRetStat = true;

	while ((fret_code = dbresults(fdbproc)) != NO_MORE_RESULTS && fret_code != FAIL) {
		Trace("entering processing of query results");
		if (fret_code == SUCCEED) {
			// check if the real command succeeded, especially when using stored procedures
			// or multiple sql-commands the return code of dbsqlexec does not tell about
			// the success of it
			bool bDBHasRows = (DBROWS(fdbproc) == SUCCEED);
			bool bStoredPHasRetStat = (dbhasretstat(fdbproc) == TRUE);
			Trace("StoredProcedureHasReturnStatus: " << (bStoredPHasRetStat ? "true" : "false"));
			long tmp = dbretstatus(fdbproc);
			Trace( String() << "!!! dbretstatus: " << tmp);
			bStoredPRetStat = (tmp == 0);  // SUCCEED;
			Trace("StoredProcedureReturnStatus: " << (bStoredPRetStat ? "true" : "false"));
			if ( bStoredPHasRetStat && !bStoredPRetStat ) {
				Trace("StoredProcedureReturnStatus: runtime-error encountered");
				break;
			}
			Trace("ReturnStatus of StoredProcedure: " << (bStoredPRetStat ? "true" : "false"));
			Trace("before entering HasRows(" << (bDBHasRows ? "true" : "false") << ")");
			if (bDBHasRows) {
				Anything anyColumnInfo;
				Trace("ok for reading rowdata");
				long iMax = dbnumcols(fdbproc);
				Trace("getting column informations for " << iMax << " rows");
				for ( long i = 1; i <= iMax; i++ ) {
					// What if dbcolname returns a NULL-value ?  XXXX
					anyColumnInfo[i-1] = dbcolname(fdbproc, i);
				}
				TraceAny(anyColumnInfo, "ColInfo");
				long iRowCount = 1;
				while ( (fret_code = dbnextrow(fdbproc)) != NO_MORE_ROWS ) {
					for ( long i = 1; i <= iMax; i++ ) {
						char myBuf[1024];
						// Bind results to program variables.
						DBINT ldatlen = dbdatlen(fdbproc, i);
						switch (ldatlen) {
							case -1: {
								Error(String("invalid column (") << i << ") accessed");
								break;
							}
							case 0: {
								Trace("RetVal: - row: " << iRowCount << " col:" << i << " datlen: " << (long)ldatlen << " data:[NULL]");
								// Collect the useful info for the result:  the values of the fields of the current row.
								anyResult[String() << (long)(iRowCount-1)][anyColumnInfo[i-1].AsString()] = Anything();
								break;
							}
							default:
								// I suppose that dbconvetr even fails if dbcoltype or dbdata have failed.
								long dbcRetval = dbconvert( fdbproc, dbcoltype(fdbproc, i), (dbdata(fdbproc, i)), ldatlen, SYBTEXT, (CS_BYTE *)myBuf, -1);
								Trace("RetVal: " << dbcRetval << " row: " << iRowCount << " col:" << i << " datlen: " << (long)ldatlen << " data:[" << myBuf << "]");
								if ( ( -1 == dbcRetval ) || ( FAIL == dbcRetval ) ) {
									fret_code = FAIL;
								} else {
									// Collect the useful info for the result:  the values of the fields of the current row.
									anyResult[String() << (long)(iRowCount-1)][anyColumnInfo[i-1].AsString()] = myBuf;
								}
						}
					}
					iRowCount++;
				}
			} else {
				// might be a INSERT, UPDATE and so on command
				// if everything was OK, fret_code is correct for returning a true value
			}
		}
	}
	TraceAny( anyResult, "DoGetResult returns: ");
	Trace ( String( "fret_code = ") << (long)fret_code << " !!!" );
	// SUCCEED and NO_MORE_RESULTS are positive results coming from dbresults
	// NO_MORE_ROWS is the positive result coming from dbnextrow (no more pending results)
	// bStoredPRetStat is true if there have not been runtime-errors
	//
	// Other values of fret_code mean ERROR
	//
	// If a stored procedure contains an insert statement and the user does not have
	// insert permission, dbresults will return SUCCEED.  To check runtime-errors inside
	// stored procedures, use the dbretstatus routine!!!
	if  ( ( bStoredPRetStat ) && (fret_code == SUCCEED || fret_code == NO_MORE_RESULTS || fret_code == NO_MORE_ROWS) ) {
		boRet = true;
	}
	Trace ( String( "boRet = ") << (long)boRet << " !!!!!" );
	return boRet;
}

bool SybDB::GetResult( Anything &anyResult )
{
	return DoGetResult( anyResult );
}

bool SybDB::Close(void)
{
	StartTrace(SybDB.Close);
	dbexit();
	return true;
}

void SybDB::Warning(String str)
{
	String strErr("SybDB::");
	strErr << str;
	SysLog::Warning(strErr);
	{
		SimpleMutexEntry aME(SybDB::fgMessagesMutex);
		aME.Use();
		SybDB::fgMessages["Messages"].Append(strErr);
	}
}

void SybDB::Error(String str)
{
	String strErr("SybDB::");
	strErr << str;
	SysLog::Error(strErr);
	{
		SimpleMutexEntry aME(SybDB::fgMessagesMutex);
		aME.Use();
		SybDB::fgMessages["Messages"].Append(strErr);
	}
}

int SybDB_msg_handler(
	DBPROCESS       *dbproc,
	DBINT           msgno,
	int             msgstate,
	int             severity,
	char            *msgtext,
	char            *servername,
	char            *procname,
	int    			line)
{
	Anything anyData;
	StringStream ss;
	anyData["severity"] = (long)severity;
	anyData["msgno"] = (long)msgno;
	anyData["msgstate"] = (long)msgstate;
	anyData["msgtext"] = NotNull(msgtext);
	anyData["servername"] = NotNull(servername);
	anyData["procname"] = NotNull(procname);
	if (line != 0) {
		anyData["line"] = (long)line;
	}
	anyData.PrintOn(ss);
	{
		SimpleMutexEntry aME(SybDB::fgMessagesMutex);
		aME.Use();
		SybDB::fgMessages["Messages"].Append(anyData.DeepClone());
		// The msgtext of the first severity unequal 0 must saved
		if ( (long)severity != 0 && !SybDB::fgMessages.IsDefined("MainMsgErr") ) {
			String help;
			help << "[" << anyData["msgtext"].AsString("") << "]";
			SybDB::fgMessages["MainMsgErr"] = help;
		}
	}

	if (dbproc != NULL) {
		SysLog::Warning(String("SybDB_msg_handler:\n") << ss.str());
	}

	return 0;
}

int SybDB_err_handler(
	DBPROCESS    *dbproc,
	int          severity,
	int          dberr,
	int          oserr,
	char         *dberrstr,
	char         *oserrstr)
{
	Anything anyData;
	StringStream ss;
	anyData["severity"] = (long)severity;
	if (dberr != DBNOERR) {
		anyData[String("dberr(") << (long)dberr << ")"] = NotNull(dberrstr);
	}
	if (oserr != DBNOERR) {
		anyData[String("oserr(") << (long)oserr << ")"] = NotNull(oserrstr);
	}
	anyData.PrintOn(ss);
	{
		SimpleMutexEntry aME(SybDB::fgMessagesMutex);
		aME.Use();
		SybDB::fgMessages["Messages"].Append(anyData.DeepClone());
		// The msgtext of the first severity unequal 0 must saved
		if ( (long)severity != 0 && !SybDB::fgMessages.IsDefined("MainMsgErr") ) {
			String errStr;
			errStr << "dberr: ["
				   << anyData[String("dberr(") << (long)dberr << ")"].AsString("")
				   << "] --- oserr: ["
				   << anyData[String("oserr(") << (long)oserr << ")"].AsString("")
				   << "]";

			SybDB::fgMessages["MainMsgErr"] = errStr;
		}

	}
	if ((dbproc == NULL) || (DBDEAD(dbproc))) {
		SysLog::Error(String("SybDB_err_handler: dbproc structure is null (db allocation error)\n") << ss.str());
		return INT_CANCEL;
	} else {
		SysLog::Error(String("SybDB_err_handler:\n") << ss.str());
		return INT_CANCEL;
	}
}
