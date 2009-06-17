#include "OracleConnection.h"

#include "SysLog.h"
#include "Dbg.h"

#include <string.h>		// for strlen
//----- OracleConnection -----------------------------------------------------------------
OracleConnection::OracleConnection() :
	fConnected(false), fEnvhp(0), fSrvhp(0), fErrhp(0), fSvchp(0), fUsrhp(0), fStmthp(0)
{
	StartTrace(OracleConnection.OracleConnection);
}

OracleConnection::~OracleConnection()
{
	StartTrace(OracleConnection.~OracleConnection);
	// disconnect if OracleConnection exists
	Close();
}

sword OracleConnection::AllocStmtHandle()
{
	StartTrace(OracleConnection.AllocStmtHandle);
	// allocates and returns new statement handle
	return OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fStmthp, (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
}

void OracleConnection::StmtCleanup()
{
	StartTrace(OracleConnection.StmtCleanup);
	if (fStmthp) {
		(void) OCIHandleFree((dvoid *) fStmthp, (ub4) OCI_HTYPE_STMT);
		fStmthp = 0;
	}
}

sword OracleConnection::GetReplyDescription()
{
	StartTrace(OracleConnection.GetReplyDescription);
	// retrieves descriptions of return values for a
	// given SQL statement

	return OCIStmtExecute(fSvchp, fStmthp, fErrhp, (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DESCRIBE_ONLY);
}

sword OracleConnection::ExecuteStmt()
{
	StartTrace(OracleConnection.ExecuteStmt);
	// executes a SQL statement (first row is also fetched)

	return OCIStmtExecute(fSvchp, fStmthp, fErrhp, (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_COMMIT_ON_SUCCESS);
}

sword OracleConnection::StmtPrepare(text *stmt)
{
	StartTrace(OracleConnection.StmtPrepare);
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

bool OracleConnection::Open(String const &strServer, String const &strUsername, String const &strPassword)
{
	StartTrace(OracleConnection.Open);

	text const *server(reinterpret_cast<const text *> ((const char *) strServer));
	text const *username(reinterpret_cast<const text *> ((const char *) strUsername));
	text const *password(reinterpret_cast<const text *> ((const char *) strPassword));

	if (fConnected) {
		SYSERROR("tried to open already opened connection to server [" << strServer << "] and user [" << strUsername << "]!");
		return false;
	}

	if (OCIInitialize((ub4) OCI_THREADED, (dvoid *) 0, (dvoid * ( *)(dvoid *, size_t)) 0, (dvoid * ( *)(dvoid *, dvoid *, size_t)) 0,
					  (void( *)(dvoid *, dvoid *)) 0)) {
		SysLog::Error("FAILED: OCIInitialize()");
		return false;
	}

	if (OCIEnvInit(&fEnvhp, (ub4) OCI_ENV_NO_MUTEX, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIEnvInit()");
		return false;
	}
	// --- alloc error handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fErrhp, (ub4) OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIHandleAlloc() on errhp");
		return false;
	}
	// --- alloc service context handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fSvchp, (ub4) OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIHandleAlloc() on svchp");
		return false;
	}
	// --- alloc server connection handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fSrvhp, (ub4) OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIHandleAlloc() on srvhp");
		return false;
	}
	// --- alloc user session handle
	if (OCIHandleAlloc((dvoid *) fEnvhp, (dvoid **) &fUsrhp, (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIHandleAlloc() on svchp");
		return false;
	}
	// --- attach server
	if (OCIServerAttach(fSrvhp, fErrhp, server, strlen((const char *) server), (ub4) OCI_DEFAULT)) {
		SysLog::Error("FAILED: OCIHandleAlloc() on svchp");
		return false;
	}
	// --- set attribute server context in the service context
	if (OCIAttrSet((dvoid *) fSvchp, (ub4) OCI_HTYPE_SVCCTX, (dvoid *) fSrvhp, (ub4) 0, (ub4) OCI_ATTR_SERVER, (OCIError *) fErrhp)) {
		SysLog::Error("FAILED: OCIAttrSet() server attribute");
		return false;
	}
	// --- logon

	// --- set attributes in the authentication handle
	if (OCIAttrSet((dvoid *) fUsrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) username, (ub4) strlen((const char *) username), (ub4) OCI_ATTR_USERNAME, fErrhp)) {
		SysLog::Error("FAILED: OCIAttrSet() userid"); // --- user name
		return false;
	}

	if (OCIAttrSet((dvoid *) fUsrhp, (ub4) OCI_HTYPE_SESSION, (dvoid *) password, (ub4) strlen((const char *) password), (ub4) OCI_ATTR_PASSWORD, fErrhp)) {
		SysLog::Error("FAILED: OCIAttrSet() passwd"); // --- user password
		return false;
	}

	Trace("connected to oracle as " << (const char *) username);

	if (OCISessionBegin(fSvchp, fErrhp, fUsrhp, OCI_CRED_RDBMS, (ub4) OCI_DEFAULT)) {
		SysLog::Error("FAILED: OCIAttrSet() passwd");
		return false;
	}

	// --- Set the authentication handle in the Service handle
	if (OCIAttrSet((dvoid *) fSvchp, (ub4) OCI_HTYPE_SVCCTX, (dvoid *) fUsrhp, (ub4) 0, OCI_ATTR_SESSION, fErrhp)) {
		SysLog::Error("FAILED: OCIAttrSet() session");
		return false;
	}
	fConnected = true;
	return fConnected;
}

bool OracleConnection::Close(bool bForce)
{
	StartTrace1(OracleConnection.Close, (bForce ? "" : "not ") << "forcing connection closing");
	if (fConnected) {
		if (OCISessionEnd(fSvchp, fErrhp, fUsrhp, (ub4) 0)) {
			SysLog::Error("FAILED: OCISessionEnd()");
		}
		if (OCIServerDetach(fSrvhp, fErrhp, (ub4) OCI_DEFAULT)) {
			SysLog::Error("FAILED: OCISessionEnd()");
		}
	}
	if (fSrvhp) {
		(void) OCIHandleFree((dvoid *) fSrvhp, (ub4) OCI_HTYPE_SERVER);
		fSrvhp = 0;
	}
	if (fSvchp) {
		(void) OCIHandleFree((dvoid *) fSvchp, (ub4) OCI_HTYPE_SVCCTX);
		fSvchp = 0;
	}
	if (fErrhp) {
		(void) OCIHandleFree((dvoid *) fErrhp, (ub4) OCI_HTYPE_ERROR);
		fErrhp = 0;
	}
	if (fUsrhp) {
		(void) OCIHandleFree((dvoid *) fUsrhp, (ub4) OCI_HTYPE_SESSION);
		fUsrhp = 0;
	}
	if (fEnvhp) {
		(void) OCIHandleFree((dvoid *) fEnvhp, (ub4) OCI_HTYPE_ENV);
		fEnvhp = 0;
	}
	fConnected = false;
	return true;
}

String OracleConnection::checkerr(OCIError *errhp, sword status, bool &estatus)
{
	StartTrace1(OracleConnection.checkerr, "status: " << (long) status);
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

	estatus = true;
	SysLog::Error(error);
	return error;
}
