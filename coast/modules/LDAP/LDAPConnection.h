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
//! <B>Not persitent	LDAP connection.</B>
//!
//!						Below values apply to LDAPConnections and PersistentLDAPConnections
///!/Server				IP or DNS name of the target server. Default is localhost.
//!	/Port				Port of the target ldap server. Default is 389
//!	/ConnnectionTimeout	How many seconds to wait until connection is established, abort when timeout is reached. Default is 10 sec.
//!						<b>Important:</b> connection pools are distinguished by Server/Port/DN/Password/ConnectionTimeout
//!						If you need different pools for the same Server/Port/DN/Password vary the ConnectionTimeout
//!						in order to create different pools.
//!	/Timeout			Timeout for the executed ldap op (search, compare), abort when timeout is reached. Default is 10 sec.
//!	/BindName			The distinguished name (dn) we bind with. If /BindName is not given or "anonymous" a anonymous bind
//!						takes place.
//!	/BindPW				The bind password
//!	/MapUTF8			Translate string results to html-escape sequences. Default is yes.
//!

class EXPORTDECL_LDAPDA LDAPConnection
{
public:
	enum EConnectState { eOk, eNok, eMustRebind, eMustNotRebind, eRebindOk, eInitNok, eBindNok, eSetOptionsNok };

	//! create a new ldap session
	//! \param connectionParams an Anything that contains Server, Port,
	//!		   Timeout and MapUTF8 parameters
	//! If some of those slots are not present, the following defaults
	//! are taken: server=localhost, port=389, timeout=60, maputf8=true
	LDAPConnection(ROAnything connectionParams);
	virtual ~LDAPConnection();

	/*! wait for result (with connection-specific timeout)
		\param msgId 	id of message to wait for
		\param result	returned result (not changed if not successful).
		                all attributes in result will be normalized to lowercase!
		\param eh		error handler object
		\return false if error occurred or timeout */
	bool WaitForResult(int msgId, Anything &result, LDAPErrorHandler &eh);

	//! Release handle when using LDAPConnectionManager
	bool ReleaseHandleInfo();

	//! connect to server (init, bind). returns true, if connection succeeded.
	//! connection is automatically closed when this object dies.
	//! \param bindParams an Anything that contains BindName and BindPW
	//! If some of those slots are not present, the following defaults
	//! are taken: bindname="", bindpw="" (if bindname="", anonymous
	//! bind is attempted)
	bool Connect(ROAnything bindParams, LDAPErrorHandler &eh);

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

	//! Release handle, eg. Disconnect
	virtual bool DoReleaseHandleInfo();

	//! bind (asynchronous)
	virtual bool Bind(String BindName, String BindPW, int &msgId, LDAPErrorHandler &eh);

	/*! handle the case of a bind failure, for this connection type we do not disconnect because this case is handled elsewhere
		\param eh error handling structure
		\param errMsg callers message to possibly print out */
	virtual void DoHandleBindFailure(LDAPErrorHandler &eh, String &errMsg);

	/*! handle the case of a timeout when waiting for a result, maybe try to rebind etc.
		\param eh error handling structure */
	virtual void DoHandleWaitForResultTimeout(LDAPErrorHandler &eh) {}

	/*! handle additional steps to to after a WaitForResult error has occured
		\param eh error handling structure */
	virtual void DoHandleWait4ResultError(LDAPErrorHandler &eh) {}

	//! does the Connect and reports details what it has done.
	virtual LDAPConnection::EConnectState DoConnect(ROAnything bindParams, LDAPErrorHandler &eh);

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
	LDAP *Init(LDAPErrorHandler &eh);

	//! set ldap protocol
	bool SetProtocol(LDAPErrorHandler &eh);

	//! set ldap timelimit for searches
	bool SetSearchTimeout(LDAPErrorHandler &eh);

	//! set ldap connection timeout - important to avoid deadlocks caused by hanging ldap connects
	bool SetConnectionTimeout(LDAPErrorHandler &eh);

private:
	//! converts textual attribute values from UTF-8 to HTML format
	//! (LDAPv3 uses UTF-8). conversion is changes the passed string.
	void MapUTF8Chars(String &str);

	//! returns a human readable string describing the message type code
	String GetTypeStr(int msgType);

	//! Issue error message and abadon connection
	void HandleWait4ResultError(int msgId, String &errMsg, LDAPErrorHandler &eh);

	//! These test classes acesse private methods of LDAPConnection
	friend class LDAPConnectionManagerTest;
	friend class LDAPConnectionTest;
};

#endif
