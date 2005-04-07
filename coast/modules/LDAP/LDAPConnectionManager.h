/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LDAPConnectionManager_H
#define _LDAPConnectionManager_H

//---- baseclass include -------------------------------------------------
#include "WDModule.h"
#include "Threads.h"
#include "Anything.h"
#include "ldap.h"

//---- LDAPConnectionManager ----------------------------------------------------------
//! Manages LDAP connections represented by binding handles.
//! If there is already a binding handle for a given key, LDAPConnectionManager performs
//! first a unbind.
//! If the RebindTimeout is expired, LDAPConnectionManager sets a corresponding flag.
//! This module does not need any configuration data.

class EXPORTDECL_WDBASE LDAPConnectionManager: public WDModule
{
public:
	//!it exists only one since it is a not cloned
	LDAPConnectionManager(const char *name);

	//!does nothing since everything should be done in Finis
	~LDAPConnectionManager();

	static LDAPConnectionManager *LDAPCONNMGR();

	//! Get a LdapConnection handle
	Anything GetLdapConnection(const String &uniqueConnectionId, long rebindTimeout = 0L);
	//! Set new LdapConnection handle - returns false a previously stored session with same key
	//! could not be released. You may safely ignore the return code, it is of interest in test cases.
	bool SetLdapConnection(const String &uniqueConnectionId, LDAP *handle);

	//--- module initialization termination ---
	//!initialize
	virtual bool Init(const Anything &config);
	//!finalize
	virtual bool Finis();
	//!terminate LDAPConnectionManager for reset
	virtual bool ResetFinis(const Anything &);
	//!reinitializes
	virtual bool ResetInit(const Anything &config);

	//!support reinit
	virtual void EnterReInit();
	//!support reinit
	virtual void LeaveReInit();

	friend class LDAPConnectionManagerTest;

protected:
	// Only used by LDAPConnectionManagerTest
	void EmptyLdapConnectionStore();
	bool RemoveCtx(const String &ip, const String &port);
	//!singleton cache
	static LDAPConnectionManager *fgLDAPConnectionManager;

	//!The rw lock that protects the ssl context store
	RWLock fLdapConnectionStoreRWLock;

	//!The Anything containing the SSL Context objects
	Anything fLdapConnectionStore;

private:

	LDAPConnectionManager(const LDAPConnectionManager &);
	LDAPConnectionManager &operator=(const LDAPConnectionManager &);
	// use careful, you inhibit subclass use
};

#endif
