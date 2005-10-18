/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PersistentLDAPConnection.h"

//--- project modules used -----------------------------------------------------
#include "LDAPMessageEntry.h"
#include "LDAPConnectionManager.h"
#include "ldap-extension.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "Threads.h"
#include "StringStream.h"
#include "Base64.h"
#include "MD5.h"
#include "SysLog.h"

//--- c-modules used -----------------------------------------------------------
#include <string.h>

/* Function to set up thread-specific data. */
void PersistentLDAPConnection::tsd_setup()
{
	StartTrace(PersistentLDAPConnection.tsd_setup);
	void *tsd = NULL;
	if ( GETTLSDATA(LDAPConnectionManager::fgErrnoKey, tsd, void) ) {
		Trace("Thread specific data for fgErrnoKey already set up. Thread [" << Thread::MyId() << "]");
		return;
	}

	tsd = (void *) calloc( 1, sizeof(ldap_error) );
	if ( !SETTLSDATA(LDAPConnectionManager::fgErrnoKey, tsd) ) {
		Trace("Setting Thread specific data for fgErrnoKey failed. Thread [" << Thread::MyId() << "]");
	}
	SysLog::Info(String("PersistentLDAPConnection::tsdsetup() Setting thread specific data. Thread [") <<  Thread::MyId() << "]");
}

void PersistentLDAPConnection::tsd_destruct(void *tsd)
{
	StartTrace(PersistentLDAPConnection.tsd_destruct);
	if ( tsd != (void *) NULL ) {
		SysLog::Info(String("PersistentLDAPConnection::tsd_destruct() Releasing thread specific data. Thread [") <<  Thread::MyId() << "]");
		free(tsd);
	}
}

/* Function for setting an LDAP error. */
void PersistentLDAPConnection::set_ld_error( int err, char *matched, char *errmsg, void *dummy )
{
	StartTrace(PersistentLDAPConnection.set_ld_error);

	PersistentLDAPConnection::ldap_error *le = NULL;
	GETTLSDATA(LDAPConnectionManager::fgErrnoKey, le, ldap_error);
	le->le_errno = err;
	if ( le->le_matched != NULL ) {
		ldap_memfree( le->le_matched );
	}
	le->le_matched = matched;
	if ( le->le_errmsg != NULL ) {
		ldap_memfree( le->le_errmsg );
	}
	le->le_errmsg = errmsg;
}

/* Function for getting an LDAP error. */
int PersistentLDAPConnection::get_ld_error( char **matched, char **errmsg, void *dummy )
{
	StartTrace(PersistentLDAPConnection.get_ld_error);

	PersistentLDAPConnection::ldap_error *le = NULL;
	GETTLSDATA(LDAPConnectionManager::fgErrnoKey, le, ldap_error);
	if ( matched != NULL ) {
		*matched = le->le_matched;
	}
	if ( errmsg != NULL ) {
		*errmsg = le->le_errmsg;
	}
	return( le->le_errno );
}

/* Function for setting errno. */
void PersistentLDAPConnection::set_errno( int err )
{
	StartTrace(PersistentLDAPConnection.set_errno);
	errno = err;
}

/* Function for getting errno. */
int PersistentLDAPConnection::get_errno( void )
{
	StartTrace(PersistentLDAPConnection.get_errno);
	return( errno );
}

PersistentLDAPConnection::PersistentLDAPConnection(ROAnything connectionParams) : LDAPConnection(connectionParams)
{
	StartTrace(PersistentLDAPConnection.PersistentLDAPConnection);
	fRebindTimeout = connectionParams["RebindTimeout"].AsLong(0L);
	fTryAutoRebind = connectionParams["TryAutoRebind"].AsLong(0L) > 0L;
	fMaxConnections = connectionParams["MaxConnections"].AsLong(0L);
	TraceAny(connectionParams, "ConnectionParams");
	fPoolId = "";
	tsd_setup();
}

PersistentLDAPConnection::~PersistentLDAPConnection()
{
	StartTrace(PersistentLDAPConnection.~PersistentLDAPConnection);
}

int PersistentLDAPConnection::GetMaxConnections()
{
	StartTrace(PersistentLDAPConnection.GetMaxConnections);
	return fMaxConnections;
}

bool PersistentLDAPConnection::SetErrnoHandler(LDAPErrorHandler eh)
{
	StartTrace(PersistentLDAPConnection.SetErrnoHandler);

	struct ldap_thread_fns  tfns;

	/* Set the function pointers for dealing with mutexes
	and error information. */
	memset( &tfns, '\0', sizeof(struct ldap_thread_fns) );
	tfns.ltf_get_errno = get_errno;
	tfns.ltf_set_errno = set_errno;
	tfns.ltf_get_lderrno = get_ld_error;
	tfns.ltf_set_lderrno = set_ld_error;
	tfns.ltf_lderrno_arg = NULL;

	/* Set up this session to use those function pointers. */
	if ( ::ldap_set_option( fHandle, LDAP_OPT_THREAD_FN_PTRS, (void *) &tfns ) != LDAP_SUCCESS ) {
		Trace("ldap_set_option: LDAP_OPT_THREAD_FN_PTRS FAILED");
		eh.HandleSessionError(fHandle, "Could not set errno handlers.");
		return false;
	}
	return true;
}

PersistentLDAPConnection::EConnectState PersistentLDAPConnection::DoConnectHook(ROAnything bindParams, LDAPErrorHandler &eh)
{
	StartTrace(PersistentLDAPConnection.DoConnectHook);
	TraceAny(bindParams, "bindParams");
	fPoolId = GetLdapPoolId(bindParams);
	Anything returned = LDAPConnectionManager::LDAPCONNMGR()->GetLdapConnection(eh.IsRetry(), GetMaxConnections(), fPoolId, fRebindTimeout);
	PersistentLDAPConnection::EConnectState eConnectState =
		returned["MustRebind"].AsBool(1) == false ?
		PersistentLDAPConnection::eMustNotRebind  : PersistentLDAPConnection::eMustRebind;
	fHandle = (LDAP *) returned["Handle"].AsIFAObject(0);
	if ( eConnectState == PersistentLDAPConnection::eMustNotRebind ) {
		Trace("Retrning: " << ConnectRetToString(PersistentLDAPConnection::eOk));
		eConnectState = PersistentLDAPConnection::eOk;
	}
	Trace("eConnectState: " << ConnectRetToString(eConnectState) << " Handle: " << DumpConnectionHandle(fHandle));
	return eConnectState;
}

bool PersistentLDAPConnection::ReleaseHandleInfo()
{
	StartTrace(PersistentLDAPConnection.ReleaseHandleInfo);
	bool ret = LDAPConnectionManager::LDAPCONNMGR()->ReleaseHandleInfo(fMaxConnections, fPoolId);
	return ret;
}

bool PersistentLDAPConnection::Bind(String bindName, String bindPW, int &msgId, LDAPErrorHandler eh)
{
	StartTrace(LDAPConnection.PersistentLDAPConnection);

	String errMsg = "Binding request failed. ";
	errMsg << "[Server: '" << fServer << "', Port: '" << fPort << "'] ";

	if ( bindName.IsEqual("") || bindName.IsEqual("Anonymous") ) {
		Trace("Binding as Anonymous");
		errMsg << " (Anonymous bind.)";
		msgId = ::ldap_simple_bind( fHandle, NULL, NULL );
	} else {
		errMsg << " (Authorized bind.)";

		if ( bindPW == "undefined" ) {
			Trace("Bindpassword NOT OK: pwd = ["  << bindPW << "]");
			eh.HandleSessionError(fHandle, errMsg);
			return false;
		} else {
			Trace("Binding with <" << bindName << "><" << bindPW << ">");
			msgId = ::ldap_simple_bind( fHandle, bindName, bindPW );
		}
	}

	// bind request successful?
	if ( msgId == -1 ) {
		Trace("Binding request FAILED!");
		eh.HandleSessionError(fHandle, errMsg);
		Disconnect();
		return false;
	}

	Trace("Binding request SUCCEEDED, waiting for connection...");
	return true;
}

LDAPConnection::EConnectState PersistentLDAPConnection::DoConnect(ROAnything bindParams, LDAPErrorHandler eh)
{
	StartTrace(PersistentLDAPConnection.DoConnect);

	// set handle to null
	fHandle = NULL;
	String errMsg;
	String bindName = bindParams["BindName"].AsString("");
	String bindPW = bindParams["BindPW"].AsString("");
	long maxConnections = bindParams["MaxConnections"].AsLong(0);
	PersistentLDAPConnection::EConnectState eConnectState = PersistentLDAPConnection::eNok;
	if ( (eConnectState = DoConnectHook(bindParams, eh)) == PersistentLDAPConnection::eOk ) {
		return eConnectState;
	}
	// get connection handle
	if ( !(fHandle = Init(eh)) ) {
		return eInitNok;
	}

	// set protocol, timeout and rebind procedure
	if ( !SetProtocol(eh)			||
		 !SetConnectionTimeout(eh)	||
		 !SetSearchTimeout(eh)		||
		 !SetErrnoHandler(eh)
		 /* || !SetRebindProc(eh) */  ) {
		return eSetOptionsNok;
	}
	// send bind request (asynchronous)
	int msgId;
	if ( !Bind(bindName, bindPW, msgId, eh) ) {
		return eBindNok;
	}

	// wait for bind result (using msgId)
	Anything result;
	bool ret = WaitForResult(msgId, result, eh);
	if ( ret ) {
		LDAPConnectionManager::LDAPCONNMGR()->SetLdapConnection(maxConnections, fPoolId, fHandle);
		if ( eConnectState == PersistentLDAPConnection::eMustRebind ) {
			return PersistentLDAPConnection::eRebindOk;
		}
	}
	return ret ? PersistentLDAPConnection::eOk : PersistentLDAPConnection::eBindNok;
}

bool PersistentLDAPConnection::WaitForResult(int msgId, Anything &result, LDAPErrorHandler &eh)
{
	StartTrace(PersistentLDAPConnection.WaitForResult);

	timeval tv;
	tv.tv_sec  = fSearchTimeout;
	tv.tv_usec = 0;

	timeval *tvp = (fSearchTimeout == 0) ? NULL : &tv;

	bool finished = false;
	bool success = false;

	String errMsg;
	int resultCode;

	LDAPMessage *ldapResult;
	LDAPMessageEntry lmAutoDestruct(&ldapResult);	// automatic destructor for LDAPMessage
	lmAutoDestruct.Use();

	while (!finished) {
		// wait for result
		resultCode = ldap_result(fHandle, msgId, 1, tvp, &ldapResult);

		// check result
		if (resultCode == -1 && fSearchTimeout == 0) {
			// error, abandon!
			Trace("WaitForResult [Timeout: 0] received an error");
			int opRet;
			ldap_parse_result( fHandle, ldapResult, &opRet, NULL, NULL, NULL, NULL, 0 );
			errMsg << "Synchronous Wait4Result: ErrorCode: [" << (long)opRet << "] ErrorMsg: " << ldap_err2string( opRet );
			HandleWait4ResultError(msgId, errMsg, eh);
			finished = true;
		} else if (resultCode == 0 || (resultCode == -1 && fSearchTimeout != 0)) {
			// resultCode 0 means timeout, abandon
			Trace("WaitForResult [Timeout != 0] encountered a timeout ...");
			errMsg << "Asynchronous Wait4Result: The request <" << (long) msgId << "> timed out.";
			HandleWait4ResultError(msgId, errMsg, eh);
			if ( fTryAutoRebind ) {
				eh.SetShouldRetry();
			}
			finished = true;
		} else {
			// received a result
			int errCode = ldap_result2error(fHandle, ldapResult, 0);
			if (errCode == LDAP_SUCCESS || errCode == LDAP_SIZELIMIT_EXCEEDED) {
				Trace("WaitForResult recieved a result and considers it to be ok ...");
				success = true;

				// transform LDAPResult into an Anything with Meta Information
				TransformResult(ldapResult, result, eh.GetQueryParams());

				// add extra flag to inform client, if sizelimit was exceeded
				if (errCode == LDAP_SIZELIMIT_EXCEEDED) {
					result["SizeLimitExceeded"] = 1;
				}
			} else if (errCode == LDAP_COMPARE_FALSE || errCode == LDAP_COMPARE_TRUE) {
				Trace("WaitForResult recieved a result and considers it to be ok (compare) ...");
				success = true;

				// this is a bit special
				int rc;
				ldap_get_option(fHandle, LDAP_OPT_ERROR_NUMBER, &rc);
				result["Type"] = "LDAP_RES_COMPARE";
				result["Equal"] = (errCode == LDAP_COMPARE_TRUE);
				result["LdapCode"] = rc;
				result["LdapMsg"] = ldap_err2string(rc);
			} else {
				Trace("WaitForResult recieved a result and considers it to be WRONG ...");
				errMsg = "LDAP request failed.";
				eh.HandleSessionError(fHandle, errMsg);
			}
			finished = true;
		}
	}
	return success;
}

void PersistentLDAPConnection::HandleWait4ResultError(int msgId, String &errMsg, LDAPErrorHandler eh)
{
	StartTrace(PersistentLDAPConnection.HandleWait4ResultError);
	if ( msgId != -1 ) {
		int errCode = ldap_abandon(fHandle, msgId);
		errMsg << " Request abandoned: " << (errCode == LDAP_SUCCESS ? " successfully." : " FAILED!");
	} else {
		errMsg << " Request abandoned: binding handle was invalid, ldap_abandon not called";
	}
	eh.HandleSessionError(fHandle, errMsg);
	// Invalidate our handle because LDAP server might be down, will rebind next time!
	LDAPConnectionManager::LDAPCONNMGR()->SetLdapConnection(GetMaxConnections(), fPoolId, (LDAP *) NULL);
}

String PersistentLDAPConnection::Base64ArmouredMD5Hash(const String &text)
{
	StartTrace(PersistentLDAPConnection.Base64ArmouredMD5Hash);
	String hash, result;
	MD5Signer::DoHash(text, hash);
	Base64Regular b64("b64reg");
	b64.DoEncode(result, hash);
	return result;
}

String PersistentLDAPConnection::GetLdapPoolId(ROAnything bindParams)
{
	StartTrace(PersistentLDAPConnection.GetLdapPoolId);
	String bindName = bindParams["BindName"].AsString("");
	String bindPW 	= Base64ArmouredMD5Hash(bindParams["BindPW"].AsString(""));

	return	String("Host[") << fServer << "] Port[" << fPort << "] DN[" <<
			bindName << "] BindPW[" << bindPW << "] ConnTimeout[" << fConnectionTimeout << "]";
}

String PersistentLDAPConnection::GetLdapPoolId(const String &server, long port, const String &bindName,
		const String &bindPW, long connectionTimeout)
{
	StartTrace(PersistentLDAPConnection.GetLdapPoolId);
	String armouredPW = Base64ArmouredMD5Hash(bindPW);

	return	String("Host[") << server << "] Port[" << port << "] DN[" <<
			bindName << "] BindPW[" << armouredPW << "] ConnTimeout[" << connectionTimeout << "]";
}
