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
#include "SystemLog.h"
#include "Dbg.h"
#include "Registry.h"
#include "SSLModule.h"
#include "SSLAPI.h"

//--- interface include --------------------------------------------------------
#include "SSLObjectManager.h"

SSLObjectManager *SSLObjectManager::fgSSLObjectManager = 0;

SSLObjectManager *SSLObjectManager::SSLOBJMGR()
{
	fgSSLObjectManager = SafeCast(WDModule::FindWDModule("SSLObjectManager"), SSLObjectManager);
	return fgSSLObjectManager;
}

RegisterModule(SSLObjectManager);
//---- SSLObjectManager ----------------------------------------------------------------
SSLObjectManager::SSLObjectManager(const char *name)
	: WDModule(name)
	, fSSLCtxStoreMutex("SSLCtxStoreMutex")
	, fSSLSessionIdStoreMutex("SSLSessionIdStoreMutex")
	, fSSLCtxStore(Storage::Global())
	, fSSLSessionIdStore(Storage::Global())
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
	StartTrace(SSLObjectManager.GetCtx);
	SSL_CTX *sslctx;
	TraceAny(sslModuleCfg, "sslModuleCfg");
	TRACE_LOCK_START("GetCtx");
	{
		LockUnlockEntry me(fSSLCtxStoreMutex);
		if ( ( sslctx = (SSL_CTX *) fSSLCtxStore[ip][port].AsIFAObject(0)) != (SSL_CTX *) NULL ) {
			Trace("Found ssl context for ip: " << ip << " port:  " << port);
			return sslctx;
		}
	}
	if (!(sslModuleCfg.IsNull())) {
		Trace("Creating ssl context [SSLModule with config] for ip: " << ip << " port:  " << port);
		{
			LockUnlockEntry me(fSSLCtxStoreMutex);
			sslctx = SSLModule::GetSSLClientCtx(sslModuleCfg);
			fSSLCtxStore[ip][port] = (IFAObject *) sslctx;
		}
	} else {
		Trace("Creating ssl context [default SSLV23_client_method] for ip: " << ip << " port:  " << port);
		{
			LockUnlockEntry me(fSSLCtxStoreMutex);
			sslctx = SSL_CTX_new(SSLv23_client_method());
			fSSLCtxStore[ip][port] = (IFAObject *) sslctx;
		}
	}
	return sslctx;
}

bool SSLObjectManager::RemoveCtx(const String &ip, const String &port)
{
	StartTrace(SSLObjectManager.RemoveCtx);
	SSL_CTX *sslctx;
	TRACE_LOCK_START("GetCtx");
	{
		LockUnlockEntry me(fSSLCtxStoreMutex);
		if ( ( sslctx = (SSL_CTX *) fSSLCtxStore[ip][port].AsIFAObject(0)) != (SSL_CTX *) NULL ) {
			Trace("Found ssl context for ip: " << ip << " port:  " << port);
			SSL_CTX_free(sslctx);
			fSSLCtxStore[ip][port] = (IFAObject *) NULL;
			return true;
		}
	}
	return false;
}

SSL_SESSION *SSLObjectManager::GetSessionId(const String &ip, const String &port)
{
	StartTrace(SSLObjectManager.GetSessionId);
	String thrId;
	thrId.Append( Thread::MyId());
	TRACE_LOCK_START("GetSessionId");
	{
		LockUnlockEntry me(fSSLSessionIdStoreMutex);

		Trace("Got SessionId for: " << ip << " port:  " << port <<
			  " thread: " << thrId <<
			  " session: " << SessionIdAsHex((SSL_SESSION *) fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0)));
		return (SSL_SESSION *) fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0);
	}
}

void SSLObjectManager::SetSessionId(const String &ip, const String &port, SSL_SESSION *sslSession)
{
	StartTrace(SSLObjectManager.SetSessionId);
	String thrId;
	thrId.Append( Thread::MyId());
	SSL_SESSION *sslSessionStored = NULL;
	TRACE_LOCK_START("SetSessionId");
	{
		LockUnlockEntry me(fSSLSessionIdStoreMutex);
		// If there is already a session stored in this slot, free it
		sslSessionStored = (SSL_SESSION *) fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0);
		if ( sslSessionStored != (SSL_SESSION *) NULL ) {
			Trace("Freeing old SessionId: "  << SessionIdAsHex(sslSessionStored) << " RefCount: " << sslSessionStored->references);
			SSL_SESSION_free(sslSessionStored);
			fSSLSessionIdStore[ip][port][thrId].Remove("length");
		}
		fSSLSessionIdStore[ip][port][thrId]["session"] = (IFAObject *) sslSession;
		fSSLSessionIdStore[ip][port][thrId]["length"] =
			(long) ((sslSession != (SSL_SESSION *)  NULL) ? sslSession->session_id_length : 0L);
	}
	Trace("Set SessionId for: " << ip << " port:  " << port << " thread: " << thrId <<
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
		fSSLCtxStore = Anything(Storage::Global());
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
							  " sessionId" <<  SessionIdAsHex((SSL_SESSION *) fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0)));
						SSL_SESSION_free((SSL_SESSION *) fSSLSessionIdStore[ip][port][thrId]["session"].AsIFAObject(0));
					}
				}
			}
		}
		fSSLSessionIdStore = Anything(Storage::Global());
	}
	return true;
}

void SSLObjectManager::EmptySessionIdStore()
{
	StartTrace(SSLObjectManager.EmptySessionIdStore);
	fSSLSessionIdStore = Anything(Storage::Global());
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
	if (sslSession == (SSL_SESSION *) NULL) {
		result["session_id"] = SSLObjectManager::SessionIdAsHex(sslSession);
		return result;
	}
	result["session_id"] = SSLObjectManager::SessionIdAsHex(sslSession);
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
