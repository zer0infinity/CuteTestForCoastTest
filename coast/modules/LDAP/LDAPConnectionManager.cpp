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
#include "PersistentLDAPConnection.h"

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
	, fLdapConnectionStoreMutex("LdapConnectionStoreMutex", Storage::Global())
	, fFreeListMutex("FreeListMutex", Storage::Global())
	, fLdapConnectionStore(Storage::Global())
	, fFreeList(Storage::Global())
	, fDefMaxConnections(0L)
{
	StartTrace1(LDAPConnectionManager.LDAPConnectionManager, "Name:<" << NotNull(name) << ">");
}

LDAPConnectionManager::~LDAPConnectionManager()
{
	StartTrace(LDAPConnectionManager.~LDAPConnectionManager);
	Finis();
}

Anything LDAPConnectionManager::GetLdapConnection(bool isLocked, long maxConnections, const String &poolId, long rebindTimeout)
{
	StartTrace(LDAPConnectionManager.GetLdapConnection);
	Anything returned;
	TimeStamp ts;
	LDAP *handle;
	Anything handleInfo;
	Trace("maxConnections: " << maxConnections << " fDefMaxConnections: " << fDefMaxConnections);
	maxConnections = (maxConnections != 0L) ? maxConnections :  fDefMaxConnections;

	bool ret;
	if ( !isLocked ) {
		Semaphore *sema = LookupSema(maxConnections, poolId);
		TraceAny(fLdapConnectionStore[poolId], "Contents for: " << poolId);
		if ( sema != ( Semaphore * ) NULL ) {
			// we aquire the sema. If no handles are available, we block until one is ready
			Trace("Acquiring sema for: " << poolId);
			sema->Acquire();
		} else {
			String msg;
			msg << "Retrieving semaphore poolId: [" << poolId << "]failed!";
			SysLog::Info(msg);
			return returned;
		}
		ret = GetHandleInfo(maxConnections, poolId, handleInfo);
	}
	// We already hold the lock (we are in "on the fly" retry)
	else {
		ret = ReGetHandleInfo(maxConnections, poolId, handleInfo);
	}
	if ( ret ) {
		handle = (LDAP *) handleInfo["Handle"].AsIFAObject(0);
		returned["LastRebind"] = handleInfo["LastRebind"].AsString();
	} else {
		handle = (LDAP *) NULL;
		returned["LastRebind"] = TimeStamp::Min().AsString();
	}
	Trace("Get: " << poolId << " "  << PersistentLDAPConnection::DumpConnectionHandle(handle) << " Ret: " << ret);
	return HandleRebindTimoeut(returned, rebindTimeout, handle);
}

Anything LDAPConnectionManager::HandleRebindTimoeut(Anything &returned, long rebindTimeout, LDAP *handle)
{
	StartTrace(LDAPConnectionManager.HandleRebindTimoeut);
	TimeStamp now = TimeStamp().Now();
	TimeStamp lastRebind(returned["LastRebind"].AsString());
	bool mustRebind;
	String msg;
	if ( handle == (LDAP *) NULL ) {
		msg << "Rebind because connection handle is [NULL]";
		SysLog::Info(msg);
		Trace(msg);
		mustRebind = true;
	} else {
		if ( rebindTimeout != 0L ) {
			mustRebind =  ( ((lastRebind + rebindTimeout) <=  now) );
			if ( mustRebind ) {
				msg << "Now: " << now << " LastRebind: " << lastRebind  << " RebindTimeout: " <<
					rebindTimeout << " Must rebind: " << mustRebind;
				SysLog::Info(msg);
				Trace(msg);
			}
		} else {
			mustRebind = false;
		}
	}
	returned["Handle"] 		= (IFAObject * )handle;
	returned["MustRebind"]	= mustRebind;
	TraceAny(returned, "Returning:");
	return returned;
}

bool LDAPConnectionManager::SetLdapConnection(long maxConnections, const String &poolId, LDAP *handle)
{
	StartTrace(LDAPConnectionManager.SetLdapConnection);
	// Lock is already set!
	Trace("maxConnections: " << maxConnections << " fDefMaxConnections: " << fDefMaxConnections);
	maxConnections = (maxConnections != 0L) ? maxConnections :  fDefMaxConnections;
	Anything handleInfo;

	// If there is  an old connection stored, release it
	bool ret = ReGetHandleInfo(maxConnections, poolId, handleInfo);
	if ( ret ) {
		LDAP *storedHandle = (LDAP *) handleInfo["Handle"].AsIFAObject(0);
		if ( storedHandle != (LDAP *) NULL ) {
			Trace("Set: " << poolId << " Disconnecting storedHandle: " <<
				  PersistentLDAPConnection::DumpConnectionHandle((LDAP *) storedHandle));
			ret = PersistentLDAPConnection::Disconnect(storedHandle);
		}
		Trace("handleInfo type: " << handleInfo.GetType());
		handleInfo["Handle"]  	= (IFAObject *) handle;
		handleInfo["LastRebind"]  = TimeStamp::Now().AsString();
		handleInfo["MustRebind"]  = 0;
		SetHandleInfo(maxConnections, poolId, handleInfo);
	}
	Trace("Set: " << poolId << " "  <<
		  PersistentLDAPConnection::DumpConnectionHandle((LDAP *) handleInfo["Handle"].AsIFAObject(0))
		  << " Ret: " << ret);
	return ret;
}

Semaphore *LDAPConnectionManager::LookupSema(long maxConnections, const String &poolId)
{
	StartTrace(LDAPConnectionManager.LookupSema);
	Semaphore *sema;
	MutexEntry me(fLdapConnectionStoreMutex);
	me.Use();
	{
		if ( !(fLdapConnectionStore.IsDefined(poolId)) ) {
			// We are the first ones
			// semaphore is initialized to the desired amount of connections
			Trace("Creating sema for: " << poolId << " with maxConnections: " << maxConnections);
			sema = new Semaphore(maxConnections);
			fLdapConnectionStore[poolId]["Sema"] = (IFAObject *) sema;
		}
	}
	return (Semaphore * ) fLdapConnectionStore[poolId]["Sema"].AsIFAObject(0);
}

bool LDAPConnectionManager::GetHandleInfo(long maxConnections, const String &poolId, Anything &handleInfo)
{
	StartTrace(LDAPConnectionManager.GetHandleInfo);
	// We look for a unlocked handle and set the mutex.
	long slotIndex = GetAndLockSlot(maxConnections, poolId);
	bool ret = (slotIndex != -1L);
	if ( ret ) {
		MutexEntry me(fLdapConnectionStoreMutex);
		me.Use();
		{
			handleInfo = fLdapConnectionStore[poolId]["HandleInfo"][slotIndex];
		}
	}
	Trace("Entry for: " << poolId << " found at slotIndex: " << slotIndex << " Returning: " << ret);
	return ret;
}

long LDAPConnectionManager::GetUnusedFreeListEntry(long maxConnections, const String &poolId)
{
	StartTrace(LDAPConnectionManager.GetAndLockSlot);
	// Because we use a counting semaphore, we know that we should get one unlocked entry
	MutexEntry me(fFreeListMutex);
	me.Use();
	{
		for (long l = 0 ; l < maxConnections; l++ ) {
			if ( fFreeList[poolId][l].AsLong(-1L) == -1L ) {
				fFreeList[poolId][l] = Thread::MyId();
				Trace("Found freelist entry: [" << poolId << "] at index: [" << l << "] thread: [" << myId << "]");
				return l;
			}
		}
	}
	// We should never end up here
	Trace("NO free entry: [" << poolId << "] We should not end up here !!!!!");
	return -1L;
}

long LDAPConnectionManager::GetThisThreadsFreeListEntry(long maxConnections, const String &poolId)
{
	StartTrace(LDAPConnectionManager.GetAndLockSlot);
	// Because we use a counting semaphore, we know that we should get one unlocked entry
	long myId = Thread::MyId();
	MutexEntry me(fFreeListMutex);
	me.Use();
	{
		for (long l = 0 ; l < maxConnections; l++ ) {
			if ( fFreeList[poolId][l].AsLong(-1L) == myId ) {
				Trace("Found freelist entry: [" << poolId << "] at index: [" << l << "] thread: [" << myId << "]");
				return l;
			}
		}
	}
	// We should never end up here
	Trace("NO free entry: [" << poolId << "] We should not end up here !!!!!");
	return -1L;
}

long LDAPConnectionManager::GetAndLockSlot(long maxConnections, const String &poolId)
{
	StartTrace(LDAPConnectionManager.GetAndLockSlot);
	// Because we use a counting semaphore, we know that we should get one unlocked entry
	Mutex *mutex = (Mutex *) NULL;
	long l = GetUnusedFreeListEntry(maxConnections, poolId);
	if ( l != -1L ) {
		MutexEntry me(fLdapConnectionStoreMutex);
		me.Use();
		{
			mutex = (Mutex * ) fLdapConnectionStore[poolId]["Mutexes"][l].AsIFAObject(0);
			if ( mutex != (Mutex * ) NULL ) {
				TRACE_LOCK_START("GetAndLockSlot");
				if ( mutex->TryLock() ) {
					Trace("Found handle AND locking mutex  for: [" << poolId << "] at index: [" << l << "]");
					return l;
				}
			} else {
				String name;
				name << "Mutex_" << poolId << "_Index_" << l;
				// We are the first ones and must create the mutex on the heap
				mutex = new Mutex(name, Storage::Global());
				if ( mutex != ( Mutex *) NULL ) {
					fLdapConnectionStore[poolId]["Mutexes"][l] = (IFAObject *) mutex;
					TRACE_LOCK_START("GetAndLockSlot");
					Trace("Locking mutex for: " << poolId << " at index: " << l);
					mutex->Lock();
					return l;
				}
			}
		}
	}
	// We should never end up here
	Trace("NO handle for: [" << poolId << "] at index: [" << l << "]");
	return -1L;
}

bool LDAPConnectionManager::ReleaseHandleInfo(long maxConnections, const String &poolId)
{
	StartTrace(LDAPConnectionManager.ReleaseHandleInfo);
	Trace("maxConnections: " << maxConnections << " fDefMaxConnections: " << fDefMaxConnections);
	maxConnections = (maxConnections != 0L) ? maxConnections :  fDefMaxConnections;
	long slotIndex = GetThisThreadsFreeListEntry(maxConnections, poolId);
	bool ret = ( slotIndex != -1L );
	if ( ret ) {
		Semaphore *sema = (Semaphore *) NULL;
		MutexEntry me1(fLdapConnectionStoreMutex);
		me1.Use();
		{
			Mutex *mutex = (Mutex * ) fLdapConnectionStore[poolId]["Mutexes"][slotIndex].AsIFAObject(0);
			if ( mutex != (Mutex *) NULL ) {
				Trace("Unlocking mutex for: " << poolId << " at index: " << slotIndex);
				mutex->Unlock();
			}
			sema = (Semaphore * ) fLdapConnectionStore[poolId]["Sema"].AsIFAObject(0);
		}
		MutexEntry me2(fFreeListMutex);
		me2.Use();
		{
			fFreeList[poolId][slotIndex] = -1L;
			Trace("Releasing freelist entry: [" << poolId << "] at index: [" << slotIndex << "]");
			// Sema must be released at the end of all housekeeping to be done
			if ( sema != ( Semaphore *) NULL ) {
				sema->Release();
				String msg;
				msg << "Releasing LDAP connection at entry: [" << slotIndex << "] for connection pool: " << poolId;
				Trace(msg);
//				SysLog::Info(msg);
			}
		}
	}
	return ret;
}

bool LDAPConnectionManager::ReGetHandleInfo(long maxConnections, const String &poolId, Anything &handleInfo)
{
	StartTrace(LDAPConnectionManager.ReGetHandleInfo);
	long slotIndex = ReGetLockedSlot(maxConnections, poolId);
	bool ret = ( slotIndex != -1L);
	if ( ret ) {
		MutexEntry me(fLdapConnectionStoreMutex);
		me.Use();
		{
			// We already hold the lock for this handle, safe access now
			handleInfo = fLdapConnectionStore[poolId]["HandleInfo"][slotIndex];
		}
	}
	Trace("Entry for: " << poolId << " found at slotIndex: " << slotIndex << " Returning: " << ret);
	return ret;
}

bool LDAPConnectionManager::SetHandleInfo(long maxConnections, const String &poolId, Anything &handleInfo)
{
	StartTrace(LDAPConnectionManager.ReGetHandleInfo);
	long slotIndex = ReGetLockedSlot(maxConnections, poolId);
	bool ret = ( slotIndex != -1L);
	if ( ret ) {
		MutexEntry me(fLdapConnectionStoreMutex);
		me.Use();
		{
			// We already hold the lock for this handle, safe access now
			fLdapConnectionStore[poolId]["HandleInfo"][slotIndex] = handleInfo;
		}
	}
	Trace("Entry for: " << poolId << " found at slotIndex: " << slotIndex << " Returning: " << ret);
	return ret;
}

long LDAPConnectionManager::ReGetLockedSlot(long maxConnections, const String &poolId)
{
	StartTrace(LDAPConnectionManager.ReGetLockedSlot);
	// Because we use a counting semaphore, we know that there will be at least one unlocked entry, (MaxCount - semaphore count)
	long l = GetThisThreadsFreeListEntry(maxConnections, poolId);
	if ( l != -1L ) {
		// Was mutex locked by the same thread (us)?
		Mutex *mutex = (Mutex *) NULL;
		mutex = (Mutex * ) fLdapConnectionStore[poolId]["Mutexes"][l].AsIFAObject(0);
		Assert(mutex != (Mutex *) NULL);
		Assert(mutex->IsLockedByMe());
		Trace("Found handle for: [" << poolId << "] at index: [" << l << "]");
		return l;
	}
	String msg;
	msg << "ReGetLockedSlot failed for poolId: [" << poolId << "]failed!";
	SysLog::Info(msg);
	Trace(msg);
	return -1L;
}

bool LDAPConnectionManager::Init(const Anything &config)
{
	StartTrace(LDAPConnectionManager.Init);
	TraceAny(config, "Config: ");
	ROAnything moduleConfig;
	((ROAnything)config).LookupPath(moduleConfig, "LDAPConnectionManager");
	TraceAny(moduleConfig, "moduleConfig");
	fDefMaxConnections = moduleConfig["DefMaxConnections"].AsLong(2L);
	Trace("fDefMaxConnections: " << fDefMaxConnections);
	SysLog::WriteToStderr(String("\t") << fName << " Default is [" << fDefMaxConnections << "] connections per pool\n");
	SysLog::WriteToStderr(String("\t") << fName << ". done\n");
	return ResetInit(config);
}

bool LDAPConnectionManager::Finis()
{
	StartTrace(LDAPConnectionManager.Finis);
	bool ret = true;
	{
		TraceAny(fLdapConnectionStore, "fLdapConnectionStore");
		MutexEntry me(fLdapConnectionStoreMutex);
		me.Use();
		{
			for ( long pools = 0; pools < fLdapConnectionStore.GetSize(); pools++ ) {
				for ( long items = 0; items < fLdapConnectionStore[pools]["Mutexes"].GetSize(); items++ ) {
					SysLog::Info(String("LDAPConnectionManager: At index: [") << items   << "]");
					Mutex *mutex = (Mutex * ) fLdapConnectionStore[pools]["Mutexes"][items].AsIFAObject(0);
					if ( mutex != ( Mutex * ) NULL ) {
						bool locked = mutex->TryLock();
						if ( locked ) {
							LDAP *handle = (LDAP *) fLdapConnectionStore[pools]["HandleInfo"][items]["Handle"].AsIFAObject(0);
							SysLog::Info(String("LDAPConnectionManager: Pool: [") << fLdapConnectionStore.SlotName(pools) << "] " <<
										 "At index: [" << items   << "] " <<
										 "LDAPConnectionManager: Freeing LDAP " <<
										 PersistentLDAPConnection::DumpConnectionHandle(handle) << "\n");
							if (handle) {
								PersistentLDAPConnection::Disconnect(handle);
							}
							mutex->Unlock();
							delete mutex;
						}
					} else {
						SysLog::Info(String("LDAPConnectionManager: Pool: [") << fLdapConnectionStore.SlotName(pools) << "] " <<
									 "At index: [" << items   << "] " <<
									 "LDAPConnectionManager: TryLock mutex and deleting of mutex failed!");
						ret = false;
					}
				}
				Semaphore *sema = (Semaphore * ) fLdapConnectionStore[pools]["Sema"].AsIFAObject(0);
				if ( sema != ( Semaphore * ) NULL ) {
					delete sema;
				}
			}
		}
		fLdapConnectionStore = Anything(Storage::Global());
	}
	return ret;
}

void LDAPConnectionManager::EmptyLdapConnectionStore()
{
	StartTrace(LDAPConnectionManager.EmptyLdapConnectionStore);
	fLdapConnectionStore = Anything(Storage::Global());
}

bool LDAPConnectionManager::ResetFinis(const Anything &)
{
	StartTrace(LDAPConnectionManager.ResetFinis);
	// Do not allow to reset this module!
	return true;
}

bool LDAPConnectionManager::ResetInit(const Anything &config)
{
	StartTrace(LDAPConnectionManager.ResetInit);
	// Do not allow to reset this module!
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

bool LDAPConnectionManager::ReplaceHandlesForConnectionPool(const String &poolId, LDAP *handle)
{
	StartTrace(LDAPConnectionManager.ReplaceHandlesForConnectionPool);
	bool ret = true;
	Trace("Working on pool: [" << poolId << "]");
	{
		TraceAny(fLdapConnectionStore, "fLdapConnectionStore");
		MutexEntry me(fLdapConnectionStoreMutex);
		me.Use();
		{
			for ( long items = 0; items < fLdapConnectionStore[poolId]["Mutexes"].GetSize(); items++ ) {
				Mutex *mutex = (Mutex * ) fLdapConnectionStore[poolId]["Mutexes"][items].AsIFAObject(0);
				Trace("Trying to get lock for: " << poolId << " at index: " <<  items);
				if ( mutex != ( Mutex * ) NULL ) {
					if ( mutex->TryLock() ) {
						LDAP *oldHandle = (LDAP *) fLdapConnectionStore[poolId]["HandleInfo"][items]["Handle"].AsIFAObject(0);
						if (oldHandle) {
							PersistentLDAPConnection::Disconnect(oldHandle);
						}
						Trace("Replaced " << LDAPConnection::DumpConnectionHandle(oldHandle) << " New " <<   LDAPConnection::DumpConnectionHandle(handle));
						fLdapConnectionStore[poolId]["HandleInfo"][items]["Handle"] = (IFAObject *) handle;
						mutex->Unlock();
					}
				} else {
					Trace("Locking mutex for connection pool: " << poolId << " at index: " <<  items << " failed!");
					ret = false;
				}
			}
		}
	}
	return ret;
}

