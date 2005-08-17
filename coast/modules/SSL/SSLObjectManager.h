/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLObjectManager_H
#define _SSLObjectManager_H

//---- baseclass include -------------------------------------------------
#include "config_ssl.h"
#include "WDModule.h"
#include "Threads.h"
#include "SSLModule.h"
#include "SSLAPI.h"

//---- forward declaration -----------------------------------------------
class RWLock;

//---- SSLObjectManager ----------------------------------------------------------
//! Manages SSL_CTX and SSL_SESSIONS. SSL_CTX creation involves reading cert files,
//! private keys and certificate verification chains/pathes from the file system.
//! These i/o operations are too expensive to be executed on every SLL connect.
//! SSL_SESSIONS need to pass the ssl protocol defined handshake phase
//! (see client hello, server hello), which - depending  on what ciphers you
//! (or the openssl lib) chooses - involves more or less expensive private/public key
//! calculations. Session resumptions by-passes the handshake relying on the fact
//! that a session stored away in  order to be resumed was checked  once  when it
//! was stored away. Nevertheless, opennssl will check  if a stored  away sessions
//! has timed out.
class EXPORTDECL_SSL SSLObjectManager: public WDModule
{
public:
	//!it exists only one since it is a not cloned
	SSLObjectManager(const char *name);

	//!does nothing since everything should be done in Finis
	~SSLObjectManager();

	static SSLObjectManager *SSLOBJMGR();

	//--- module initialization termination ---
	//!initialize
	virtual bool Init(const Anything &config);
	//!finalize
	virtual bool Finis();
	//!terminate SSLObjectManager for reset
	virtual bool ResetFinis(const Anything &);
	//!reinitializes
	virtual bool ResetInit(const Anything &config);

	//!support reinit
	virtual void EnterReInit();
	//!support reinit
	virtual void LeaveReInit();

	//!Get SSL_CTX, if not registered, create SSL_CTX
	SSL_CTX *GetCtx(const String &ip, const String &port, ROAnything sslModuleCfg);

	//!Get the ssl session id
	SSL_SESSION *GetSessionId(const String &ip, const String &port);
	void SetSessionId(const String &ip, const String &port, SSL_SESSION *sslSession);
	static String SessionIdAsHex(SSL_SESSION *sslSession);
	static Anything SSLObjectManager::TraceSSLSession(SSL_SESSION *sslSession);

	friend class SSLObjectManagerTest;

protected:
	// Only used by SSLObjectManagerTest
	void EmptySessionIdStore();
	bool RemoveCtx(const String &ip, const String &port);
	//!singleton cache
	static SSLObjectManager *fgSSLObjectManager;

	//!The rw lock that protects the ssl context store
	Mutex fSSLCtxStoreMutex;

	//!The rw lock that protects the ssl sessionId store
	Mutex fSSLSessionIdStoreMutex;

	//!The Anything containing the SSL Context objects
	Anything fSSLCtxStore;

	//!The Anything containing the SSL sessionId strigs
	Anything fSSLSessionIdStore;
private:

	SSLObjectManager(const SSLObjectManager &);
	SSLObjectManager &operator=(const SSLObjectManager &);
	// use careful, you inhibit subclass use
};

#endif

