/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PersistentLDAPConnection.h"
#include "LDAPConnectionManager.h"
#include "Base64.h"
#include "MD5.h"
#include "SystemLog.h"
#include <cstring>
#include <errno.h>
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
	SystemLog::Info(String("PersistentLDAPConnection::tsdsetup() Setting thread specific data. Thread [") <<  Thread::MyId() << "]");
}

void PersistentLDAPConnection::tsd_destruct(void *tsd)
{
	StartTrace(PersistentLDAPConnection.tsd_destruct);
	if ( tsd != (void *) NULL ) {
		SystemLog::Info(String("PersistentLDAPConnection::tsd_destruct() Releasing thread specific data. Thread [") <<  Thread::MyId() << "]");
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

bool PersistentLDAPConnection::SetErrnoHandler(LDAPErrorHandler &eh)
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

bool PersistentLDAPConnection::DoReleaseHandleInfo()
{
	StartTrace(PersistentLDAPConnection.DoReleaseHandleInfo);
	return LDAPConnectionManager::LDAPCONNMGR()->ReleaseHandleInfo(fMaxConnections, fPoolId);
}

void PersistentLDAPConnection::DoHandleBindFailure(LDAPErrorHandler &eh, String &errMsg)
{
	StartTrace1(PersistentLDAPConnection.DoHandleBindFailure, "Binding request FAILED!");
	eh.HandleSessionError(fHandle, errMsg);
	Disconnect();
}

LDAPConnection::EConnectState PersistentLDAPConnection::DoConnect(ROAnything bindParams, LDAPErrorHandler &eh)
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

void PersistentLDAPConnection::DoHandleWaitForResultTimeout(LDAPErrorHandler &eh)
{
	StartTrace(PersistentLDAPConnection.DoHandleWaitForResultTimeout);
	if ( fTryAutoRebind ) {
		eh.SetShouldRetry();
	}
}

void PersistentLDAPConnection::DoHandleWait4ResultError(LDAPErrorHandler &eh)
{
	StartTrace(PersistentLDAPConnection.DoHandleWait4ResultError);
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
