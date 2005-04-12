/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//#define TRACE_LOCKS

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "TraceLocks.h"
#include "Threads.h"
#include "SysLog.h"
#include "Registry.h"
#include "TimeStamp.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "LDAPConnectionManager.h"
#include "LDAPConnection.h"

LDAPConnectionManager *LDAPConnectionManager::fgLDAPConnectionManager = 0;

LDAPConnectionManager *LDAPConnectionManager::LDAPCONNMGR()
{
	fgLDAPConnectionManager = SafeCast(WDModule::FindWDModule("LDAPConnectionManager"), LDAPConnectionManager);
	return fgLDAPConnectionManager;
}

RegisterModule(LDAPConnectionManager);
//---- LDAPConnectionManager ----------------------------------------------------------------
LDAPConnectionManager::LDAPConnectionManager(const char *name)
	: WDModule(name)
	, fLdapConnectionStoreRWLock("LdapConnectionRWLock")
	, fLdapConnectionStore(Storage::Global())
{
	StartTrace1(LDAPConnectionManager.LDAPConnectionManager, "Name:<" << NotNull(name) << ">");
}

LDAPConnectionManager::~LDAPConnectionManager()
{
	StartTrace(LDAPConnectionManager.~LDAPConnectionManager);
	Finis();
}

Anything LDAPConnectionManager::GetLdapConnection(const String &uniqueConnectionId, long rebindTimeout)
{
	StartTrace(LDAPConnectionManager.GetLdapConnection);
	Anything returned;
	TimeStamp ts;
	LDAP *handle;
	TRACE_LOCK_START("GetLdapConnection");
	{
		RWLockEntry me(fLdapConnectionStoreRWLock, true);
		me.Use();
		if ( fLdapConnectionStore.IsDefined(uniqueConnectionId) ) {
			handle = (LDAP *) fLdapConnectionStore[uniqueConnectionId]["Handle"].AsIFAObject(0);
			returned["LastRebind"] = fLdapConnectionStore[uniqueConnectionId]["LastRebind"].AsString();
		} else {
			handle = (LDAP *) NULL;
			returned["LastRebind"] = TimeStamp::Min().AsString();
		}
	}
	TimeStamp now = TimeStamp().Now();
	TimeStamp lastRebind(returned["LastRebind"].AsString());
	bool mustRebind =  ( ((lastRebind + rebindTimeout) <  now) );
	Trace("Get: " << uniqueConnectionId << " "  << LDAPConnection::DumpConnectionHandle(handle));
	Trace("Now: " << now << " LastRebind: " << lastRebind  << " RebindTimeout: "
		  << rebindTimeout << " Must rebind: " << mustRebind);
	returned["Handle"] 		= (IFAObject * )handle;
	returned["MustRebind"]	= mustRebind;
	TraceAny(returned, "Returning:");
	return returned;
}

bool LDAPConnectionManager::SetLdapConnection(const String &uniqueConnectionId, LDAP *handle)
{
	StartTrace(LDAPConnectionManager.SetLdapConnection);
	bool ret = true;
	TRACE_LOCK_START("SetLdapConnection");
	{
		RWLockEntry me(fLdapConnectionStoreRWLock, false);
		me.Use();
		// If there is already a connection stored, release it
		LDAP *storedHandle = (LDAP *) fLdapConnectionStore[uniqueConnectionId]["Handle"].AsIFAObject(0);
		if ( storedHandle != (LDAP *) NULL ) {
			Trace("Set: " << uniqueConnectionId << " Disconnecting storedHandle: " <<
				  LDAPConnection::DumpConnectionHandle((LDAP *) storedHandle));
			ret = LDAPConnection::Disconnect(storedHandle);
		}
		fLdapConnectionStore[uniqueConnectionId]["Handle"]  	= (IFAObject *) handle;
		fLdapConnectionStore[uniqueConnectionId]["LastRebind"]  = TimeStamp::Now().AsString();
		fLdapConnectionStore[uniqueConnectionId]["MustRebind"]  = 0;
	}
	Trace("Set: " << uniqueConnectionId << " "  <<
		  LDAPConnection::DumpConnectionHandle((LDAP *) fLdapConnectionStore[uniqueConnectionId]["Handle"].AsIFAObject(0)));
	return ret;
}

bool LDAPConnectionManager::Init(const Anything &config)
{
	StartTrace(LDAPConnectionManager.Init);
	SysLog::WriteToStderr(String("\t") << fName << ". done\n");
	return ResetInit(config);
}

bool LDAPConnectionManager::Finis()
{
	StartTrace(LDAPConnectionManager.Finis);
	{
		TraceAny(fLdapConnectionStore, "fLdapConnectionStore");
		RWLockEntry me(fLdapConnectionStoreRWLock, false);
		me.Use();
		for ( long items = 0; items < fLdapConnectionStore.GetSize(); items++ ) {
			LDAP *handle = (LDAP *) fLdapConnectionStore[items]["Handle"].AsIFAObject(0);
			Trace("Freeing LDAP connection for: " << fLdapConnectionStore.SlotName(items) << handle
				  << LDAPConnection::DumpConnectionHandle(handle));
			if (handle) {
				LDAPConnection::Disconnect(handle);
			}
		}
		fLdapConnectionStore = Anything(Storage::Global());
	}
	return true;
}

void LDAPConnectionManager::EmptyLdapConnectionStore()
{
	StartTrace(LDAPConnectionManager.EmptyLdapConnectionStore);
	fLdapConnectionStore = Anything(Storage::Global());
}

bool LDAPConnectionManager::ResetFinis(const Anything &)
{
	StartTrace(LDAPConnectionManager.ResetFinis);
	return true;
}

bool LDAPConnectionManager::ResetInit(const Anything &config)
{
	StartTrace(LDAPConnectionManager.ResetInit);
	return true;
}

void LDAPConnectionManager::EnterReInit()
{
	StartTrace(LDAPConnectionManager.EnterReInit);
}

void LDAPConnectionManager::LeaveReInit()
{
	StartTrace(LDAPConnectionManager.LeaveReInit);
}

