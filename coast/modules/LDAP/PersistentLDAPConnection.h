/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PersistentLDAPConnection_H
#define _PersistentLDAPConnection_H

//---- baseclass include --------------------------------------------------
#include "config_LDAPDA.h"
#include "LDAPConnection.h"

//---- PersistentLDAPConnection ----------------------------------------------------------
//! <B>Persistent LDAP connections used by LDAPConnectonManger.</B>
/*!
Below values apply to LDAPConnections and PersistentLDAPConnections
\par Configuration
\code
{
	/Server				IP or DNS name of the target server. Default is localhost.
	/Port				Port of the target ldap server. Default is 389
	/ConnnectionTimeout	How many seconds to wait until connection is established, abort when timeout is reached. Default is 10 sec.
						<b>Important:</b> connection pools are distinguished by Server/Port/DN/Password/ConnectionTimeout
						If you need different pools for the same Server/Port/DN/Password vary the ConnectionTimeout
						in order to create different pools.
	/Timeout			Timeout for the executed ldap op (search, compare), abort when timeout is reached. Default is 10 sec.
	/BindName			The distinguished name (dn) we bind with. If /BindName is not given or "anonymous" a anonymous bind
						takes place.
	/BindPW				The bind password
	/MapUTF8			Translate string results to html-escape sequences. Default is yes.
}
\endcode
Below values apply to PersistentLDAPConnections only.
\code
{
	/PooledConnections  Use persistent ldap connections (creates PersistentLDAPConnection objects). Default is not to use pooled
						connections.
	/MaxConnections		Every tuple consisting of /Server /Port /ConnectionTimeout /BindName Md5Hash off /BindPW is considered as
						a connection pool. /MaxConnections defines the maximum number of allowed connections. Default is 5
						connections per pool.
	/TryAutoRebind		In the case the stored (persistent) ldap connection is no more valid (ldap server reboot, firewall "cuts"
						the open connection) the LDAPAbstractDAI attempts a "on the fly" rebind.
						Default is not to attempt "on the fly" rebinds.
	/RebindTimeout		If set to 0, this setting is ignored. Otherwise a connection is re-established after the /RebindTimeout
						second. Evaluation of this value takes place every time a LDAP operation on this connection is executed.
						Default is to ignore this setting.
}
\endcode
*/
class EXPORTDECL_LDAPDA PersistentLDAPConnection: public LDAPConnection
{
public:

	//! create a new persistent ldap connection
	/*! \param see above. connectionParams an Anything that contains Server, Port, Timeout and MapUTF8 parameters
		If some of those slots are not present, the following defaults
		are taken: server=localhost, port=389, timeout=60, maputf8=true */
	PersistentLDAPConnection(ROAnything connectionParams);
	~PersistentLDAPConnection();

	//! return max number of connections to use for connection type
	int GetMaxConnections();

	//! Called by THRKEYDELETE (LDAPConnectionManager)
	static void tsd_destruct(void *tsd);

protected:
	//! Release handle when using LDAPConnectionManager
	virtual bool DoReleaseHandleInfo();

	/*! handle the case of a bind failure, eg. disconnect
		\param eh error handling structure
		\param errMsg callers message to possibly print out */
	virtual void DoHandleBindFailure(LDAPErrorHandler &eh, String &errMsg);

	/*! handle the case of a timeout when waiting for a result, maybe try to rebind etc.
		\param eh error handling structure */
	virtual void DoHandleWaitForResultTimeout(LDAPErrorHandler &eh);

	/*! handle additional steps to to after a WaitForResult error has occured
		\param eh error handling structure */
	virtual void DoHandleWait4ResultError(LDAPErrorHandler &eh);

	//! does the Connect and reports details what it has done.
	LDAPConnection::EConnectState DoConnect(ROAnything bindParams, LDAPErrorHandler &eh);

private:
	//! Function to set up thread-specific data.
	static void tsd_setup();
	//! Function for setting an LDAP error.
	static void set_ld_error( int err, char *matched, char *errmsg, void *dummy );
	//! Function for getting an LDAP error.
	static int get_ld_error( char **matched, char **errmsg, void *dummy );
	//! Function for setting errno.
	static void set_errno( int err );
	//! Function for getting errno.
	static int get_errno( void);

	long 	fRebindTimeout;
	String	fPoolId;
	bool	fTryAutoRebind;
	int		fMaxConnections;
	int		fPooledConnections;

	/* Error structure. */
	struct ldap_error_struct {
		int  le_errno;
		char  *le_matched;
		char  *le_errmsg;
	};
	typedef struct ldap_error_struct ldap_error;

	//! Hook when using LDAPConnectionManager
	LDAPConnection::EConnectState DoConnectHook(ROAnything bindParams, LDAPErrorHandler &eh);

	//! Build the connection pool id used by LDAPConnectionManager
	String GetLdapPoolId(ROAnything bindParams);

	static String GetLdapPoolId(const String &server, long port, const String &bindName,
								const String &bindPW, long connectionTimeout);

	//! create a base64 armoured md5 hash
	static String Base64ArmouredMD5Hash(const String &text);

	//! set errno handler because the ldap lib does not know whether threads are used or not. By calling
	//! this wrapper functions to errno, at compile time the right (thread safe errno macro) is used.
	bool SetErrnoHandler(LDAPErrorHandler &eh);

	//! These test classes acesse private methods of PersistentLDAPConnection
	friend class LDAPConnectionManager;
	friend class LDAPConnectionManagerTest;
	friend class LDAPConnectionTest;
};

#endif
