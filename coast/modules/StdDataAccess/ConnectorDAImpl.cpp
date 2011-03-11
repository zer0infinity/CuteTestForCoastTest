/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ConnectorDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "SSLSocket.h"
#include "SystemLog.h"
#include "SocketStream.h"
#include "StringStream.h"
#include "Timers.h"

//--- c-modules used -----------------------------------------------------------
#if defined(WIN32)
#include <io.h>				// for SO_ERROR
#else
#include <sys/socket.h>		// for SO_ERROR
#endif

//--- ConnectorDAImpl -----------------------------------------------------
RegisterDataAccessImpl(ConnectorDAImpl);

IFAObject *ConnectorDAImpl::Clone(Allocator *a) const
{
	return new (a) ConnectorDAImpl(fName);
}

bool ConnectorDAImpl::Exec( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ConnectorDAImpl.Exec);

	bool bCloseSocket = true, bRetCode = false, bRecreate = false;
	Anything anySocketParams;
	Connector *pConnector = NULL;
	Socket *pSocket = NULL;
	in->Get("CloseSocket", bCloseSocket, context);
	if ( in->Get("SocketParams", anySocketParams, context) && !anySocketParams.IsNull() ) {
		TraceAny(anySocketParams, "Reloaded SocketParams");
		if ( (pSocket = RecreateSocket(anySocketParams, context, in, out)) != NULL ) {
			bRecreate = true;
		} else {
			// clean SocketParams and anySocketParams
			anySocketParams = Anything();
			out->Put("SocketParams", anySocketParams, context);
		}
	}

	if ( pSocket == NULL ) {
		// create new connector
		pConnector = DoMakeConnector(context, in, out);
		if ( pConnector ) {
			pSocket = DoConnect(pConnector, context, in, out);
			if ( pSocket ) {
				// store params
				anySocketParams = pSocket->ClientInfo();
				anySocketParams["SocketFd"] = pSocket->GetFd();
				TraceAny(anySocketParams, "SocketParams");
				out->Put("SocketParams", anySocketParams, context);
			}
		} else {
			SYSWARNING("failed to create Connector");
		}
	}

	if ( pSocket ) {
		// set socket option not to close the filedescriptor when Socket gets destructed
		Trace("closeflag : " << (bCloseSocket ? "true" : "false"));
		pSocket->CloseOnDelete(bCloseSocket);
		bRetCode = DoExec(pSocket, context, in, out);
		Trace("bRetCode : " << (bRetCode ? "true" : "false"));
	} else {
		SYSWARNING("Socket not valid!");
	}
	if ( bRecreate && pSocket ) {
		Trace("deleting recreated socket");
		delete pSocket;
	}

	// delete Connector object -> also deletes socket!
	delete pConnector;
	pSocket = NULL;
	if ( bCloseSocket ) {
		// clean SocketParams
		out->Put("SocketParams", Anything(), context);
	}

	return bRetCode;
}

Connector *ConnectorDAImpl::DoMakeConnector( Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ConnectorDAImpl.DoMakeConnector);
	Connector *pConnector = NULL;
	String strAddress;
	long lPort = 0L, lConnTimeout = 10L;
	bool bUseSSL = false;
	if ( !in->Get("Address", strAddress, context) ) {
		strAddress = EndPoint::GetLocalHost();
		SYSINFO("address set to " << strAddress);
	}
	if ( !in->Get("Port", lPort, context) ) {
		SYSERROR("Port not given!");
		return NULL;
	}
	in->Get("UseSSL", bUseSSL, context);
	in->Get("ConnectTimeout", lConnTimeout, context);
	lConnTimeout *= 1000L;
	Trace( "Address<" << strAddress << "> Port[" << lPort << "] " << (bUseSSL ? "SSL" : "") << " ConnectTimeout: " << lConnTimeout << "ms");
	if ( bUseSSL ) {
		pConnector = new SSLConnector(strAddress, lPort, lConnTimeout, (SSL_CTX *)context.Lookup("SSLContext").AsIFAObject(0));
	} else {
		pConnector = new Connector(strAddress, lPort, lConnTimeout);
	}
	return pConnector;
}

Socket *ConnectorDAImpl::DoConnect(Connector *csc, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ConnectorDAImpl.DoConnect);
	Socket *s = NULL;
	DAAccessTimer(ConnectorDAImpl.DoConnect, "connecting <" << fName << ">", context);
	s = csc->Use();
	// error message here in case of failure
	return s;
}

Socket *ConnectorDAImpl::RecreateSocket(Anything &anyParams, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ConnectorDAImpl.RecreateSocket);
	Socket *s = NULL;
	long socketFd = 0L;
	bool bUseSSL = false;
	ROAnything roaParams = anyParams;
	socketFd = roaParams["SocketFd"].AsLong(0L);

	if ( socketFd != 0L ) {
		int error = 0;
		// check if given socketFd is valid
		bool bSuccess = Socket::GetSockOptInt(socketFd, SO_ERROR, error);
		if ( bSuccess && error == 0 ) {
			// recreate socket using the params
			in->Get("UseSSL", bUseSSL, context);
			if ( roaParams["HTTPS"].AsBool() || bUseSSL ) {
				s = new (Coast::Storage::Global()) SSLClientSocket((SSL_CTX *)context.Lookup("SSLContext").AsIFAObject(0), socketFd, anyParams, true);
			} else {
				s = new (Coast::Storage::Global()) Socket(socketFd, anyParams, true);
			}

			Trace("socket recreated");
			long retCode;
			if ( s->IsReadyForReading(0, retCode) ) {
				std::iostream *pIos = s->GetStream();
				pIos->peek();
				if ( !pIos->good() ) {
					Trace("stream is bad, destroying recreated socket");
					delete s;
					s = NULL;
				}
			}
		}
	}

	return s;
}

bool ConnectorDAImpl::DoExec(Socket *pSocket, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ConnectorDAImpl.DoExec);
	long lSocketStreamTimeout = 30L, lCheckReadyTimeout = 30L, lSocketTimeout = 300L;
	bool bTcpNoDelay = true;
	in->Get("SocketTimeout", lSocketTimeout, context);
	in->Get("SocketStreamTimeout", lSocketStreamTimeout, context);
	in->Get("CheckReadyTimeout", lCheckReadyTimeout, context);
	in->Get("TcpNoDelay", bTcpNoDelay, context);
	lSocketTimeout *= 1000L;
	lSocketStreamTimeout *= 1000L;
	lCheckReadyTimeout *= 1000L;
	std::iostream *pIos = NULL;
	if ( pSocket ) {
		if ( bTcpNoDelay ) {
			pSocket->SetNoDelay();
		}
		pSocket->SetTimeout(lSocketTimeout);
		pIos = pSocket->GetStream();
	}
	if ( pIos == NULL ) {
		return HandleError(pSocket, "connection to: ", context, in, out);
	}
	{
		DAAccessTimer(ConnectorDAImpl.DoExec, "writing", context);
		if ( !SendInput(pIos, pSocket, lCheckReadyTimeout, lSocketStreamTimeout, context, in, out) || !(*pIos)) {
			return false;
		}
	}
	{
		DAAccessTimer(ConnectorDAImpl.DoExec, "reading", context);
		long lRetCode = 0L;
		Trace("waiting on ReadyForReading, CheckReadyTimeout:" << lCheckReadyTimeout);
		if ( pSocket->IsReadyForReading(lCheckReadyTimeout, lRetCode) ) {
			Trace("waiting on output from other party, SocketStreamTimeout:" << lSocketStreamTimeout);
			DiffTimer aReadTimer;
			TimeoutModifier aTimeoutModifier((SocketStream *) pIos, lSocketStreamTimeout);
			aTimeoutModifier.Use();
			if ( out->Put("Output", *pIos, context) ) {
				return true;
			} else {
				String strError("put into Output slot took ");
				strError << (long)aReadTimer.Diff() << "ms from: ";
				HandleError(pSocket, strError, context, in, out);
			}
		} else {
			String strError("not ready for reading");
			if ( lRetCode == 0 ) {
				strError << " after " << lCheckReadyTimeout << "s";
			}
			strError << " from: ";
			HandleError(pSocket, strError, context, in, out);
		}
	}
	return false;
}

bool ConnectorDAImpl::SendInput(std::iostream *pIos, Socket *s, long lCheckReadyTimeout, long lSocketStreamTimeout, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ConnectorDAImpl.SendInput);

	if ( pIos ) {
		long lRetCode = 0L;
		Trace("waiting on ReadyForWriting, CheckReadyTimeout:" << lCheckReadyTimeout);
		if ( s->IsReadyForWriting(lCheckReadyTimeout, lRetCode) ) {
			Trace("writing input to other party, SocketStreamTimeout:" << lSocketStreamTimeout);
			TimeoutModifier aTimeoutModifier((SocketStream *) pIos, lSocketStreamTimeout);
			aTimeoutModifier.Use();
			bool retCode = in->Get("Input", *pIos, context);
			pIos->flush();
			Trace("input sent and stream flushed");
			if (retCode) {
				Trace("returning true");
				return true;
			}
			HandleError(s, "sending request to: ", context, in, out);
		} else {
			String strError("not ready for writing");
			if ( lRetCode == 0 ) {
				strError << " after " << lCheckReadyTimeout << "s";
			}
			strError << " to: ";
			HandleError(s, strError, context, in, out);
		}
	}
	return false;
}

bool ConnectorDAImpl::HandleError( Socket *s, const char *streamText, Context &context, ParameterMapper *in, ResultMapper *out)
{
	StartTrace(ConnectorDAImpl.HandleError);
	Trace(" Errortext to use : " << streamText);

	String strBuf;
	{
		OStringStream stream(strBuf);
		stream << streamText;
		s->ClientInfo().PrintOn(stream) << " failed!";
		stream.flush();
	}
	out->Put("Error", strBuf, context);
	SYSERROR(strBuf);

	return false;
}
