/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLModule_H
#define _SSLModule_H

#include "config_ssl.h"
#include "RequestListener.h"

//---- forward declarations ----
class SSLSocket;

//---- SSLModule -----------------------------------------------------------
class EXPORTDECL_SSL SSLModule : public WDModule
{
public:
	enum MakeContextFor {
		eServer,
		eClient
	};
	// Module methods
	SSLModule(const char *name);
	virtual ~SSLModule();
	virtual bool Init(const Anything &config);
	virtual bool Finis();

	// verification
	static SSL_CTX *GetSSLCtx(ConfNamedObject *object);
	static SSL_CTX *GetSSLClientCtx(ROAnything config);
protected:
	static SSL_CTX *SetOwnCertificateAndKey(SSL_CTX *ctx, LookupInterface *object, MakeContextFor eContextFor = SSLModule::eServer);
	static void SetSSLSetAcceptableClientCAs(SSL_CTX *ctx, LookupInterface *object);
	static void SetSSLCtxVerifyParameters(SSL_CTX *ctx, LookupInterface *object);
	static SSL_CTX *PrepareClientContext(LookupInterface *object);
	static SSL_CTX *PrepareServerContext(LookupInterface *object);
	static SSL_CTX *DoMakeServerContext(LookupInterface *object);

};

//---- SSLAcceptorFactory -----------------------------------------------------------
class EXPORTDECL_SSL SSLAcceptorFactory : public AcceptorFactory
{
public:
	SSLAcceptorFactory(const char *SSLAcceptorFactoryName);
	~SSLAcceptorFactory();

	//-- Cloning interface
	IFAObject *Clone() const {
		return new SSLAcceptorFactory(fName);
	}

	Acceptor *MakeAcceptor(AcceptorCallBack *ac);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	SSLAcceptorFactory();
	SSLAcceptorFactory(const SSLAcceptorFactory &);
	SSLAcceptorFactory &operator=(const SSLAcceptorFactory &);
};

#endif		//not defined _SSLModule_H
