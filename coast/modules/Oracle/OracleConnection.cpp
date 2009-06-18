#include "OracleConnection.h"

#include "SysLog.h"
#include "Dbg.h"

#include <string.h>		// for strlen

// set to 1 to track OCI's memory usage
#if (1)
dvoid *malloc_func(dvoid * /* ctxp */, size_t size)
{
	dvoid *ptr(malloc(size));
	StatTrace(OracleConnection.malloc_func, "size: " << (long)size << " ptr: &" << (long)ptr, Storage::Current());
	return ptr;
}
dvoid *realloc_func(dvoid * /* ctxp */, dvoid *ptr, size_t size)
{
	dvoid *nptr(realloc(ptr, size));
	StatTrace(OracleConnection.realloc_func, "size: " << (long)size << " oldptr: &" << (long)ptr << " new ptr: &" << (long)nptr, Storage::Current());
	return (nptr);
}
void free_func(dvoid * /* ctxp */, dvoid *ptr)
{
	StatTrace(OracleConnection.free_func, "ptr: &" << (long)ptr, Storage::Current());
	free(ptr);
}
#else
#	define	malloc_func		NULL
#	define	realloc_func	NULL
#	define	free_func		NULL
#endif

static struct OraTerminator {
	~OraTerminator() {
		OCITerminate(OCI_DEFAULT);
	}
} fgOraTerminator;

//----- OracleConnection -----------------------------------------------------------------
OracleConnection::OracleConnection() :
	fConnected(false),
	fEnvhp(OCI_HTYPE_ENV),
	fSrvhp(OCI_HTYPE_SERVER),
	fErrhp(OCI_HTYPE_ERROR),
	fSvchp(OCI_HTYPE_SVCCTX),
	fUsrhp(OCI_HTYPE_SESSION),
	fStmthp(OCI_HTYPE_STMT)
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
	return OCIHandleAlloc(fEnvhp.getHandle(), fStmthp.getVoidAddr(), (ub4) OCI_HTYPE_STMT, (size_t) 0, (dvoid **) 0);
}

void OracleConnection::StmtCleanup()
{
	StartTrace(OracleConnection.StmtCleanup);
	fStmthp.reset();
}

sword OracleConnection::GetReplyDescription()
{
	StartTrace(OracleConnection.GetReplyDescription);
	// retrieves descriptions of return values for a
	// given SQL statement

	return OCIStmtExecute(fSvchp.getHandle(), fStmthp.getHandle(), fErrhp.getHandle(), (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_DESCRIBE_ONLY);
}

sword OracleConnection::ExecuteStmt()
{
	StartTrace(OracleConnection.ExecuteStmt);
	// executes a SQL statement (first row is also fetched)

	return OCIStmtExecute(fSvchp.getHandle(), fStmthp.getHandle(), fErrhp.getHandle(), (ub4) 1, (ub4) 0, (CONST OCISnapshot *) NULL, (OCISnapshot *) NULL, OCI_COMMIT_ON_SUCCESS);
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
	checkerr(fErrhp.getHandle(), (sStatus = AllocStmtHandle()), error);
	if (error) {
		return sStatus;
	}
	return OCIStmtPrepare(fStmthp.getHandle(), fErrhp.getHandle(), stmt, (ub4) strlen((const char *) stmt), (ub4) OCI_NTV_SYNTAX, (ub4) OCI_DEFAULT);
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

	// caution: the following memory handles supplied must allocate on Storage::Global()
	//  because memory gets allocated through them in Open and freed in Close. Throughout the
	//  lifetime of the connection, mutliple threads could share the same connection and so we
	//  must take care not to allocate on the first Thread opening the connection
	if ( OCIEnvCreate(fEnvhp.getHandleAddr(), (ub4)(OCI_THREADED | OCI_ENV_NO_MUTEX), NULL, // context
					  malloc_func, // malloc function to allocate handles and env specific memory
					  realloc_func, // realloc function to allocate handles and env specific memory
					  free_func, // free function to allocate handles and env specific memory
					  0, // extra memory to allocate
					  NULL // pointer to user-memory
					 ) != OCI_SUCCESS) {
		SysLog::Error("FAILED: OCIEnvCreate(): could not create OCI environment");
		return false;
	}

	// --- alloc error handle
	if (OCIHandleAlloc(fEnvhp.getHandle(), fErrhp.getVoidAddr(), (ub4) OCI_HTYPE_ERROR, (size_t) 0, (dvoid **) 0) != OCI_SUCCESS ) {
		SysLog::Error("FAILED: OCIHandleAlloc(): alloc error handle failed");
		return false;
	}
	// --- alloc service context handle
	if (OCIHandleAlloc(fEnvhp.getHandle(), fSvchp.getVoidAddr(), (ub4) OCI_HTYPE_SVCCTX, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIHandleAlloc(): alloc service context handle failed");
		return false;
	}
	// --- alloc server connection handle
	if (OCIHandleAlloc(fEnvhp.getHandle(), fSrvhp.getVoidAddr(), (ub4) OCI_HTYPE_SERVER, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIHandleAlloc(): alloc server connection handle failed");
		return false;
	}
	// --- alloc user session handle
	if (OCIHandleAlloc(fEnvhp.getHandle(), fUsrhp.getVoidAddr(), (ub4) OCI_HTYPE_SESSION, (size_t) 0, (dvoid **) 0)) {
		SysLog::Error("FAILED: OCIHandleAlloc(): alloc user session handle failed");
		return false;
	}
	// --- attach server
	if (OCIServerAttach(fSrvhp.getHandle(), fErrhp.getHandle(), server, strlen((const char *) server), (ub4) OCI_DEFAULT)) {
		SysLog::Error(String("FAILED: OCIServerAttach() to server [") << strServer << "] failed");
		return false;
	}
	// --- set attribute server context in the service context
	if (OCIAttrSet(fSvchp.getHandle(), (ub4) OCI_HTYPE_SVCCTX, fSrvhp.getHandle(), (ub4) 0, (ub4) OCI_ATTR_SERVER, fErrhp.getHandle())) {
		SysLog::Error("FAILED: OCIAttrSet(): setting attribute <server> into the service context failed");
		return false;
	}

	// --- set attributes in the authentication handle
	if (OCIAttrSet(fUsrhp.getHandle(), (ub4) OCI_HTYPE_SESSION, (dvoid *) username, (ub4) strlen((const char *) username), (ub4) OCI_ATTR_USERNAME, fErrhp.getHandle())) {
		SysLog::Error("FAILED: OCIAttrSet(): setting attribute <username> in the authentication handle failed");
		return false;
	}

	if (OCIAttrSet(fUsrhp.getHandle(), (ub4) OCI_HTYPE_SESSION, (dvoid *) password, (ub4) strlen((const char *) password), (ub4) OCI_ATTR_PASSWORD, fErrhp.getHandle())) {
		SysLog::Error("FAILED: OCIAttrSet(): setting attribute <password> in the authentication handle failed");
		return false;
	}

	Trace("connected to oracle as " << strUsername);

	if (OCISessionBegin(fSvchp.getHandle(), fErrhp.getHandle(), fUsrhp.getHandle(), OCI_CRED_RDBMS, (ub4) OCI_DEFAULT)) {
		SysLog::Error("FAILED: OCISessionBegin()");
		return false;
	}

	// --- Set the authentication handle in the Service handle
	if (OCIAttrSet(fSvchp.getHandle(), (ub4) OCI_HTYPE_SVCCTX, fUsrhp.getHandle(), (ub4) 0, OCI_ATTR_SESSION, fErrhp.getHandle())) {
		SysLog::Error("FAILED: OCIAttrSet(): setting attribute <session> into the service context failed");
		return false;
	}

	fConnected = true;
	return fConnected;
}

bool OracleConnection::Close(bool bForce)
{
	StartTrace1(OracleConnection.Close, (bForce ? "" : "not ") << "forcing connection closing");
	if (fConnected) {
		if (OCISessionEnd(fSvchp.getHandle(), fErrhp.getHandle(), fUsrhp.getHandle(), (ub4) 0)) {
			SysLog::Error("FAILED: OCISessionEnd() on svchp failed");
		}
		if (OCIServerDetach(fSrvhp.getHandle(), fErrhp.getHandle(), (ub4) OCI_DEFAULT)) {
			SysLog::Error("FAILED: OCISessionEnd() on srvhp failed");
		}
	}
	fStmthp.reset();
	fSrvhp.reset();
	fSvchp.reset();
	fErrhp.reset();
	fUsrhp.reset();
	fEnvhp.reset();
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
