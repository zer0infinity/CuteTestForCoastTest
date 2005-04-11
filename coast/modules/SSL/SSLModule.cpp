/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SSLModule.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "System.h"
#include "SSLSocket.h"
#include "SSLAPI.h"
#include "DiffTimer.h"
#include "Dbg.h"
#include "ROAnyLookupAdapter.h"

//--- c-library modules used ---------------------------------------------------

static void thread_setup(void);
static void thread_cleanup(void);

//----- hack ------------------------------
static unsigned char dh512_p[] = {
	0xDA, 0x58, 0x3C, 0x16, 0xD9, 0x85, 0x22, 0x89, 0xD0, 0xE4, 0xAF, 0x75,
	0x6F, 0x4C, 0xCA, 0x92, 0xDD, 0x4B, 0xE5, 0x33, 0xB8, 0x04, 0xFB, 0x0F,
	0xED, 0x94, 0xEF, 0x9C, 0x8A, 0x44, 0x03, 0xED, 0x57, 0x46, 0x50, 0xD3,
	0x69, 0x99, 0xDB, 0x29, 0xD7, 0x76, 0x27, 0x6B, 0xA2, 0xD3, 0xD4, 0x12,
	0xE2, 0x18, 0xF4, 0xDD, 0x1E, 0x08, 0x4C, 0xF6, 0xD8, 0x00, 0x3E, 0x7C,
	0x47, 0x74, 0xE8, 0x33,
};
static unsigned char dh512_g[] = {
	0x02,
};

static DH *get_dh512()
{
	DH *dh = NULL;

	if ((dh = DH_new()) == NULL) {
		return(NULL);
	}
	dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), NULL);
	dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), NULL);
	if ((dh->p == NULL) || (dh->g == NULL)) {
		return(NULL);
	}
	return(dh);
}

// ------------------- SSLModule ---------------------------------------------
RegisterModule(SSLModule);

SSLModule::SSLModule(const char *name) : WDModule(name)
{
	SSL_load_error_strings();
	SSL_library_init();
#ifdef __sun
	// SOP: seed the random number generator on Solaris
	//     because it lacks /dev/random (up to Solaris 8)
	unsigned int randseed = (unsigned int)GetHRTIME();
	unsigned int SSLRandSeed[1024];
	for (size_t i = 0; i < sizeof(SSLRandSeed) / sizeof(unsigned int); i++) {
		SSLRandSeed[i] = rand_r(&randseed);
	}
	RAND_seed(&SSLRandSeed, sizeof(SSLRandSeed));
#endif
}

SSLModule::~SSLModule()
{
}
// provide locks for SSL lib using our own abstractions
RWLock **fgCryptoMutexes = 0;
long	fgNofCryptoMutexes = 0;
void sslLockingCallback(int mode, int n, const char *file, int line)
{
	Assert(n < fgNofCryptoMutexes);
	Assert(fgCryptoMutexes);
	if (fgCryptoMutexes && n < fgNofCryptoMutexes) {
		bool readlock = (mode & CRYPTO_READ) && ! (mode & CRYPTO_WRITE);
		if (mode & CRYPTO_LOCK) {
			fgCryptoMutexes[n]->Lock(readlock);
		} else {
			fgCryptoMutexes[n]->Unlock(readlock);
		}

	} else {
		Assert(false);
	}
}
static unsigned long	sslThreadIdCallback()
{
	return Thread::MyId();
}
static void cleanCryptoMutexes(long i)
{
	while (--i >= 0 && fgCryptoMutexes) {
		delete fgCryptoMutexes[i];
		fgCryptoMutexes[i] = 0;
	}
	delete [] fgCryptoMutexes;
	fgCryptoMutexes = 0;
	fgNofCryptoMutexes = 0;
}
static void thread_setup()
{
	StartTrace(SSLModule.thread_setup());
	bool ok = true;
	fgNofCryptoMutexes = CRYPTO_num_locks(); // assume this is always the same
	if (fgNofCryptoMutexes > 0 && !fgCryptoMutexes) {
		Trace("allocating CRYPTO locks: " << fgNofCryptoMutexes);
		Assert(!fgCryptoMutexes);
		fgCryptoMutexes = new RWLock*[fgNofCryptoMutexes];
		Assert(fgCryptoMutexes);
		ok = (fgCryptoMutexes != NULL);
		long i = 0;
		for (; i < fgNofCryptoMutexes && ok; i++) {
			fgCryptoMutexes[i] = 0;
			String name("SSLMutex");
			name.Append(i);
			fgCryptoMutexes[i] = new RWLock(name);
			ok = ok && (fgCryptoMutexes[i] != NULL);
			Trace("allocated " << name << (fgCryptoMutexes[i] != NULL) ? "ok" : "ERROR");
			Assert(fgCryptoMutexes[i]);
		}
		if (ok) {
			CRYPTO_set_locking_callback(sslLockingCallback);
			CRYPTO_set_id_callback(sslThreadIdCallback);
		} else {	// clean up again,
			cleanCryptoMutexes(i);
		}
	}
}

static void thread_cleanup()
{
	CRYPTO_set_locking_callback(0);
	cleanCryptoMutexes(fgNofCryptoMutexes);
	ERR_remove_state(0);
}
bool SSLModule::Init(const Anything &config)
{
	StartTrace(SSLModule.Init);
	thread_setup();
	SysLog::WriteToStderr(String("\t") << fName << ". done\n");
//	BIO *pbio = BIO_new(BIO_s_file());
//	BIO_set_fp(pbio,stderr,BIO_NOCLOSE);
//	CRYPTO_malloc_debug_init();
//	MemCheck_start();
//	MemCheck_on();
	return true;
}

bool SSLModule::Finis()
{
	thread_cleanup();
	return true;
}

SSL_CTX *SSLModule::GetSSLClientCtx(ROAnything config)
{
	StartTrace(SSLModule.GetSSLClientCtx);
	ROAnyLookupAdapter object(config);
	return PrepareClientContext(&object);
}

SSL_CTX *SSLModule::GetSSLCtx(ConfNamedObject *object)
{
	StartTrace(SSLModule.GetSSLCtx);
	String name;
	SSL_CTX *ctx = 0;

	Assert(object);
	if (object && object->GetName(name)) {
		Trace("Creating new Context...");
		ctx = DoMakeServerContext(object);
		if ( !ctx ) {
			String logMsg("Could not create SSL Context for: ");
			logMsg << name;
			Trace(logMsg);
			SysLog::Error(logMsg);
		}
	}
	return ctx;
}
SSL_CTX *SSLModule::PrepareServerContext(LookupInterface *object)
{
	StartTrace(SSLModule.PrepareServerContext);
	Trace("Creating SSL Context with method SSLv23");
	SSL_CTX *ctx = SSL_CTX_new (SSLv23_server_method());

	Assert(ctx); // we expect ssl context to be not null
	if (ctx) {
		int sslSessionCacheSize = (int)object->Lookup(
									  "SSLSessionCacheSize",
									  (long)SSL_SESSION_CACHE_MAX_SIZE_DEFAULT);     // default of openssl is 1024 * 20
		long sslSessionTimeout = object->Lookup(
									 "SSLSessionTimeout", (long)300); // default is 5 minutes
		String sslSessionIdContext(object->Lookup("SSLSessionIdContext", "wdapp"));
		sslSessionIdContext.Trim(SSL_MAX_SSL_SESSION_ID_LENGTH);

		// set some ssl options
		SSL_CTX_set_options(ctx, SSL_OP_ALL);
		SSL_CTX_set_quiet_shutdown(ctx, 1);
		SSL_CTX_sess_set_cache_size(ctx, sslSessionCacheSize);
		SSL_CTX_set_timeout(ctx, sslSessionTimeout);
		SSL_CTX_set_session_id_context(ctx, (const unsigned char *)(const char *)sslSessionIdContext, (unsigned int)sslSessionIdContext.Length());
		SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
	}
	return ctx;
}

SSL_CTX *SSLModule::PrepareClientContext(LookupInterface *object)
{
	StartTrace(SSLModule.PrepareClientContext);
	Trace("Creating SSL client Context with method SSLv23");
	SSL_CTX *ctx = SSL_CTX_new (SSLv23_client_method());

	Assert(ctx); // we expect ssl context to be not null
	int sslSessionCacheSize = (int)object->Lookup(
								  "SSLClientSessionCacheSize",
								  (long)SSL_SESSION_CACHE_MAX_SIZE_DEFAULT);     // default of openssl is 1024 * 20
	// By default, every 255th created session the cache is looked up for
	// expired sessions.
	long sslSessionTimeout = object->Lookup(
								 "SSLClientSessionTimeout", (long)300);		  // default is 5 minutes

	String sslSessionIdContext(object->Lookup("SSLClientSessionIdContext", "wdapp"));
	sslSessionIdContext.Trim(SSL_MAX_SSL_SESSION_ID_LENGTH);

	if (ctx) {
		// set some ssl options
		SSL_CTX_set_options(ctx, SSL_OP_ALL);
		SSL_CTX_set_quiet_shutdown(ctx, 1);
		SSL_CTX_sess_set_cache_size(ctx, sslSessionCacheSize);
		SSL_CTX_set_timeout(ctx, sslSessionTimeout);
		SSL_CTX_set_session_id_context(ctx, (const unsigned char *)(const char *)sslSessionIdContext, (unsigned int)sslSessionIdContext.Length());
		SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_CLIENT);
		if (object->Lookup("KeyFileClient", (const char *)0)) {
			ctx = SetOwnCertificateAndKey(ctx, object, SSLModule::eClient);
		}
		if (ctx) {
			SetSSLCtxVerifyParameters(ctx, object);
		}
	}
	return ctx;
}

SSL_CTX *SSLModule::SetOwnCertificateAndKey(SSL_CTX *ctx, LookupInterface *object, MakeContextFor eContextFor)
{
	StartTrace(SSLModule.SetOwnCertificateAndKey);
	if ( object->Lookup("NoCertAndPrivateKey", 0L) == 1L ) {
		SSL *ssl = SSL_new(ctx);
		Assert(ssl);
		return ctx;
	}
	String keyLookup, certLookup, keyDefault, certDefault;
	if ( eContextFor == SSLModule::eServer ) {
		keyLookup	= "KeyFileServer";
		keyDefault	= "serverkey.pem";
		certLookup	= "CertFileServer";
		certDefault	= "servercert.pem";
	}
	if ( eContextFor == SSLModule::eClient ) {
		keyLookup	= "KeyFileClient";
		keyDefault	= "clientkey.pem";
		certLookup	= "CertFileClient";
		certDefault	= "clientcert.pem";
	}
	String keyFileName = (char *)object->Lookup(
							 keyLookup, keyDefault);
	String certFileName = (char *)object->Lookup(
							  certLookup, certDefault );

	// Key must be loaded first, will be used in SSL_CTX_use_certificate_chain_file
	keyFileName =  System::GetFilePath(keyFileName, (const char *)0);
	Trace("Loading key file: " << keyFileName);
	long ret = SSL_CTX_use_PrivateKey_file (ctx, keyFileName,  SSL_FILETYPE_PEM );
	SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
	if ( ret != 1 ) {
		String logMsg("SSL PrivateKey File: ");
		logMsg << keyFileName << " not found";
		Trace(logMsg);
		SysLog::Error(logMsg);
		SSL_CTX_free(ctx);
		return 0;
	}
	certFileName = System::GetFilePath(certFileName, (const char *)0);
	Trace("Loading cert file: " << certFileName);
	// First certificate must be the server's cert. (Otherwise you get
	// a key verification error)
	// Append more certs in the path verification order. The ertificate the client
	// trusts is the last one in the chain. The cert the client trusts
	// may be omitted, but all other intermediate certs must build a complete
	// verifiation path.
	ret = SSL_CTX_use_certificate_chain_file (ctx, certFileName);
	SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
	if ( ret != 1 ) { // SOP according to openssl.org
		String logMsg("SSL Certificate File: ");
		logMsg << certFileName << " not found";
		Trace(logMsg);
		SysLog::Error(logMsg);
		SSL_CTX_free(ctx);
		return 0;
	}

	SSL *ssl = SSL_new(ctx);
	Assert(ssl);
	if ( ssl ) {
		X509 *x509 = SSL_get_certificate(ssl);
		Assert(x509);
		if ( x509 ) {
			EVP_PKEY_copy_parameters(X509_get_pubkey(x509),
									 SSL_get_privatekey(ssl));
		}
		SSL_free(ssl);
	}
	/* Now we know that a key and cert have been set against
	* the SSL context */
	if (1 != (ret = SSL_CTX_check_private_key(ctx))) {
		SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
		String logMsg("Private key does not match the certificate public key\n");
		Trace(logMsg);
		SysLog::Error(logMsg);
		SSL_CTX_free(ctx);
		return(0);
	}

	return ctx;
}
void SSLModule::SetSSLSetAcceptableClientCAs(SSL_CTX *ctx, LookupInterface *object)
{
	StartTrace(SSLModule.SetSSLSetAcceptableClientCAs);
	String peerCAFileName = (char *)object->Lookup(
								"SSLPeerCAFile", "" ); // used for verify location and client CA list
	String realpeerCAFileName =  System::GetFilePath(peerCAFileName, (const char *)0);
	if (peerCAFileName.Length()) {
		Trace("using client CA list from \n" << realpeerCAFileName << "\n");
		SSL_CTX_set_client_CA_list(ctx,
								   SSL_load_client_CA_file(realpeerCAFileName));
	}
}
void SSLModule::SetSSLCtxVerifyParameters(SSL_CTX *ctx, LookupInterface *object)
{
	StartTrace(SSLModule.SetSSLCtxVerifyParameters);
	long ret;
	String peerCAFileName = (char *)object->Lookup(
								"SSLPeerCAFile", "" ); // used for verify location and client CA list
	long sslVerifyPeerCert = object->Lookup(
								 "SSLVerifyPeerCert", (long)0); // default client

	long sslVerifyFailIfNoPeerCert = object->Lookup(
										 "SSLVerifyFailIfNoPeerCert", (long)0); // default client
	int sslVerifyMode =
		(sslVerifyPeerCert ? SSL_VERIFY_PEER : SSL_VERIFY_NONE)
		| (sslVerifyFailIfNoPeerCert ? SSL_VERIFY_FAIL_IF_NO_PEER_CERT : 0);
	String sslVerifyPath = object->Lookup(
							   "SSLVerifyPath", (const char *)0); // must be prepared with openssl's c_rehash if defined
	int sslVerifyDepth = object->Lookup(
							 "SSLVerifyDepth", (long)0); // default client CA verification depth
	Trace("SSLVerifyMode = " << (long)sslVerifyMode);
	Trace("SSLVerifyDepth = " << (long)sslVerifyDepth);
	SSL_CTX_set_verify_depth(ctx, sslVerifyDepth);

	long sslUseAppCallBack = object->Lookup(
								 "SSLUseAppCallback", (long)0);
	if (sslUseAppCallBack == 0L) {
		SSL_CTX_set_verify(ctx, sslVerifyMode, 0);
	} else {
		// Setting application specific SSL-callback, you may wish to overwrite the
		// callback provided in SSLSocket.
		SSL_CTX_set_verify(ctx, sslVerifyMode, SSLSocket::SSLVerifyCallback);
	}
	if (peerCAFileName.Length() || sslVerifyPath) {
		peerCAFileName = System::GetFilePath(peerCAFileName, (const char *)0);
		const char *pcafn = peerCAFileName.Length() ? (const char *)peerCAFileName : (const char *)0;
		if ( sslVerifyPath.Length() == 0L ) {
			sslVerifyPath = (const char *) NULL;
		}
		Trace("load SSL verify locations from: " <<  NotNull(pcafn) << "\n path=" << sslVerifyPath);
		if (1 != (ret = SSL_CTX_load_verify_locations(ctx, pcafn, sslVerifyPath))) {
			SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
			Trace("couldn't load verify locations from" << peerCAFileName << sslVerifyPath );
		}
	}
}
SSL_CTX *SSLModule::DoMakeServerContext(LookupInterface *object)
{
	StartTrace(SSLModule.DoMakeServerContext);
	Assert(object); // we expect object to be not null
	long ret;
	SSL_CTX *ctx = 0;

	if ( object ) { // we check it nevertheless for robustness
		ctx = PrepareServerContext(object);

		if ( ctx ) {

			DH *dh = get_dh512();
			SSL_CTX_set_tmp_dh(ctx, dh);
			DH_free(dh);

			ctx = SetOwnCertificateAndKey(ctx, object, SSLModule::eServer);
			if (ctx) {
				SetSSLCtxVerifyParameters(ctx, object);
				SetSSLSetAcceptableClientCAs(ctx, object);

//			SSL_CTX_set_tmp_rsa_callback(ctx,tmp_rsa_cb);

//			Trace("Setting ciphers: " << NotNull(ciphers));
//			if (ciphers) SSL_CTX_set_cipher_list(ctx,ciphers);

				Trace("Generating temp (512 bit) RSA key..." );

				RSA *rsa = RSA_generate_key(512, RSA_F4, NULL, NULL);
				Assert(rsa);
				if (1 != (ret = SSL_CTX_set_tmp_rsa(ctx, rsa))) {
					SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
					Trace("Generation failed");
				}
				Trace("Generation succeeded");
				RSA_free(rsa);
			}
		}
	}
	return ctx;
}

//---- SSLAcceptorFactory -----------------------------------------------------------
RegisterAcceptorFactory(SSLAcceptorFactory);

SSLAcceptorFactory::SSLAcceptorFactory(const char *SSLAcceptorFactoryName)
	: AcceptorFactory(SSLAcceptorFactoryName)
{
	StartTrace(SSLAcceptorFactory.Ctor);
}

SSLAcceptorFactory::~SSLAcceptorFactory()
{
	StartTrace(SSLAcceptorFactory.Dtor);
}

Acceptor *SSLAcceptorFactory::MakeAcceptor(AcceptorCallBack *ac)
{
	StartTrace(SSLAcceptorFactory.MakeAcceptor);

//	SSLModule *sslModule=(SSLModule*)WDModule::FindWDModule("SSLModule");
//	if(!sslModule) return 0;
//
	SSL_CTX *sslCtx = SSLModule::GetSSLCtx(this);
	if (!sslCtx) {
		return 0;
	}

	SSLSocketArgs sslSocketArgs(Lookup("VerifyCertifiedEntity", 0L) != 0L,
								Lookup("CertVerifyString", ""),
								Lookup("CertVerifyStringIsFilter", 0L) != 0L,
								false); // Server Sockets do session resumption based upon settings  in SSLModule,
	// using  openssl's  own session cache
	return new SSLAcceptor( sslCtx,
							Lookup("Address", (const char *)0),
							Lookup("Port", 80L),
							Lookup("Backlog", 50L),
							ac, sslSocketArgs);
}
