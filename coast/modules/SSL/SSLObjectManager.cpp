/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SSLObjectManager.h"

//#define TRACE_LOCKS

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "TraceLocks.h"
#include "Threads.h"
#include "SystemLog.h"
#include "Dbg.h"
#include "Registry.h"
#include "SSLModule.h"
#include "SSLAPI.h"
#include "Resolver.h"

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

SSLObjectManager *SSLObjectManager::fgSSLObjectManager = 0;

SSLObjectManager *SSLObjectManager::SSLOBJMGR()
{
	fgSSLObjectManager = SafeCast(WDModule::FindWDModule("SSLObjectManager"), SSLObjectManager);
	return fgSSLObjectManager;
}

RegisterModule(SSLObjectManager);

namespace {
	const char storeIdDelim('@');
}
//---- SSLObjectManager ----------------------------------------------------------------
SSLObjectManager::SSLObjectManager(const char *name)
	: WDModule(name)
	, fSSLCtxStoreMutex("SSLCtxStoreMutex")
	, fSSLSessionIdStoreMutex("SSLSessionIdStoreMutex")
	, fSSLCtxStore(Anything::ArrayMarker(), Storage::Global())
	, fSSLSessionIdStore(Anything::ArrayMarker(), Storage::Global())
{
	StartTrace1(SSLObjectManager.SSLObjectManager, "Name:<" << NotNull(name) << ">");
	SystemLog::Info("SSLObjectManager: <unblocked>");
}

SSLObjectManager::~SSLObjectManager()
{
	StartTrace(SSLObjectManager.~SSLObjectManager);
	Finis();
}

SSL_CTX *SSLObjectManager::GetCtx(const String &ip, const String &port, ROAnything sslModuleCfg)
{
	StartTrace1(SSLObjectManager.GetCtx, "ip: " << ip << " port: " << port);
	String storeId(Resolver::DNS2IPAddress(ip, ip).Append(storeIdDelim).Append(port));
	Trace("storeId [" << storeId << "]");
	TRACE_LOCK_START("GetCtx");
	{
		Anything anySess;
		LockUnlockEntry me(fSSLCtxStoreMutex);
		if ( fSSLCtxStore.LookupPath(anySess, storeId, storeIdDelim) && anySess.AsIFAObject(0) != 0 ) {
			Trace("Found ssl context for id " << storeId);
			return reinterpret_cast<SSL_CTX*>(anySess.AsIFAObject(0));
		}
	}
	SSL_CTX *sslctx(0);
	if (!(sslModuleCfg.IsNull())) {
		TraceAny(sslModuleCfg, "sslModuleCfg");
		Trace("Creating ssl context [SSLModule with config] for id " << storeId);
		sslctx = SSLModule::GetSSLClientCtx(sslModuleCfg);
	} else {
		Trace("Creating ssl context [default SSLV23_client_method] for id " << storeId);
		sslctx = SSL_CTX_new(SSLv23_client_method());
	}
	if ( sslctx != 0 ) {
		LockUnlockEntry me(fSSLCtxStoreMutex);
		Anything anySess(reinterpret_cast<IFAObject*>(sslctx));
		SlotPutter::Operate(anySess, fSSLCtxStore, storeId, false, storeIdDelim);
		TraceAny(ROAnything(fSSLCtxStore), "SSL context store");
	}
	return sslctx;
}

bool SSLObjectManager::RemoveCtx(const String &ip, const String &port)
{
	StartTrace1(SSLObjectManager.RemoveCtx, "ip: " << ip << " port: " << port);
	String storeId(Resolver::DNS2IPAddress(ip, ip).Append(storeIdDelim).Append(port));
	Trace("storeId [" << storeId << "]");
	TRACE_LOCK_START("GetCtx");
	{
		Anything anySess;
		LockUnlockEntry me(fSSLCtxStoreMutex);
		if ( fSSLCtxStore.LookupPath(anySess, storeId, storeIdDelim) && anySess.AsIFAObject(0) != 0 ) {
			Trace("Found ssl context for id " << storeId);
			SSL_CTX_free(reinterpret_cast<SSL_CTX*>(anySess.AsIFAObject(0)));
			SlotCleaner::Operate(fSSLCtxStore, storeId, true, storeIdDelim);
			TraceAny(fSSLCtxStore, "session store after removal");
			return true;
		}
		TraceAny(fSSLCtxStore, "session store, no removal");
	}
	return false;
}

SSL_SESSION *SSLObjectManager::GetSessionId(const String &ip, const String &port)
{
	StartTrace1(SSLObjectManager.GetSessionId, "ip: " << ip << " port: " << port);
	String thrId;
	thrId.Append( Thread::MyId());
	String storeId(Resolver::DNS2IPAddress(ip, ip).Append(storeIdDelim).Append(port).Append(storeIdDelim).Append(thrId));
	Trace("storeId [" << storeId << "]");
	SSL_SESSION *sslsess(0);
	TRACE_LOCK_START("GetSessionId");
	{
		Anything anySess;
		LockUnlockEntry me(fSSLSessionIdStoreMutex);
		TraceAny(ROAnything(fSSLSessionIdStore), "SSL session store");
		if ( fSSLSessionIdStore.LookupPath(anySess, storeId, storeIdDelim) && anySess["session"].AsIFAObject(0) != 0 ) {
			sslsess=reinterpret_cast<SSL_SESSION *>(anySess["session"].AsIFAObject(0));
			Trace("Got SessionId for id " << storeId << " thread: " << thrId <<
			  " session: " << SessionIdAsHex(sslsess));
		}
	}
	return sslsess;
}

void SSLObjectManager::SetSessionId(const String &ip, const String &port, SSL_SESSION *sslSession)
{
	StartTrace1(SSLObjectManager.SetSessionId, "ip: " << ip << " port: " << port);
	String thrId;
	thrId.Append( Thread::MyId());
	String storeId(Resolver::DNS2IPAddress(ip, ip).Append(storeIdDelim).Append(port).Append(storeIdDelim).Append(thrId));
	Trace("storeId [" << storeId << "]");
	SSL_SESSION *sslSessionStored(0);
	TRACE_LOCK_START("SetSessionId");
	{
		LockUnlockEntry me(fSSLSessionIdStoreMutex);
		TraceAny(ROAnything(fSSLSessionIdStore), "SSL session store before");
		Anything anySess;
		// If there is already a session stored in this slot, free it
		if ( fSSLSessionIdStore.LookupPath(anySess, storeId, storeIdDelim) && anySess["session"].AsIFAObject(0) != 0 ) {
			sslSessionStored=reinterpret_cast<SSL_SESSION *>(anySess["session"].AsIFAObject(0));
			Trace("Freeing old SessionId: "  << SessionIdAsHex(sslSessionStored) << " RefCount: " << sslSessionStored->references);
			SSL_SESSION_free(sslSessionStored);
		}
		if ( sslSession != 0 ) {
			anySess["session"] = (IFAObject *) sslSession;
			anySess["length"] = static_cast<long>(sslSession->session_id_length);
			Trace("Storing SessionId: "  << SessionIdAsHex(sslSession) << " RefCount: " << sslSession->references);
			SlotPutter::Operate(anySess, fSSLSessionIdStore, storeId, false, storeIdDelim);
		} else {
			SlotCleaner::Operate(fSSLSessionIdStore, storeId, true, storeIdDelim);
		}
		TraceAny(ROAnything(fSSLSessionIdStore), "SSL session store after");
	}
	Trace("Set SessionId for id " << storeId << " thread: " << thrId <<
		  " session: " << SessionIdAsHex(sslSession));
}

bool SSLObjectManager::Init(const ROAnything config)
{
	StartTrace(SSLObjectManager.Init);
	SystemLog::WriteToStderr(String("\t") << fName << ". done\n");
	return ResetInit(config);
}

bool SSLObjectManager::Finis()
{
	StartTrace(SSLObjectManager.Finis);
	{
		TraceAny(fSSLCtxStore, "fSSLCtxStore");
		LockUnlockEntry me(fSSLCtxStoreMutex);
		for ( long ip = 0; ip < fSSLCtxStore.GetSize(); ip++) {
			for ( long port = 0; port < fSSLCtxStore[ip].GetSize(); port++) {
				Trace("Freeing ssl context for address: " << fSSLCtxStore.SlotName(ip) << " port: " << fSSLCtxStore[ip].SlotName(port));
				SSL_CTX *sslctx = (SSL_CTX *) fSSLCtxStore[ip][port].AsIFAObject(0);
				if (sslctx) {
					SSL_CTX_free(sslctx);
				}
			}
		}
		fSSLCtxStore = Anything(Anything::ArrayMarker(),fSSLCtxStore.GetAllocator());
	}
	{
		TraceAny(fSSLSessionIdStore, "fSSLSessionIdStore");
		LockUnlockEntry me(fSSLSessionIdStoreMutex);
		for (long ip = 0; ip < fSSLSessionIdStore.GetSize(); ip++) {
			for ( long port = 0; port < fSSLSessionIdStore[ip].GetSize(); port++) {
				for ( long thrId = 0; thrId < fSSLSessionIdStore[ip][port].GetSize(); thrId++) {
					if (fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0) != (IFAObject *) NULL) {
						Trace("Freeing ssl session for address: " << fSSLSessionIdStore.SlotName(ip) <<
							  " port: " << fSSLSessionIdStore[ip].SlotName(port) <<
							  " thread: " << fSSLSessionIdStore[ip][port].SlotName(thrId) <<
							  " session: " <<  SessionIdAsHex((SSL_SESSION *) fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0)));
						SSL_SESSION_free((SSL_SESSION *) fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0));
					}
				}
			}
		}
		fSSLSessionIdStore = Anything(Anything::ArrayMarker(),fSSLSessionIdStore.GetAllocator());
	}
	return true;
}

void SSLObjectManager::EmptySessionIdStore()
{
	StartTrace(SSLObjectManager.EmptySessionIdStore);
	fSSLSessionIdStore = Anything(Anything::ArrayMarker(),fSSLSessionIdStore.GetAllocator());
}

bool SSLObjectManager::ResetFinis(const ROAnything )
{
	StartTrace(SSLObjectManager.ResetFinis);
	return true;
}

bool SSLObjectManager::ResetInit(const ROAnything config)
{
	StartTrace(SSLObjectManager.ResetInit);
	return true;
}

void SSLObjectManager::EnterReInit()
{
	StartTrace(SSLObjectManager.EnterReInit);
}

void SSLObjectManager::LeaveReInit()
{
	StartTrace(SSLObjectManager.LeaveReInit);
}

String SSLObjectManager::SessionIdAsHex(SSL_SESSION *sslSession)
{
	if ((sslSession != (SSL_SESSION *) NULL) && (sslSession->session_id != (unsigned char *) NULL)) {
		String out;
		out.AppendAsHex((const unsigned char *) (void *)sslSession->session_id, sslSession->session_id_length);
		return out;
	} else {
		return String("SESSION ID IS NULL POINTER!!!");
	}
}

Anything SSLObjectManager::TraceSSLSession(SSL_SESSION *sslSession)
{
	Anything result;
	result["session_id"] = SSLObjectManager::SessionIdAsHex(sslSession);
	if (sslSession == (SSL_SESSION *) NULL) {
		return result;
	}
	if (sslSession->key_arg != (unsigned char *) NULL) {
		result["session_size"] = i2d_SSL_SESSION(sslSession, (unsigned char **) NULL);
	} else {
		result["key_arg"] = "NULL POINTER SESSIONVALUE";
	}
	if (sslSession->master_key != (unsigned char *) NULL) {
		String out;
		out.AppendAsHex((const unsigned char *) (void *)sslSession->master_key, sslSession->master_key_length);
		result["master_key"] = out;
	} else {
		result["master_key"] = "NULL POINTER SESSIONVALUE";
	}
	if (sslSession != (SSL_SESSION *) NULL) {
		result["not_resumable"] = sslSession->not_resumable;
		result["references"] =   sslSession->references;
		result["timeout"] = sslSession->timeout;
		result["time"] = sslSession->time;
	} else {
		result["not_resumable"] = "NULL POINTER SESSIONVALUE";
		result["references"] =   "NULL POINTER SESSIONVALUE";
		result["timeout"] = "NULL POINTER SESSIONVALUE";
		result["time"] = "NULL POINTER SESSIONVALUE";
	}
	if ((sslSession->cipher != (SSL_CIPHER *) NULL) && (sslSession->cipher->name != (char *) NULL)) {
		result["cipher"] = sslSession->cipher->name;
	} else {
		result["cipher"] = "NULL POINTER SESSIONVALUE";
	}
	return result;
}
