/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPConnection_H
#define _LDAPConnection_H

//---- baseclass include --------------------------------------------------
#include "config_LDAPDA.h"
#include "LDAPErrorHandler.h"

//---- LDAPConnection ----------------------------------------------------------
//! <B>Wraps an LDAP connection and handles connecting and disconnecting.</B>

class EXPORTDECL_LDAPDA LDAPConnection
{
public:
	enum EConnectState { eOk, eNok, eMustRebind, eMustNotRebind, eRebindOk, eInitNok, eBindNok, eSetOptionsNok };

//	static int  LDAP_CALL LDAP_CALLBACK RebindProc( LDAP *ld, char **dnp, char **passwdp, int *authmethodp, int freeit, void *arg );

	//! create a new ldap session
	//! \param connectionParams an Anything that contains Server, Port,
	//!		   Timeout and MapUTF8 parameters
	//! If some of those slots are not present, the following defaults
	//! are taken: server=localhost, port=389, timeout=60, maputf8=true
	LDAPConnection(ROAnything connectionParams);
	virtual ~LDAPConnection();

	//! wait for result (with connection-specific timeout)
	//! returns false if error occurred or timeout
	//! \param msgId 	id of message to wait for
	//! \param result	returned result (not changed if not successful).
	//!                 all attributes in result will be normalized to lowercase!
	//! \param eh		error handler object
	virtual bool WaitForResult(int msgId, Anything &result, LDAPErrorHandler &eh);

	//! Release handle when using LDAPConnectionManager - this method does nothing here!
	virtual bool ReleaseHandleInfo();

	//! connect to server (init, bind). returns true, if connection succeeded.
	//! connection is automatically closed when this object dies.
	//! \param bindParams an Anything that contains BindName and BindPW
	//! If some of those slots are not present, the following defaults
	//! are taken: bindname="", bindpw="" (if bindname="", anonymous
	//! bind is attempted)
	bool Connect(ROAnything bindParams, LDAPErrorHandler eh);

	//! return connection handle
	LDAP *GetConnection();

	//! Dump Connection handle as hex string
	static String DumpConnectionHandle(LDAP *handle);

	//! return timeout for this session
	int GetSearchTimeout();

	//! return connection timeout for this session
	int GetConnectionTimeout();

protected:
	String		fServer;
	long		fPort;
	int			fConnectionTimeout;
	int			fSearchTimeout;
	LDAP		*fHandle;
	bool		fMapUTF8;

	//! bind (asynchronous)
	virtual bool Bind(String BindName, String BindPW, int &msgId, LDAPErrorHandler eh);

	//! does the Connect and reports details what it has done.
	virtual LDAPConnection::EConnectState DoConnect(ROAnything bindParams, LDAPErrorHandler eh);

	//! determine Connect() return code
	static bool IsConnectOk(LDAPConnection::EConnectState eConnectState);

	//! Translate Connect() returncodes to string literals
	static String ConnectRetToString(LDAPConnection::EConnectState eConnectState);

	//! disconnect from server (unbind)
	bool Disconnect();

	//! disconnect from server (unbind)
	static bool Disconnect(LDAP *handle);

	//! transforms ldapMessage (i.e. result) into an Anything
	//! ALL attribute names are normalized to lowercase!!
	void TransformResult(LDAPMessage *ldapResult, Anything &result, Anything qp);

	//! init connection
	LDAP *Init(LDAPErrorHandler eh);

	//! set ldap protocol
	bool SetProtocol(LDAPErrorHandler eh);

	//! set ldap timelimit for searches
	bool SetSearchTimeout(LDAPErrorHandler eh);

	//! set ldap connection timeout - important to avoid deadlocks caused by hanging ldap connects
	bool SetConnectionTimeout(LDAPErrorHandler eh);

private:

	//! set rebind procedure (in re-authenticates and when chasing referals)
//	bool SetRebindProc(LDAPErrorHandler eh);

	//! converts textual attribute values from UTF-8 to HTML format
	//! (LDAPv3 uses UTF-8). conversion is changes the passed string.
	void MapUTF8Chars(String &str);

	//! returns a human readable string describing the message type code
	String GetTypeStr(int msgType);

	//! Issue error message and abadon connection
	virtual void HandleWait4ResultError(int msgId, String &errMsg, LDAPErrorHandler eh);

	//! These test classes acesse private methods of LDAPConnection
	friend class LDAPConnectionManagerTest;
	friend class LDAPConnectionTest;
};

#endif
