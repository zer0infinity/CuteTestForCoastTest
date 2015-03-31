/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SSLModule.h"
#include "SystemLog.h"
#include "SystemFile.h"
#include "SSLSocket.h"
#include "AnyLookupInterfaceAdapter.h"
#if defined(sun) && OS_RELMAJOR == 5 && OS_RELMINOR <= 8
#include "DiffTimer.h"
#endif
using namespace coast;

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
	DH *dh = 0;

	if ((dh = DH_new()) == 0) {
		return(0);
	}
	dh->p = BN_bin2bn(dh512_p, sizeof(dh512_p), 0);
	dh->g = BN_bin2bn(dh512_g, sizeof(dh512_g), 0);
	if ((dh->p == 0) || (dh->g == 0)) {
		return(0);
	}
	return(dh);
}

// Generate with: dhparam -C -noout 1024
static unsigned char dh1024_p[] = {
	0xA6, 0xD9, 0x24, 0xC3, 0x0C, 0x6A, 0x2E, 0x84, 0x6C, 0xEC, 0x6A, 0x54,
	0xEE, 0xE1, 0x28, 0x25, 0x20, 0xA6, 0x63, 0xD4, 0x2E, 0xC8, 0x2B, 0x99,
	0x48, 0x1B, 0xB1, 0x98, 0xA4, 0x2B, 0x6F, 0xA7, 0x23, 0x00, 0x4C, 0xAD,
	0x25, 0x57, 0xDC, 0x9B, 0x00, 0xB7, 0x8A, 0x7E, 0xB3, 0x83, 0xF3, 0xAB,
	0x56, 0xF9, 0xF7, 0x29, 0x9F, 0x48, 0x88, 0xF7, 0xBC, 0x86, 0xD8, 0xCA,
	0x4C, 0xD1, 0x13, 0xC5, 0x66, 0xE2, 0xC4, 0x50, 0xB2, 0x0E, 0xE8, 0xCE,
	0xAC, 0x5C, 0x0D, 0xF9, 0x10, 0xA8, 0x34, 0x91, 0x2C, 0x39, 0xDA, 0x6E,
	0xEB, 0xFB, 0xCC, 0x4E, 0x18, 0x4B, 0xAB, 0x74, 0x33, 0x8E, 0xA5, 0x1E,
	0x16, 0x8A, 0xFE, 0x73, 0xE4, 0xEC, 0xEE, 0x44, 0x5D, 0xAD, 0x2F, 0xEF,
	0x32, 0xFE, 0x5F, 0x45, 0x55, 0xD2, 0x57, 0xEE, 0x0B, 0x73, 0x92, 0xE3,
	0x57, 0x31, 0x88, 0xF5, 0x9C, 0x26, 0x1C, 0x53,
};
static unsigned char dh1024_g[] = {
	0x02,
};

DH *get_dh1024()
{
	DH *dh;

	if ((dh = DH_new()) == 0) {
		return(0);
	}
	dh->p = BN_bin2bn(dh1024_p, sizeof(dh1024_p), 0);
	dh->g = BN_bin2bn(dh1024_g, sizeof(dh1024_g), 0);
	if ((dh->p == 0) || (dh->g == 0)) {
		DH_free(dh);
		return(0);
	}
	return(dh);
}

// Generate with: dhparam -C -noout 2048
static unsigned char dh2048_p[] = {
	0xD1, 0x77, 0xFA, 0x96, 0x41, 0x88, 0x35, 0x34, 0x93, 0x70, 0xD8, 0x2B,
	0x93, 0x82, 0x35, 0xC6, 0xA2, 0x2D, 0x45, 0x2B, 0x99, 0x03, 0x4F, 0x71,
	0xE9, 0xD5, 0xF7, 0xE0, 0xEF, 0x83, 0x1B, 0x2F, 0xCF, 0x14, 0xBD, 0xCA,
	0x36, 0x8C, 0xD4, 0x23, 0xD1, 0x24, 0xFF, 0x0C, 0x0B, 0x00, 0x21, 0xF0,
	0xE0, 0xD9, 0x51, 0xE7, 0x37, 0xB8, 0xA4, 0x2C, 0x47, 0x50, 0x5E, 0x91,
	0x92, 0xD4, 0xBA, 0x45, 0xFB, 0xEE, 0xB1, 0x0C, 0x2D, 0x39, 0x6C, 0x82,
	0x91, 0x26, 0x43, 0xD4, 0xF8, 0x93, 0xFE, 0xE0, 0x20, 0x49, 0xC0, 0xDC,
	0xF3, 0x19, 0xDD, 0xA2, 0x1D, 0x85, 0x28, 0x06, 0x33, 0x97, 0x97, 0xD6,
	0x0D, 0x8B, 0xEB, 0xBC, 0x89, 0x26, 0x87, 0x4A, 0x93, 0x38, 0x03, 0x10,
	0x98, 0x09, 0x36, 0x72, 0xA6, 0x51, 0xD5, 0x24, 0x17, 0x11, 0x24, 0xA6,
	0xDE, 0xB8, 0x90, 0x67, 0x13, 0x4E, 0x1E, 0x2E, 0x55, 0x22, 0xEB, 0xF2,
	0x0F, 0x49, 0xE0, 0x6C, 0x60, 0xEA, 0xBD, 0x4D, 0x02, 0x88, 0x3B, 0x8C,
	0xB7, 0x93, 0x22, 0x0B, 0x52, 0x87, 0xCE, 0xC9, 0xCC, 0x80, 0x67, 0xB2,
	0xB8, 0x1A, 0xDE, 0x66, 0xF2, 0x41, 0xFE, 0xF7, 0xAA, 0x07, 0x32, 0x63,
	0xB0, 0xB8, 0xDD, 0x1A, 0xF7, 0x6E, 0x14, 0x01, 0xE1, 0xEC, 0xD0, 0x98,
	0x4E, 0x33, 0x16, 0xE4, 0x8F, 0x97, 0x30, 0x86, 0xE1, 0x66, 0xEA, 0x33,
	0xD0, 0x76, 0x8A, 0x45, 0x89, 0xD0, 0xB0, 0x89, 0x3D, 0x56, 0x3F, 0x93,
	0x86, 0x3F, 0x84, 0x38, 0x9C, 0x9D, 0xC5, 0x82, 0x9D, 0xFE, 0x1D, 0x5C,
	0x9F, 0x2E, 0x22, 0xC2, 0xA8, 0x72, 0x16, 0xF5, 0xBA, 0x54, 0x8C, 0x92,
	0x24, 0x02, 0x67, 0xFA, 0x55, 0xCE, 0xB4, 0x7F, 0xA8, 0x27, 0xC6, 0x48,
	0xA6, 0x1B, 0x76, 0x27, 0xCF, 0xAA, 0xEC, 0xDE, 0xF0, 0x96, 0x72, 0x3A,
	0xC7, 0xE4, 0x61, 0x0B,
};
static unsigned char dh2048_g[] = {
	0x02,
};

DH *get_dh2048()
{
	DH *dh = 0;

	if ((dh = DH_new()) == 0) {
		return(0);
	}
	dh->p = BN_bin2bn(dh2048_p, sizeof(dh2048_p), 0);
	dh->g = BN_bin2bn(dh2048_g, sizeof(dh2048_g), 0);
	if ((dh->p == 0) || (dh->g == 0)) {
		DH_free(dh);
		return(0);
	}
	return(dh);
}

RegisterModule(SSLModule);

SSLModule::SSLModule(const char *name) :
	WDModule(name) {
	StartTrace(SSLModule.SSLModule);
	SSL_library_init();
	SSL_load_error_strings();
	ERR_load_ERR_strings();
	ERR_load_crypto_strings();

#if defined(sun) && OS_RELMAJOR == 5 && OS_RELMINOR <= 8
	// SOP: seed the random number generator on Solaris because it lacks /dev/random (up to Solaris 8)
	unsigned int randseed = (unsigned int)GetHRTIME();
	unsigned int SSLRandSeed[1024];
	for (size_t i = 0; i < sizeof(SSLRandSeed) / sizeof(unsigned int); i++) {
		SSLRandSeed[i] = rand_r(&randseed);
	}
	RAND_seed(&SSLRandSeed, sizeof(SSLRandSeed));
#endif
}

// provide locks for SSL lib using our own abstractions
RWLock **fgCryptoMutexes = 0;
long fgNofCryptoMutexes = 0;
void sslLockingCallback(int mode, int n, const char *file, int line) {
	Assert(n < fgNofCryptoMutexes);Assert(fgCryptoMutexes);
	if (fgCryptoMutexes && n < fgNofCryptoMutexes) {
		RWLock::eLockMode lockMode = (((mode & CRYPTO_READ) && !(mode & CRYPTO_WRITE)) ? RWLock::eReading : RWLock::eWriting);
		if (mode & CRYPTO_LOCK) {
			fgCryptoMutexes[n]->Lock(lockMode);
		} else {
			fgCryptoMutexes[n]->Unlock(lockMode);
		}

	} else {
		Assert(false);
	}
}
static unsigned long sslThreadIdCallback() {
	return Thread::MyId();
}
static void cleanCryptoMutexes(long i) {
	while (--i >= 0 && fgCryptoMutexes) {
		delete fgCryptoMutexes[i];
		fgCryptoMutexes[i] = 0;
	}
	delete[] fgCryptoMutexes;
	fgCryptoMutexes = 0;
	fgNofCryptoMutexes = 0;
}
static void thread_setup() {
	StartTrace(SSLModule.thread_setup());
	bool ok = true;
	fgNofCryptoMutexes = CRYPTO_num_locks(); // assume this is always the same
	if (fgNofCryptoMutexes > 0 && !fgCryptoMutexes) {
		Trace("allocating CRYPTO locks: " << fgNofCryptoMutexes);Assert(!fgCryptoMutexes);
		fgCryptoMutexes = new RWLock*[fgNofCryptoMutexes];
		Assert(fgCryptoMutexes);
		ok = (fgCryptoMutexes != 0);
		long i = 0;
		for (; i < fgNofCryptoMutexes && ok; i++) {
			fgCryptoMutexes[i] = 0;
			String name("SSLMutex");
			name.Append(i);
			fgCryptoMutexes[i] = new RWLock(name);
			ok = ok && (fgCryptoMutexes[i] != 0);
			Trace("allocated " << name << (fgCryptoMutexes[i] != 0) ? "ok" : "ERROR");Assert(fgCryptoMutexes[i]);
		}
		if (ok) {
			CRYPTO_set_locking_callback(sslLockingCallback);
			CRYPTO_set_id_callback(sslThreadIdCallback);
		} else { // clean up again,
			cleanCryptoMutexes(i);
		}
	}
}

static void thread_cleanup() {
	CRYPTO_set_locking_callback(0);
	cleanCryptoMutexes(fgNofCryptoMutexes);
	ERR_remove_state(0);
}
bool SSLModule::Init(const ROAnything config) {
	StartTrace(SSLModule.Init);
	thread_setup();
	SystemLog::WriteToStderr(String("\t") << fName << ". done\n");
	//	BIO *pbio = BIO_new(BIO_s_file());
	//	BIO_set_fp(pbio,stderr,BIO_NOCLOSE);
	//	CRYPTO_malloc_debug_init();
	//	MemCheck_start();
	//	MemCheck_on();
	return true;
}

bool SSLModule::Finis() {
	thread_cleanup();
	return true;
}

SSL_CTX *SSLModule::GetSSLClientCtx(ROAnything config) {
	StartTrace(SSLModule.GetSSLClientCtx);
	AnyLookupInterfaceAdapter<ROAnything> object(config);
	return PrepareClientContext(&object);
}

SSL_CTX *SSLModule::GetSSLCtx(ConfNamedObject *object) {
	StartTrace(SSLModule.GetSSLCtx);
	String name;
	SSL_CTX *ctx = 0;

	Assert(object);
	if (object && object->GetName(name)) {
		Trace("Creating new SSL-Server-Context for " << name);
		ctx = DoMakeServerContext(object);
		if (!ctx) {
			String logMsg("Could not create SSL Context for: ");
			logMsg << name;
			Trace(logMsg);
			SystemLog::Error(logMsg);
		}
	}
	return ctx;
}

SSL_CTX *SSLModule::SetCipherList(SSL_CTX *ctx, LookupInterface *object, String lookupName, String defaultValue)
{
	StartTrace(SSLModule.SetCipherList);
	// Here we load our cihper lists
	String cipherList = object->Lookup(lookupName, defaultValue);
	Trace("Lookup: [" << lookupName << "] Setting cipherList: [" << cipherList << "]." );
	String logMsg;
	if (1 != (SSL_CTX_set_cipher_list(ctx, cipherList))) {
		logMsg << "Could not load cipher list: [" << cipherList << "] for config: [" << lookupName << "]";
		SystemLog::Error(logMsg);
	} else {
		logMsg << "Loaded cipher list: [" << cipherList << "] for config: [" << lookupName << "]\n";
		SystemLog::Info(logMsg);
	}
	return ctx;
}

SSL_CTX *SSLModule::SetRSAKeyLength(SSL_CTX *ctx, LookupInterface *object, String lookupName, long defaultLength)
{
	StartTrace(SSLModule.SetRSAKeyLength);

	long rsaKeyLength = object->Lookup(lookupName, defaultLength);
	// Don't allow 0 byte length
	if ( rsaKeyLength == 0L ) {
		rsaKeyLength = defaultLength;
	}
	Trace("Generating temp [" << rsaKeyLength << "] bit RSA key.");
	String logMsg;
	logMsg << "Generating RSA key with length: [" << rsaKeyLength << "] for config: [" << lookupName << "]\n";
	SystemLog::Info(logMsg);
	logMsg.Trim(0L);
	// RSA_F4 -> Exponent 17
	RSA *rsa = RSA_generate_key(rsaKeyLength, RSA_F4, 0, 0);
	Assert(rsa);
	long ret = 0;
	if (1 != (ret = SSL_CTX_set_tmp_rsa(ctx, rsa))) {
		SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
		logMsg << "Could not generate tmp RSA key for length: [" << rsaKeyLength << "]";
		SystemLog::Error(logMsg);
	}
	Trace("Generation succeeded");
	RSA_free(rsa);
	return ctx;
}

SSL_CTX *SSLModule::SetDiffieHellmannKey(SSL_CTX *ctx, LookupInterface *object, String lookupName, long defaultLength)
{
	StartTrace(SSLModule.SetDiffieHellmannKey);
	long dhMethod = object->Lookup(lookupName, defaultLength);
	DH *dh = 0;
	String logMsg;
	logMsg << "Generating DH key with length: [" << dhMethod << "] for config: [" << lookupName << "]\n";
	SystemLog::Info(logMsg);
	if (dhMethod == 512) {
		dh = get_dh512();
	} else {
		dh = get_dh1024();
	}
	// Ephemeral key exchange initialization, inherited by all SSLCtx
	// It is a bad idea to generate the keys on the fly
	long ret = 0;
	if (1 != (ret = SSL_CTX_set_tmp_dh(ctx, dh))) {
		SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
	  logMsg.Trim(0L);
		logMsg << "Could not generate DH ephemeral key for length: [" << dhMethod << "]";
		SystemLog::Error(logMsg);
	}
	DH_free(dh);
	return ctx;
}

SSL_CTX *SSLModule::PrepareServerContext(LookupInterface *object)
{
	StartTrace(SSLModule.PrepareServerContext);
	Trace("Creating SSL Context with method SSLv23");
	SSL_CTX *ctx = SSL_CTX_new(SSLv23_server_method());

	Assert(ctx); // we expect ssl context to be not null
	if (ctx) {
		int sslSessionCacheSize = (int) object->Lookup("SSLSessionCacheSize", (long) SSL_SESSION_CACHE_MAX_SIZE_DEFAULT); // default of openssl is 1024 * 20
		long sslSessionTimeout = object->Lookup("SSLSessionTimeout", (long) 300); // default is 5 minutes
		String sslSessionIdContext(object->Lookup("SSLSessionIdContext", "coastd"));
		sslSessionIdContext.Trim(SSL_MAX_SSL_SESSION_ID_LENGTH);

		// set some ssl options
		SSL_CTX_set_options(ctx, SSL_OP_ALL);
		SSL_CTX_set_quiet_shutdown(ctx, 1);
		SSL_CTX_sess_set_cache_size(ctx, sslSessionCacheSize);
		SSL_CTX_set_timeout(ctx, sslSessionTimeout);
		SSL_CTX_set_session_id_context(ctx, (const unsigned char *) (const char *) sslSessionIdContext,
				(unsigned int) sslSessionIdContext.Length());
		SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_SERVER);
	}
	return ctx;
}

SSL_CTX *SSLModule::PrepareClientContext(LookupInterface *object) {
	StartTrace(SSLModule.PrepareClientContext);
	Trace("Creating SSL client Context with method SSLv23");
	SSL_CTX *ctx = SSL_CTX_new (SSLv23_client_method());
	Assert(ctx); // we expect ssl context to be not null
	SetCipherList(ctx, object, "ClientCipherList", "HIGH:-SSLv2:+SSLv3");
	SetRSAKeyLength(ctx, object, "ClientRSALength", 2048L);
	SetDiffieHellmannKey(ctx, object, "ClientDHLength", 1024L);
	int sslSessionCacheSize = (int)object->Lookup(
								  "SSLClientSessionCacheSize",
								  (long)SSL_SESSION_CACHE_MAX_SIZE_DEFAULT);     // default of openssl is 1024 * 20
	// By default, every 255th created session the cache is looked up for
	// expired sessions.
	long sslSessionTimeout = object->Lookup("SSLClientSessionTimeout", (long) 300); // default is 5 minutes

	String sslSessionIdContext(object->Lookup("SSLClientSessionIdContext", "coastd"));
	sslSessionIdContext.Trim(SSL_MAX_SSL_SESSION_ID_LENGTH);

	if (ctx) {
		// set some ssl options
		SSL_CTX_set_options(ctx, SSL_OP_ALL);
		SSL_CTX_set_quiet_shutdown(ctx, 1);
		SSL_CTX_sess_set_cache_size(ctx, sslSessionCacheSize);
		SSL_CTX_set_timeout(ctx, sslSessionTimeout);
		SSL_CTX_set_session_id_context(ctx, (const unsigned char *) (const char *) sslSessionIdContext,
				(unsigned int) sslSessionIdContext.Length());
		SSL_CTX_set_session_cache_mode(ctx, SSL_SESS_CACHE_CLIENT);
		if (object->Lookup("KeyFileClient", (const char *) 0)) {
			ctx = SetOwnCertificateAndKey(ctx, object, SSLModule::eClient);
		}
		if (ctx) {
			SetSSLCtxVerifyParameters(ctx, object);
		}
	}
	return ctx;
}

SSL_CTX *SSLModule::SetOwnCertificateAndKey(SSL_CTX *ctx, LookupInterface *object, MakeContextFor eContextFor) {
	StartTrace(SSLModule.SetOwnCertificateAndKey);
	if (object->Lookup("NoCertAndPrivateKey", 0L) == 1L) {
		SSL *ssl = SSL_new(ctx);
		(void) ssl;
		Assert(ssl);
		return ctx;
	}
	String keyLookup, certLookup, keyDefault, certDefault;
	if (eContextFor == SSLModule::eServer) {
		keyLookup = "KeyFileServer";
		keyDefault = "serverkey.pem";
		certLookup = "CertFileServer";
		certDefault = "servercert.pem";
	}
	if (eContextFor == SSLModule::eClient) {
		keyLookup = "KeyFileClient";
		keyDefault = "clientkey.pem";
		certLookup = "CertFileClient";
		certDefault = "clientcert.pem";
	}
	String keyFileName = (char *) object->Lookup(keyLookup, keyDefault);
	String certFileName = (char *) object->Lookup(certLookup, certDefault);

	// Key must be loaded first, will be used in SSL_CTX_use_certificate_chain_file
	keyFileName = system::GetFilePath(keyFileName, (const char *) 0);
	Trace("Loading key file: " << keyFileName);
	long ret = SSL_CTX_use_PrivateKey_file(ctx, keyFileName, SSL_FILETYPE_PEM);
	SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
	if (ret != 1) {
		String logMsg("SSL PrivateKey File: ");
		logMsg << keyFileName << " not found";
		Trace(logMsg);
		SystemLog::Error(logMsg);
		SSL_CTX_free(ctx);
		return 0;
	}
	certFileName = system::GetFilePath(certFileName, (const char *) 0);
	Trace("Loading cert file: " << certFileName);
	// First certificate must be the server's cert. (Otherwise you get
	// a key verification error)
	// Append more certs in the path verification order. The ertificate the client
	// trusts is the last one in the chain. The cert the client trusts
	// may be omitted, but all other intermediate certs must build a complete
	// verifiation path.
	ret = SSL_CTX_use_certificate_chain_file(ctx, certFileName);
	SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
	if (ret != 1) { // SOP according to openssl.org
		String logMsg("SSL Certificate File: ");
		logMsg << certFileName << " not found";
		Trace(logMsg);
		SystemLog::Error(logMsg);
		SSL_CTX_free(ctx);
		return 0;
	}

	SSL *ssl = SSL_new(ctx);
	Assert(ssl);
	if (ssl) {
		X509 *x509 = SSL_get_certificate(ssl);
		Assert(x509);
		if (x509) {
			EVP_PKEY_copy_parameters(X509_get_pubkey(x509), SSL_get_privatekey(ssl));
		}
		SSL_free(ssl);
	}
	/* Now we know that a key and cert have been set against
	 * the SSL context */
	if (1 != (ret = SSL_CTX_check_private_key(ctx))) {
		SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
		String logMsg("Private key does not match the certificate public key\n");
		Trace(logMsg);
		SystemLog::Error(logMsg);
		SSL_CTX_free(ctx);
		return (0);
	}

	return ctx;
}
void SSLModule::SetSSLSetAcceptableClientCAs(SSL_CTX *ctx, LookupInterface *object) {
	StartTrace(SSLModule.SetSSLSetAcceptableClientCAs);
	String peerCAFileName(object->Lookup("SSLPeerCAFile", "")); // used for verify location and client CA list
	if (peerCAFileName.Length()) {
		peerCAFileName = system::GetFilePath(peerCAFileName, "");
	}
	if (peerCAFileName.Length()) {
		Trace("using client CA list from \n" << peerCAFileName << "\n");
		SSL_CTX_set_client_CA_list(ctx, SSL_load_client_CA_file(peerCAFileName));
	}
}
void SSLModule::SetSSLCtxVerifyParameters(SSL_CTX *ctx, LookupInterface *object) {
	StartTrace(SSLModule.SetSSLCtxVerifyParameters);
	long ret;
	String peerCAFileName(object->Lookup("SSLPeerCAFile", "")); // used for verify location and client CA list
	if (peerCAFileName.Length()) {
		peerCAFileName = system::GetFilePath(peerCAFileName, "");
	}
	long sslVerifyPeerCert = object->Lookup("SSLVerifyPeerCert", (long) 0); // default client

	long sslVerifyFailIfNoPeerCert = object->Lookup("SSLVerifyFailIfNoPeerCert", (long) 0); // default client
	int sslVerifyMode = (sslVerifyPeerCert ? SSL_VERIFY_PEER : SSL_VERIFY_NONE)
			| (sslVerifyFailIfNoPeerCert ? SSL_VERIFY_FAIL_IF_NO_PEER_CERT : 0);
	String sslVerifyPath(object->Lookup("SSLVerifyPath", "")); // must be prepared with openssl's c_rehash if defined
	int sslVerifyDepth = object->Lookup("SSLVerifyDepth", (long) 0); // default client CA verification depth
	Trace("SSLVerifyMode = " << (long)sslVerifyMode);
	Trace("SSLVerifyDepth = " << (long)sslVerifyDepth);
	SSL_CTX_set_verify_depth(ctx, sslVerifyDepth);

	long sslUseAppCallBack = object->Lookup("SSLUseAppCallback", (long) 0);
	if (sslUseAppCallBack == 0L) {
		SSL_CTX_set_verify(ctx, sslVerifyMode, 0);
	} else {
		// Setting application specific SSL-callback, you may wish to overwrite the
		// callback provided in SSLSocket.
		SSL_CTX_set_verify(ctx, sslVerifyMode, SSLSocket::SSLVerifyCallback);
	}
	if (peerCAFileName.Length() || sslVerifyPath.Length()) {
		const char *pcafn = peerCAFileName.Length() ? (const char *) peerCAFileName : (const char *) 0;
		const char *pcvfy = sslVerifyPath.Length() ? (const char *) sslVerifyPath : (const char *) 0;
		Trace("load SSL verify locations from [" << NotNull(pcafn) << "] path [" << NotNull(pcvfy) << "]");
		if (1 != (ret = SSL_CTX_load_verify_locations(ctx, pcafn, pcvfy))) {
			SSLSocket::ReportSSLError(SSLSocket::GetSSLError(0, ret));
			Trace("couldn't load verify locations from" << peerCAFileName << sslVerifyPath );
		}
	}
}

SSL_CTX *SSLModule::DoMakeServerContext(LookupInterface *object) {
	StartTrace(SSLModule.DoMakeServerContext);
	Assert(object); // we expect object to be not null
	SSL_CTX *ctx = 0;

	if (object) { // we check it nevertheless for robustness
		ctx = PrepareServerContext(object);
		if ( ctx ) {
			SetDiffieHellmannKey(ctx, object, "ServerDHLength", 1024L);
			ctx = SetOwnCertificateAndKey(ctx, object, SSLModule::eServer);
			if (ctx) {
				SetSSLCtxVerifyParameters(ctx, object);
				SetSSLSetAcceptableClientCAs(ctx, object);
				SetCipherList(ctx, object, "ServerCipherList", "HIGH:-SSLv2:+SSLv3");
				SetRSAKeyLength(ctx, object, "ServerRSALength", 2048L);
			}
		}
	}
	return ctx;
}
RegisterAcceptorFactory(SSLAcceptorFactory);

Acceptor *SSLAcceptorFactory::MakeAcceptor(AcceptorCallBack *ac) {
	StartTrace(SSLAcceptorFactory.MakeAcceptor);
	TraceAny(GetConfig(), "my own config");
	SSL_CTX *sslCtx = SSLModule::GetSSLCtx(this);
	if (!sslCtx) {
		return 0;
	}

	SSLSocketArgs sslSocketArgs(Lookup("VerifyCertifiedEntity", 0L) != 0L, Lookup("CertVerifyString", ""),
			Lookup("CertVerifyStringIsFilter", 0L) != 0L, false); // Server Sockets do session resumption based upon settings  in SSLModule,
	// using  openssl's  own session cache
	return new SSLAcceptor(sslCtx, Lookup("Address", (const char *) 0), Lookup("Port", 80L), Lookup("Backlog", 50L), ac, sslSocketArgs);
}
