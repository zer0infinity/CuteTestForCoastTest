/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SSLModule_H
#define _SSLModule_H

#include "RequestListener.h"

class SSLSocket;

typedef struct ssl_ctx_st SSL_CTX;
typedef struct ssl_st SSL;
typedef struct x509_st X509;

class SSLModule: public WDModule {
	friend class SSLModuleTest;
public:
	enum MakeContextFor {
		eServer, eClient
	};
	SSLModule(const char *name);
	virtual bool Init(const ROAnything config);
	virtual bool Finis();

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

class SSLAcceptorFactory: public AcceptorFactory {
	SSLAcceptorFactory();
	SSLAcceptorFactory(const SSLAcceptorFactory &);
	SSLAcceptorFactory &operator=(const SSLAcceptorFactory &);
public:
	SSLAcceptorFactory(const char *SSLAcceptorFactoryName) :
		AcceptorFactory(SSLAcceptorFactoryName) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SSLAcceptorFactory(fName);
	}
	Acceptor *MakeAcceptor(AcceptorCallBack *ac);
};

#endif		//not defined _SSLModule_H
