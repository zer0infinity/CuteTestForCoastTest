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
#include "config_LDAPDA.h"
#include "WDModule.h"
#include "Threads.h"
#include "Anything.h"
#include "ldap.h"

//---- LDAPConnectionManager ----------------------------------------------------------
//! <B>Manages LDAP connections represented by binding handles.</B>
//!	Because ldap connections remain open (there is no shutdown on the socket) an implementation
//! should allow to limit the usage of open connections. This avoids "out of filehandle" conditions.
//! If for some reason a stored filehandle becomes invalid there are two ways to handle this condition:
//!	- report error to calling application. The application will or will not handle the error.
//!	- do a "on the fly" retry. This implies doing a rebind and re-executing the given ldap operation.
//!	  The application has not to bother about re-issuing the ldap operation.
//!
//!	Design considerations:
//! Because ldap queries may be built dynamically (in the tmp store) the LDAPConnectionManager may not build up
//! the connection pool at boot strap, because it will not know the needed pools at that time. The pools have to be
//! built when they are needed for the first time. Every pool has a sempahore which limits the created connections to
//! the values given in /MaxConnections. Every connection is protected by a mutex. In the case of a "on the fly" retry,
//! the mutex remains locked until the retry has been done. The technical way to do this is to iterate over the mutexes
//! of a connection pool until IsLockedByMe() returns true. Then we have the index into the handles array which contains
//! the handle we need.
//! The whole pool structure is protected by as mutex. Note that a RWLock will not do the job, because accesses to
//! Anythings may trigger houskeeping such as allocating a new slot on indexed accessses, which is a writing access, even if
//! the caller thinks it is read only access.
//! On termination, all allocated structures are freed.
//! Structure: fLdapConnectionStore
//!					/poolid
//!					{
//!						/Sema	<ptr to sema>
//!						/Mutexes
//!						{
//!							<ptr to mutex>
//!							...
//!						}
//!						/HandleInfo
//!						{
//!							{
//!								/Handle 		<ldap connection handle>
//!								/MustRebind		<bool>
//!								/LastRebind		<TimeStamp>
//!							}
//!							....
//!						}
//!
//!
//! Parameters meaningful to LDAPConnectionManager
//!
//!	/PooledConnections  Use persistent ldap connections (creates PersistentLDAPConnection objects). Default is not to use pooled
//!						connections.
//!	/MaxConnections		Every tuple consisting of /Server /Port /ConnectionTimeout /BindName /BindPW is considered as
//!						a connection pool. /MaxConnections defines the maximum number of allowed connections. Default is 5
//!						connections per pool.
//!	/TryAutoRebind		In the case the stored (persistent) ldap connection is no more valid (ldap server reboot, firewall "cuts"
//!						the open connection) the LDAPAbstractDAI attempts a "on the fly" rebind.
//!						Default is not to attempt "on the fly" rebinds.
//!	/RebindTimeout		If set to 0, this setting is ignored. Otherwise a connection is re-established after the /RebindTimeout
//!						second. Evaluation of this value takes place every time a LDAP operation on this connection is executed.
//!						Default is to ignore this setting.

class EXPORTDECL_LDAPDA LDAPConnectionManager: public WDModule
{
public:
	//!it exists only one since it is a not cloned
	LDAPConnectionManager(const char *name);

	//!does nothing since everything should be done in Finis
	~LDAPConnectionManager();

	static LDAPConnectionManager *LDAPCONNMGR();

	//! Get a LdapConnection handle
	Anything GetLdapConnection(bool isLocked, long maxConnections, const String &poolId, long rebindTimeout = 0L);
	//! Set new LdapConnection handle - returns false a previously stored session with same key
	//! could not be released. You may safely ignore the return code, it is of interest in test cases.
	bool SetLdapConnection(long MaxConnections, const String &poolId, LDAP *handle);

	//! Release HandleInfo (releases mutex and semaphore)
	virtual bool ReleaseHandleInfo(long maxConnections, const String &poolId);

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

	//!singleton cache
	static LDAPConnectionManager *fgLDAPConnectionManager;

	//!The mutexthat protects the ldap connection pools structure
	Mutex fLdapConnectionStoreMutex;

	//!The connection pools structure
	Anything fLdapConnectionStore;

	//! The maximum number of connections per connection "type"
	long fDefMaxConnections;

	//! Get the semaphore, creat it if it does not exist
	Semaphore *LookupSema(long maxConnections, const String &poolId);

	//! Factored out common parts
	bool GetHandleInfo(long maxConnections, const String &poolId, Anything &handleInfo);

	//! Get a entry in a connection pool and lock the mutex. If maxConnections >= number of entries,
	//! a entry will be created if all other entries are locked.
	long GetAndLockSlot(long maxConnections, const String &poolId);

	//! Re-get a already locked slot. (Locked by the same thread)
	long ReGetLockedSlot(long maxConnections, const String &poolId);

	//! Re-get HandleInfo
	bool ReGetHandleInfo(long maxConnections, const String &poolId, Anything &handleInfo);

	//! Set HandleInfo
	bool SetHandleInfo(long maxConnections, const String &poolId, Anything &handleInfo);

	//! This method is for test purposes only. It is used to simulate invalid ldap connections and test
	//! the "on the fly" retries.
	bool ReplaceHandlesForConnectionPool(const String &poolId, LDAP *handle);

	//! Decide wether to do a rebind because a LDAPRebindTimeout was specified
	Anything HandleRebindTimoeut(Anything &returned, long rebindTimeout, LDAP *handle);

private:

	LDAPConnectionManager(const LDAPConnectionManager &);
	LDAPConnectionManager &operator=(const LDAPConnectionManager &);
	// use careful, you inhibit subclass use
};

#endif
