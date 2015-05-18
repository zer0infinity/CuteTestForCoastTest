/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SSLSocket.h"
#include "SystemLog.h"
#include "SystemBase.h"
#include "SSLSocketStream.h"
#include "Resolver.h" // to verify host name versus certificate
#include "SSLSocketUtils.h"
#include "SSLObjectManager.h"

//#define STREAM_TRACE

//--- SSLSocket ----------------------
SSLSocket::SSLSocket(SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose, long  timeout, Allocator *a)
	: Socket(socket, clientInfo, doClose, timeout, a)
	, fContext(ctx)
	, fPeerCert(0)
	, fSSLSocketArgs()
{
	StartTrace1(SSLSocket.Ctor, "using allocator: [" << (long)a << "]");
}

SSLSocket::SSLSocket(SSLSocketArgs &sslSocketArgs, SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose, long  timeout, Allocator *a)
	: Socket(socket, clientInfo, doClose, timeout, a)
	, fContext(ctx)
	, fPeerCert(0)
	, fSSLSocketArgs(sslSocketArgs)
{
	StartTrace1(SSLSocket.Ctor, "using allocator: [" << (long)a << "]");
}

SSLSocket::~SSLSocket()
{
	StartTrace(SSLSocket.~SSLSocket);
}

unsigned long SSLSocket::GetSSLError(SSL *ssl, int res)
{
	return ( ssl ? SSL_get_error(ssl, res) : ERR_get_error() );
}

void SSLSocket::ReportSSLError( unsigned long err)
{
	StartTrace(SSLSocket.ReportSSLError);
	while (err) {
		// need to empty per thread error queue of SSL
		const int buflen = 256;
		char buf[buflen];
		ERR_error_string_n(err, buf, buflen);
		String logMsg("SSL error: ");
		logMsg << (long)err << " : " << buf;
		SystemLog::Error(logMsg);
		Trace(logMsg);
		err = GetSSLError();
	}
}

Anything SSLSocket::ReportSSLError(Anything &errAny, unsigned long err)
{
	StartTrace(SSLSocket.ReportSSLError);
	while (err) {
		// need to empty per thread error queue of SSL
		const int buflen = 256;
		char buf[buflen];
		ERR_error_string_n(err, buf, buflen);
		String logMsg("SSL error: ");
		logMsg << (long)err << " : " << buf;
		errAny.Append(logMsg);
		Trace(logMsg);
		err = GetSSLError();
	}
	return errAny;
}

bool SSLSocket::ShouldRetry(SSL *ssl, int res, bool handshake)
{
	StartTrace(SSLSocket.ShouldRetry);
	Trace("res = " << long(res) << " ssl = " << (long) ssl);
	if (res >= 1) {
		return false;
	}
	unsigned long err = GetSSLError(ssl, res);
	Trace("err:" << static_cast<long>(err));
	if (SSL_ERROR_WANT_READ == err) {
		return IsReadyForReading();
	} else if (SSL_ERROR_WANT_WRITE == err) {
		return IsReadyForWriting();
	} else if (SSL_ERROR_ZERO_RETURN) {
		// clean  way to handle peer did not send data
		// according to http://www.mail-archive.com/openssl-users@openssl.org/msg03175.html we shall not use ERR_error_string method with SSL error codes
		String msg("SSLSocket: end of data (connection closed) on file descriptor: ");
		msg << GetFd() << (handshake ? " at Handshake" : " at normal r/w");
		SystemLog::Info(msg);
		return false;
	}
	ReportSSLError(err);
	return false;
}

String SSLSocket::GetSessionID(SSL *ssl)
{
	Assert(ssl);

	String result;
	SSL_SESSION *session = SSL_get_session(ssl);
	if (!session) {
		return String();
	}
	return String((void *)session->session_id, session->session_id_length);
}

SSL_SESSION *SSLSocket::SessionResumptionHookResumeSession(SSL *ssl)
{
	StartTrace(SSLSocket.SessionResumptionHookResumeSession);
	return (SSL_SESSION *) NULL;
}

void SSLSocket::SessionResumptionHookSetSession(SSL *ssl, SSL_SESSION *sslSessionStored, bool wasResumed)
{
	StartTrace(SSLSocket.SessionResumptionHookSetSession);
	return;
}

std::iostream *SSLSocket::DoMakeStream()
{
	StartTrace(SSLSocket.DoMakeStream);
	Anything &sslinfo = fClientInfo["SSL"];
	sslinfo["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"] = false;
	sslinfo["Peer"]["AppLevelCertVerifyStatus"] = false;
	SSL *ssl = SSL_new (fContext);
	Assert(ssl);

	// Continuing with invalid SSL structure will segfault.
	// Returning a NULL pointer is a trifle better.
	if ( ssl == (SSL *) NULL ) {
		String logMsg("SSL error: SSLContext not good.");
		SystemLog::Error(logMsg);
		return NULL;
	}

	bool verifyCallbackWasSet = SSL_CTX_get_verify_callback(fContext) != NULL;
	Anything appData;
	if ( verifyCallbackWasSet ) {
		appData["SSLSocketArgs"] = (IFAObject *) &fSSLSocketArgs;
		appData.Append(sslinfo);

		int thread_id = (unsigned)Thread::MyId() % 1000000;
		if ( SSL_set_ex_data(ssl, thread_id, &appData) == false ) {
			String logMsg("SSL error: Setting application specific data failed.");
			SystemLog::Error(logMsg);
			return NULL;
		}
	}
	if (GetTimeout() > 0) {
		Socket::SetToNonBlocking(GetFd());
	}
	Trace("SSL_set_fd: " << GetFd());
	int res = SSL_set_fd (ssl, GetFd());
	ReportSSLError(GetSSLError(ssl, res));

	SSL_SESSION *sslSessionStored = SessionResumptionHookResumeSession(ssl);

	Trace("---- connect sequence");
	do {
		res = PrepareSocket(ssl);
	} while (ShouldRetry(ssl, res, true));
	if ( res <= 0 ) {
		// 0 = Handshake failure, SSL layer shut down connection properly, -1 = error, 1 = ok
		if ( ssl ) {
			SSL_free (ssl);
		}
		ssl = 0;
		ERR_clear_error();
		return 0;
	}
	Trace("---- end connect sequence");

	Trace("SSL connection using " << SSL_get_cipher (ssl));
	String sessionID = GetSessionID(ssl);
	sslinfo["SessionId"] = String().AppendAsHex((const unsigned char *)(const char *)sessionID, sessionID.Length());
	if ( verifyCallbackWasSet ) {
		// This data will be filled if /SSLUseAppCallBack was given in SSLModule
		sslinfo["PreVerify"] = appData["Chain"];
	}
	sslinfo["Cipher"] = String(SSL_get_cipher(ssl));
	sslinfo["SessionIsResumed"] = SSL_session_reused(ssl);

	DoCheckPeerCertificate(sslinfo, ssl);

	// Application decides to continue or not depending on CLIENT_INFO
	TraceAny(fClientInfo, "SSL client info");
	Trace(" session cache number: " << SSL_CTX_sess_number(fContext) << '\n'
		  << " size: " << SSL_CTX_sess_get_cache_size(fContext) << '\n'
		  << " hits: " << SSL_CTX_sess_hits(fContext) << '\n'
		  << " misses: " << SSL_CTX_sess_misses(fContext) << '\n'
		  << " timeouts: " << SSL_CTX_sess_timeouts(fContext) << '\n'
		  << " overflows: " << SSL_CTX_sess_cache_full(fContext) << '\n'
		  << " mode: " << SSL_CTX_get_session_cache_mode(fContext) << '\n'
		  << " timeout: " << SSL_CTX_get_timeout(fContext) << '\n'
		 );

	SessionResumptionHookSetSession(ssl, sslSessionStored, sslinfo["SessionIsResumed"].AsLong(0) != 0L);

	SSL_SESSION *sslSessionCurrent;
	if ( (sslSessionCurrent = SSL_get_session(ssl)) == (SSL_SESSION *) NULL) {
		Trace("No valid SSL_SESSION created!!!");
		String logMsg("SSL error: SSLSession not good.");
		SystemLog::Error(logMsg);
		return NULL;
	} else {
		TraceAny(SSLObjectManager::TraceSSLSession(sslSessionCurrent), "sslSessionCurrent");
		TraceAny(sslinfo, "SSL  info");
	}
//	SSL_set_shutdown(ssl, SSL_SENT_SHUTDOWN|SSL_RECEIVED_SHUTDOWN);
	return new(GetAllocator()) SSLSocketStream(ssl, this, GetTimeout());
}

void SSLSocket::DoCheckPeerCertificate(Anything &sslinfo, SSL *ssl)
{
	StartTrace(SSLSocket.DoCheckPeerCertificate);
// Commented code should only be done on clients for checking servers
// more work needed for wildcard certs:
// sslinfo["hostnameverify"] = (fClientInfo["REMOTE_ADDR"].AsString()
// == Resolver::DNS2IPAddress(sslinfo["peer"]["CN"].AsString()));
// should validate host name of peer also
	Trace("VerifyCertifiedEntity: " << fSSLSocketArgs.VerifyCertifiedEntity());
	Trace("CertVerifyString: " << fSSLSocketArgs.CertVerifyString());
	Trace("CertVerifyStringIsFilter: " << fSSLSocketArgs.CertVerifyStringIsFilter());
	if ( CheckPeerCertificate(ssl, sslinfo) == true ) {
		if ( fSSLSocketArgs.VerifyCertifiedEntity() == 1 ) {
			sslinfo["Peer"]["Whole"] = SSLSocketUtils::GetPeerAsString(fPeerCert);
			sslinfo["Peer"]["ParsedDn"] = SSLSocketUtils::ParseDN(sslinfo["Peer"]["Whole"].AsString());
			sslinfo["Peer"]["Issuer"] = SSLSocketUtils::GetPeerIssuerAsString(fPeerCert);
			if (fSSLSocketArgs.CertVerifyStringIsFilter()) {
				sslinfo["Peer"]["AppLevelCertVerifyStatus"] = SSLSocketUtils::VerifyDN(fSSLSocketArgs.CertVerifyString(), sslinfo["Peer"]["ParsedDn"]);
			} else {
				sslinfo["Peer"]["AppLevelCertVerifyStatus"] = (fSSLSocketArgs.CertVerifyString() == sslinfo["Peer"]["Whole"].AsString());
				Trace("Comparing: " << fSSLSocketArgs.CertVerifyString() << " vs. " << sslinfo["Peer"]["Whole"].AsString());
			}
		}
	} else {
		sslinfo["Peer"]["AppLevelCertVerifyStatus"] = false;
	}
	if ( fPeerCert ) {
		X509_free(fPeerCert);
	}
	// A not legal, but effective way to terminate the ssl handshake.
	// You need to define SSL_set_verify_result yourself, not part of
	// OpenSSL public API, but known "hack".
	// SSL_set_verify_result(ssl,X509_V_ERR_APPLICATION_VERIFICATION);

}

int SSLSocket::SSLVerifyCallbackOk(int preverify_ok, X509_STORE_CTX *ctx)
{
	StartTrace(SSLSocket.SSLVerifyCallbackOk);
	return 1;
}

int SSLSocket::SSLVerifyCallbackFalse(int preverify_ok, X509_STORE_CTX *ctx)
{
	StartTrace(SSLSocket.SSLVerifyCallbackFalse);
	return 0;
}

int SSLSocket::SSLVerifyCallback(int preverify_ok, X509_STORE_CTX *ctx)
{
	StartTrace(SSLSocket.SSLVerifyCallback);
	int err = X509_STORE_CTX_get_error(ctx);
	int depth = X509_STORE_CTX_get_error_depth(ctx);
//   /*
//	* Retrieve the pointer to the SSL of the connection currently treated
//	* and the application specific data stored into the SSL object.
//	*/
	SSL *ssl = (SSL *)  X509_STORE_CTX_get_ex_data(ctx, SSL_get_ex_data_X509_STORE_CTX_idx());
	int allowedDepth = SSL_get_verify_depth(ssl);
	int thread_id = (unsigned)Thread::MyId() % 1000000;
	Anything *pAppData = (Anything *)  SSL_get_ex_data(ssl, thread_id);
	if (depth > allowedDepth + 1) {
		preverify_ok = 0;
		err = X509_V_ERR_CERT_CHAIN_TOO_LONG;
		X509_STORE_CTX_set_error(ctx, err);
	}
	X509 *cert = X509_STORE_CTX_get_current_cert(ctx);
	(*pAppData)["Chain"]["Subjects"].Append(SSLSocketUtils::GetPeerAsString(cert));
	(*pAppData)["Chain"]["Issuers"].Append(SSLSocketUtils::GetPeerIssuerAsString(cert));
	(*pAppData)["Chain"]["Depth"] = depth;
	(*pAppData)["Chain"]["Alloweddepth"] = allowedDepth;
	if (!preverify_ok) {
		(*pAppData)["Chain"]["Error"] = String(X509_verify_cert_error_string(err));
	}
	return preverify_ok;
}

long SSLSocket::GetReadCount() const
{
	StartTrace(SSLSocket.GetReadCount);
	if (fStream) {
		SSLSocketStreamBuf *sbuf = ((SSLSocketStream *)fStream)->rdbuf();
		if (sbuf) {
			return sbuf->GetReadCount();
		}
	}
	return 0L;
}

long SSLSocket::GetWriteCount() const
{
	StartTrace(SSLSocket.GetWriteCount);
	if (fStream) {
		SSLSocketStreamBuf *sbuf = ((SSLSocketStream *)fStream)->rdbuf();
		if (sbuf) {
			return sbuf->GetWriteCount();
		}
	}
	return 0L;
}
bool SSLSocket::CheckPeerCertificate(SSL *ssl,  Anything &sslinfo)
{
	StartTrace(SSLSocket.CheckPeerCertificate);
	TraceAny(sslinfo, "ssl info");
	fPeerCert = SSL_get_peer_certificate(ssl);
	bool ret = (fPeerCert != NULL);
	Trace("peer cert " << (ret ? "OK" : "MISSING"));
	long verifyresult = SSL_get_verify_result(ssl);
	ReportSSLError(sslinfo["Peer"]["SSLCertVerifyStatus"]["SSL"]["ErrorDesc"], verifyresult);
	ret = (verifyresult == X509_V_OK);
	sslinfo["Peer"]["SSLCertVerifyStatus"]["SSL"]["SSLRetcode"] = verifyresult;
	sslinfo["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"] = ret;
	Trace("peer cert check returned " << verifyresult << " that is " << (ret ? "OK" : "ERROR"));
	return ret;
}

bool SSLSocket::IsCertCheckPassed(ROAnything config)
{
	StartTrace(SSLSocket.IsCertCheckPassed);
	TraceAny(config, "the Config");

	Anything clientInfo(ClientInfo());
	TraceAny(clientInfo, "ClientInfo");
	Trace(fSSLSocketArgs.ShowState());

	// No peer verification requested, no callback installed, no application layer check specified...
	if ((config["SSLVerifyPeerCert"].AsLong(0) 			== 0) &&
		(config["SSLVerifyFailIfNoPeerCert"].AsLong(0)	== 0) &&
		(config["SSLUseAppCallback"].AsLong(0)			== 0) &&
		(fSSLSocketArgs.VerifyCertifiedEntity() == 0)) {
		Trace("Returning true");
		return true;
	}
	// We must verify certified entity....
	if ( fSSLSocketArgs.VerifyCertifiedEntity() ) {
		bool ret = clientInfo["SSL"]["Peer"]["AppLevelCertVerifyStatus"].AsLong(0) != 0L;
		Trace("Returning " << ret);
		return ret;
	}
	if (clientInfo["SSL"]["Peer"]["SSLCertVerifyStatus"]["SSL"]["Ok"].AsLong(0) == 1) {
		Trace("Returning true");
		return true;
	}
	Trace("Returning false");
	return false;
}

//Anything SSLSocket::GetPeer()
//{
//	Anything result;
//	String multiline = GetNameFromX509Name(X509_get_subject_name(fPeerCert,XN_FLAG_FRC2253));
//	//parse string and put in anything
//}

//--- SSLClientSocket ----------------------
SSLClientSocket::SSLClientSocket(SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose, long timeout, Allocator *a)
	: SSLSocket(ctx, socket, clientInfo, doClose, timeout, a)
{
	StartTrace(SSLClientSocket.SSLClientSocket);
}

SSLClientSocket::SSLClientSocket(SSLSocketArgs sslSocketArgs, SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose, long timeout, Allocator *a)
	: SSLSocket(sslSocketArgs, ctx, socket, clientInfo, doClose, timeout, a)
{
	StartTrace(SSLClientSocket.SSLClientSocket);
}

SSLClientSocket::~SSLClientSocket()
{
	StartTrace(SSLClientSocket.~SSLClientSocket);
}

int SSLClientSocket::PrepareSocket(SSL *ssl)
{
	int res = SSL_connect( ssl );
	StatTrace(SSLClientSocket.PrepareSocket, "res:" << (long)res, coast::storage::Current());
	return res;
}

SSL_SESSION *SSLClientSocket::SessionResumptionHookResumeSession(SSL *ssl)
{
	StartTrace(SSLClientSocket.SessionResumptionHookResumeSession);
	SSL_SESSION *sslSessionStored  = NULL;
	if (fSSLSocketArgs.SessionResumption()) {
		sslSessionStored = SSLObjectManager::SSLOBJMGR()->GetSessionId(fClientInfo["REMOTE_ADDR"].AsString(), fClientInfo["REMOTE_PORT"].AsString());
		if (sslSessionStored != (SSL_SESSION *) NULL) {
			TraceAny(SSLObjectManager::TraceSSLSession(sslSessionStored), "sslSessionStored");
			Trace("Trying to re-use sslSessionIdStored: " << SSLObjectManager::SessionIdAsHex(sslSessionStored));
			int res = SSL_set_session(ssl, sslSessionStored);
			ReportSSLError(GetSSLError(ssl, res));
			Trace("Trying re-using sslSessionIdStored; SSL_set_session() returned: " << res);
		}
	}
	return   sslSessionStored;
}

void SSLClientSocket::SessionResumptionHookSetSession(SSL *ssl, SSL_SESSION *sslSessionStored, bool wasResumed)
{
	StartTrace(SSLClientSocket.SessionResumptionHookSetSession);
	SSL_SESSION *sslSessionCurrent = NULL;
	if (fSSLSocketArgs.SessionResumption()) {
		if (wasResumed == false) {
			sslSessionCurrent = SSL_get1_session(ssl);
			Trace("Storing inital sessionId: " << SSLObjectManager::SessionIdAsHex(sslSessionCurrent));
			SSLObjectManager::SSLOBJMGR()->SetSessionId(fClientInfo["REMOTE_ADDR"].AsString(), fClientInfo["REMOTE_PORT"].AsString(), sslSessionCurrent);
		}
	}
}

//--- SSLServerSocket ----------------------
SSLServerSocket::SSLServerSocket(SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose, long timeout, Allocator *a)
	: SSLSocket(ctx, socket, clientInfo, doClose, timeout, a)
{
	StartTrace1(SSLServerSocket.Ctor, "using allocator: [" << (long)a << "]");
}

SSLServerSocket::SSLServerSocket(SSLSocketArgs sslSocketArgs, SSL_CTX *ctx, int socket, const Anything &clientInfo, bool doClose, long timeOut, Allocator *a)
	: SSLSocket(sslSocketArgs, ctx, socket, clientInfo, doClose, timeOut, a)
{
	StartTrace1(SSLServerSocket.Ctor, "using allocator: [" << (long)a << "]");
}

SSLServerSocket::~SSLServerSocket()
{
	StartTrace(SSLSocket.~SSLSocket);

}
int SSLServerSocket::PrepareSocket(SSL *ssl)
{
	StartTrace(SSLServerSocket.PrepareSocket);
	return ( SSL_accept (ssl)  );
}

//---- SSLSocketArgs ----------------------------------------------
SSLSocketArgs::SSLSocketArgs(bool verifyCertifiedEntity, const String &certVerifyString,
							 bool certVerifyStringIsFilter, bool sessionResumption) :
	fVerifyCertifiedEntity(verifyCertifiedEntity),
	fCertVerifyString(certVerifyString),
	fCertVerifyStringIsFilter(certVerifyStringIsFilter),
	fSessionResumption(sessionResumption)
{
	StartTrace(SSLSocketArgs.SSLSocketArgs);
}

SSLSocketArgs::SSLSocketArgs()  :	fVerifyCertifiedEntity(false),
	fCertVerifyString(),
	fCertVerifyStringIsFilter(false),
	fSessionResumption(false)
{
}

SSLSocketArgs::~SSLSocketArgs()
{
	StartTrace(SSLSocketArgs.~SSLSocketArgs);
}

SSLSocketArgs &SSLSocketArgs::operator=(const SSLSocketArgs &sa)
{
	StartTrace(SSLSocketArgs.operator = );

	if ( this == &sa ) {
		return *this;
	}
	fVerifyCertifiedEntity 		= sa.fVerifyCertifiedEntity;
	fCertVerifyString 			= sa.fCertVerifyString;
	fCertVerifyStringIsFilter 	= sa.fCertVerifyStringIsFilter;
	fSessionResumption		 	= sa.fSessionResumption;
	return *this;
}

String SSLSocketArgs::CertVerifyString()
{
	StartTrace(SSLSocketArgs.CertVerifyString);
	return fCertVerifyString;
}

bool SSLSocketArgs::VerifyCertifiedEntity()
{
	StartTrace(SSLSocketArgs.VerifyCertifiedEntity);
	return fVerifyCertifiedEntity;
}

bool SSLSocketArgs::CertVerifyStringIsFilter()
{
	StartTrace(SSLSocketArgs.CertVerifyStringIsFilter);
	return fCertVerifyStringIsFilter;
}

bool SSLSocketArgs::SessionResumption()
{
	StartTrace(SSLSocketArgs.SessionResumption);
	return fSessionResumption;
}

String SSLSocketArgs::ShowState()
{
	StartTrace(SSLSocketArgs.ShowState);

	String work;
	work << "VerifyCertifiedEntity:" << VerifyCertifiedEntity() <<
		 " CertVerifyString: " << CertVerifyString() <<
		 " CertVerifyStringIsFilter: " << CertVerifyStringIsFilter() <<
		 " SessionResumption: " << SessionResumption();
	return work;
}

//--- SSLConnector --------------------------------------------
SSLConnector::SSLConnector(const char *ipAdr, long port, long connectTimeout, SSL_CTX *ctx, const char *srcIpAdr, long srcPort, bool threadLocal)
	:	Connector(ipAdr, port, connectTimeout, srcIpAdr, srcPort, threadLocal),
		fContext((ctx) ? ctx : SSL_CTX_new(SSLv23_client_method())),
		fSSLSocketArgs()
{
	fDeleteCtx = (ctx == 0);
}

SSLConnector::SSLConnector(ConnectorArgs &connectorArgs, SSL_CTX *ctx, const char *srcIpAdr, long srcPort, bool threadLocal)
	:	Connector(connectorArgs, srcIpAdr, srcPort, threadLocal),
		fContext((ctx) ? ctx : SSL_CTX_new(SSLv23_client_method())),
		fSSLSocketArgs()
{
	fDeleteCtx = (ctx == 0);
}

SSLConnector::SSLConnector(ConnectorArgs &connectorArgs,
						   SSLSocketArgs sslSocketArgs,
						   ROAnything sslModuleCfg,
						   SSL_CTX *ctx,
						   const char *srcIpAdr, long srcPort, bool threadLocal)
	:	Connector(connectorArgs, srcIpAdr, srcPort, threadLocal),
		fContext((ctx) ? ctx : SSLObjectManager::SSLOBJMGR()->GetCtx(connectorArgs.IPAddress(), connectorArgs.PortAsString(), sslModuleCfg)),
		fSSLSocketArgs(sslSocketArgs)
{
	// If caller provides SSL_CTX, we do not delete it.
	// If SSLOBJMGR creates the SSL_CTX it will delete it too!
	fDeleteCtx = false;
}

SSLConnector::SSLConnector(ROAnything config)
	: Connector(config["Address"].AsCharPtr("127.0.0.1"),
				config["Port"].AsLong(443L),
				config["Timeout"].AsLong(0L),
				config["SrcAddress"].AsCharPtr(0),
				config["SrcPort"].AsLong(0L),
				config["UseThreadLocalMemory"].AsLong(0L) != 0L),
	fContext(SSLObjectManager::SSLOBJMGR()->GetCtx(config["Address"].AsCharPtr("127.0.0.1"),
			 config["Port"].AsCharPtr("443"),
			 config)),
	fSSLSocketArgs(config["VerifyCertifiedEntity"].AsBool(0),
				   config["CertVerifyString"].AsString(),
				   config["CertVerifyStringIsFilter"].AsBool(0),
				   config["SessionResumption"].AsBool(0))
{
	StartTrace(SSLConnector.SSLConnector);
	TraceAny(config, "config used");
	fDeleteCtx = false;
}

SSLConnector::SSLConnector(ROAnything config, SSL_CTX *ctx, bool deleteCtx)
	: Connector(config["Address"].AsCharPtr("127.0.0.1"),
				config["Port"].AsLong(443L),
				config["Timeout"].AsLong(0L),
				config["SrcAddress"].AsCharPtr(0),
				config["SrcPort"].AsLong(0L),
				config["UseThreadLocalMemory"].AsLong(0L) != 0L),
	fContext((ctx) ? ctx : SSLModule::GetSSLClientCtx(config)),
	fSSLSocketArgs(config["VerifyCertifiedEntity"].AsBool(0),
				   config["CertVerifyString"].AsString(),
				   config["CertVerifyStringIsFilter"].AsBool(0),
				   config["SessionResumption"].AsBool(0))
{
	StartTrace(SSLConnector.SSLConnector);
	TraceAny(config, "config used");
	fDeleteCtx = deleteCtx;
}

SSLConnector::~SSLConnector()
{
	if (fSocket) {
		delete fSocket;
	}
	fSocket = 0;

	if ( fDeleteCtx && fContext ) {
		SSL_CTX_free(fContext);
		fContext = 0;
	}
}

Socket *SSLConnector::MakeSocket(bool doClose)
{
	StartTrace(SSLConnector.MakeSocket);
	Socket *s = 0;
	if ( (fSrcPort == 0) || (!fSocket) ) {
		if ( DoConnect()) {
			Anything clientInfo;
			clientInfo["REMOTE_ADDR"] = fIPAddress;
			clientInfo["REMOTE_PORT"] = fPort;
			coast::system::SetCloseOnExec(GetFd());
			s = DoMakeSocket(fSSLSocketArgs, GetFd(), clientInfo, doClose);
		}
		if (GetFd() >= 0 && ! s) {
			closeSocket(GetFd());
			fSockFd = -1;
		}
	}
	return s;
}

Socket *SSLConnector::DoMakeSocket(int socket, Anything &clientInfo, bool doClose)
{
	StartTrace(SSLConnector.DoMakeSocket);
	clientInfo["HTTPS"] = true;
	Allocator *wdallocator = GetSocketAllocator();
	return new (wdallocator) SSLClientSocket(fContext, socket, clientInfo, doClose, GetDefaultSocketTimeout(), wdallocator);
}

Socket *SSLConnector::DoMakeSocket(SSLSocketArgs sslSocketArgs, int socket, Anything &clientInfo, bool doClose)
{
	StartTrace(SSLConnector.DoMakeSocket);
	clientInfo["HTTPS"] = true;
	Allocator *wdallocator = GetSocketAllocator();
	return new (wdallocator) SSLClientSocket(sslSocketArgs, fContext, socket, clientInfo, doClose, GetDefaultSocketTimeout(), wdallocator);
}

//--- SSLAcceptor ---
SSLAcceptor::SSLAcceptor(SSL_CTX *ctx, const char *ipadress, long port, long backlog, AcceptorCallBack *cb, SSLSocketArgs sslSocketArgs)
	:	Acceptor(ipadress, port, backlog, cb), fContext(ctx), fSSLSocketArgs(sslSocketArgs)
{

}

SSLAcceptor::~SSLAcceptor()
{
	if (fContext) {
		SSL_CTX_free(fContext);
	}
}

Socket *SSLAcceptor::DoMakeSocket(int socket, Anything &clientInfo, bool doClose)
{
	StartTrace(SSLAcceptor.DoMakeSocket);
	clientInfo["HTTPS"] = true;
	Allocator *wdallocator = GetSocketAllocator();

	return new (wdallocator) SSLServerSocket(fSSLSocketArgs, fContext, socket, clientInfo, doClose, GetDefaultSocketTimeout(), wdallocator);
}
