/*
 * O8Connection.cpp
 *
 *  Created on: Jun 15, 2009
 *      Author: m1huber
 */

#include "O8Connection.h"

#include "SysLog.h"
#include "Dbg.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

//#include <ctype.h>

//----- O8Connection -----------------------------------------------------------------
O8Connection::O8Connection(text *username, text *password) :
	fEnvhp(0), fSrvhp(0), fErrhp(0), fSvchp(0), fUsrhp(0)
{
	fUser = String((char *) username);
	fConnected = ConnectOracleUser(username, password);
}

O8Connection::~O8Connection()
{
	// disconnect if O8Connection exists
	using namespace std;
	if (fConnected) {
		if (OCISessionEnd(fSvchp, fErrhp, fUsrhp, (ub4) 0)) {
			cerr << "FAILED: OCISessionEnd()" << endl;
		}
		if (OCIServerDetach(fSrvhp, fErrhp, (ub4) OCI_DEFAULT)) {
			cerr << "FAILED: OCISessionEnd()" << endl;
		}
	}
	if (fSrvhp) {
		(void) OCIHandleFree((dvoid *) fSrvhp, (ub4) OCI_HTYPE_SERVER);
	}
	if (fSvchp) {
		(void) OCIHandleFree((dvoid *) fSvchp, (ub4) OCI_HTYPE_SVCCTX);
	}
	if (fErrhp) {
		(void) OCIHandleFree((dvoid *) fErrhp, (ub4) OCI_HTYPE_ERROR);
	}
	if (fUsrhp) {
		(void) OCIHandleFree((dvoid *) fUsrhp, (ub4) OCI_HTYPE_SESSION);
	}
	if (fEnvhp) {
		(void) OCIHandleFree((dvoid *) fEnvhp, (ub4) OCI_HTYPE_ENV);
	}
}

sword O8Connection::AllocStmtHandle()
{
	// allocates and returns new statement handle
	return OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) fStmthp, (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
}

void O8Connection::StmtCleanup()
{
	if (fStmthp) {
		(void) OCIHandleFree((dvoid *) fStmthp, (ub4) OCI_HTYPE_STMT);
		fStmthp = 0;
	}
}

sword O8Connection::GetReplyDescription()
{
	// retrieves descriptions of return values for a
	// given SQL statement

	return OCIStmtExecute(fSvchp, fStmthp, fErrhp, (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DESCRIBE_ONLY);
}

sword O8Connection::ExecuteStmt()
{
	// executes a SQL statement (first row is also fetched)

	return OCIStmtExecute(fSvchp, fStmthp, fErrhp, (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_COMMIT_ON_SUCCESS);
}

sword O8Connection::StmtPrepare(text *stmt)
{
	// prepare SQL statement for execution (not much to do in
	// our environment, since the complete statement is already
	// constructed and binds of input variables are not necessary)

	// --- alloc statement handle
	bool error(false);
	sword sStatus;
	checkerr(fErrhp, (sStatus = AllocStmtHandle()), error);
	if (error) {
		return sStatus;
	}
	return OCIStmtPrepare(fStmthp, fErrhp, stmt, (ub4) strlen((const char *) stmt), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
}

bool O8Connection::ConnectOracleUser(text *username, text *password)
{
	using namespace std;
	if (OCIInitialize((ub4) OCI_THREADED, (dvoid *) 0, (dvoid * ( *)(dvoid *, size_t)) 0, (dvoid * ( *)(dvoid *, dvoid *, size_t)) 0,
					  (void( *)(dvoid *, dvoid *)) 0)) {
		cerr << "FAILED: OCIInitialize()" << endl;
		return FALSE;
	}

	//	if (OCIHandleAlloc((dvoid *) NULL, (dvoid **) &fEnvhp, (ub4) OCI_HTYPE_ENV,
	//			(size_t) 0, (dvoid **) 0)) {	// unnecessary
	//		cerr << "FAILED: OCIHandleAlloc() on envhp" << endl;
	//		return FALSE;
	//	}
	if (OCIEnvInit(&fEnvhp, (ub4) OCI_ENV_NO_MUTEX, (size_t) 0, (dvoid **) 0)) {
		cerr << "FAILED: OCIEnvInit()" << endl;
		return FALSE;
	}
	// --- alloc error handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fErrhp, (ub4) OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0)) {
		cerr << "FAILED: OCIHandleAlloc() on errhp" << endl;
		return FALSE;
	}
	// --- alloc service context handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fSvchp, (ub4) OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0)) {
		cerr << "FAILED: OCIHandleAlloc() on svchp" << endl;
		return FALSE;
	}
	// --- alloc server connection handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fSrvhp, (ub4) OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)) {
		cerr << "FAILED: OCIHandleAlloc() on srvhp" << endl;
		return FALSE;
	}
	// --- alloc user session handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fUsrhp, (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0)) {
		cerr << "FAILED: OCIHandleAlloc() on svchp" << endl;
		return FALSE;
	}
	// --- attach server
	static text *srvName = (text *) "//sifs-coast1.hsr.ch:1521/orcl";
	if (OCIServerAttach(fSrvhp, fErrhp, srvName, strlen((const char *) srvName), (ub4) OCI_DEFAULT)) {
		cerr << "FAILED: OCIHandleAlloc() on svchp" << endl;
		return FALSE;
	}
	// --- set attribute server context in the service context
	if (OCIAttrSet((dvoid *) fSvchp, (ub4) OCI_HTYPE_SVCCTX, (dvoid *) fSrvhp, (ub4) 0, (ub4) OCI_ATTR_SERVER, (OCIError *) fErrhp)) {
		cerr << "FAILED: OCIAttrSet() server attribute" << endl;
		return FALSE;
	}
	// --- logon

	// --- set attributes in the authentication handle
	if (OCIAttrSet((dvoid *) fUsrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) username, (ub4) strlen((const char *) username), (ub4) OCI_ATTR_USERNAME, fErrhp)) {
		cerr << "FAILED: OCIAttrSet() userid" << endl; // --- user name
		return FALSE;
	}

	if (OCIAttrSet((dvoid *) fUsrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) password, (ub4) strlen((const char *) password), (ub4) OCI_ATTR_PASSWORD, fErrhp)) {
		cerr << "FAILED: OCIAttrSet() passwd" << endl; // --- user password
		return FALSE;
	}

	cerr << "connected to oracle as " << (const char *) username << endl;

	if (OCISessionBegin(fSvchp, fErrhp, fUsrhp, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT)) {
		cerr << "FAILED: OCIAttrSet() passwd" << endl;
		return FALSE;
	}

	// --- Set the authentication handle in the Service handle
	if (OCIAttrSet((dvoid *) fSvchp, (ub4) OCI_HTYPE_SVCCTX, (dvoid *) fUsrhp, (ub4) 0, OCI_ATTR_SESSION, fErrhp)) {
		cerr << "FAILED: OCIAttrSet() session" << endl;
		return FALSE;
	}

	return TRUE;
}

String O8Connection::checkerr(OCIError *errhp, sword status, bool &estatus)
{
	StartTrace(O8Connection.checkerr);
	// error handling: checks 'status' for errors
	// in case of an error an error message is generated,
	// written to 'cerr' and also to the output ResultMapper
	// 'estatus' is used to signal if an error has occured

	text errbuf[512];
	ub4 buflen;
	sb4 errcode;
	String error;

	switch (status) {
		case OCI_NO_DATA:
		case OCI_SUCCESS:
			// no error
			return error;
			break;
		case OCI_SUCCESS_WITH_INFO:
			error << "Error - OCI_SUCCESS_WITH_INFO";
			break;
		case OCI_NEED_DATA:
			error << "Error - OCI_NEED_DATA";
			break;
		case OCI_ERROR:
			if (errhp) {
				OCIErrorGet((dvoid *) errhp, (ub4) 1, (text *) NULL, &errcode, errbuf, (ub4) sizeof(errbuf), (ub4) OCI_HTYPE_ERROR);
			}
			error << "Error - " << (char *) errbuf;
			break;
		case OCI_INVALID_HANDLE:
			error << "Error - OCI_INVALID_HANDLE";
			break;
		case OCI_STILL_EXECUTING:
			error << "Error - OCI_STILL_EXECUTE";
			break;
		case OCI_CONTINUE:
			error << "Error - OCI_CONTINUE";
			break;
		default:
			break;
	}

	estatus = TRUE;
	SysLog::Error(error);
	return error;
}
