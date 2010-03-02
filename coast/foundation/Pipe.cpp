/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Pipe.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SystemLog.h"
#include "Dbg.h"
#include "Socket.h"

//--- c-library modules used ---------------------------------------------------
#if !defined (WIN32)
#include <errno.h> /* PS wg. SUNCC5 */
#endif

//--- Pipe ----------------------
Pipe::Pipe(int readfd, int writefd, bool doClose, long timeout)
	: fReadFd(readfd)
	, fWriteFd(writefd)
	, fStream(0)
	, fDoClose(doClose)
	, fTimeout(timeout)
	, fHadTimeout(false)
{
	StartTrace1(Pipe.Pipe, "ReadFd:" << (long)readfd << " WriteFd:" << (long)writefd << (doClose ? "" : " non") <<  " closing");
}

Pipe::Pipe(long timeout)
	: fReadFd(-1)
	, fWriteFd(-1)
	, fStream(0)
	, fDoClose(false)
	, fTimeout(timeout)
	, fHadTimeout(false)
{
	StartTrace(Pipe.Pipe);
	Open();
}

Pipe::~Pipe()
{
	StartTrace(Pipe.~Pipe);
	if ( fStream ) {
		Trace("Deleting stream");
		delete fStream;
		fStream = 0;
	}
	if ( fDoClose ) {
		Trace("Closing pipes: " << fReadFd << "," << fWriteFd);
		ShutDownWriting();
		ShutDownReading();
	}
}

bool Pipe::ShutDown(long fd)
{
	StartTrace1(Pipe.ShutDown, "fd:" << fd);
	if ( (fd >= 0) && closeSocket(fd) < 0 ) {
		String logMsg("error in close (");
		logMsg << SystemLog::LastSysError() << ")";
		SYSERROR(logMsg);
		Trace(logMsg);
		return false;
	}
	return true;
}

PipeStream *Pipe::GetStream()
{
	StartTrace(Pipe.GetStream);
	if ( !fStream ) {
		Trace("creating new PipeStream");
		fStream = DoMakeStream();
	}
	return fStream;
}

PipeStream *Pipe::DoMakeStream()
{
	StartTrace(Pipe.DoMakeStream);
	if ( GetReadFd() >= 0 || GetWriteFd() >= 0) {
		Trace("creating new PipeStream");
		return new PipeStream(this, GetTimeout());
	}
	Trace("no valid readfd:" << GetReadFd() << " or writefd:" << GetWriteFd() << "!");
	return 0;
}

long Pipe::GetReadFd()
{
	return fReadFd;
}

long Pipe::GetWriteFd()
{
	return fWriteFd;
}

bool Pipe::IsReady(bool forreading, long timeout)
{
	StartTrace1(Pipe.IsReady, (forreading ? "for reading" : "for writing"));
	int fd = forreading ? GetReadFd() : GetWriteFd();
	if (-2 == timeout) {
		timeout = GetTimeout();
	}
	fHadTimeout = false;
	Trace("testing fd:" << fd << " with timeout:" << timeout);
	long res = System::DoSingleSelect(fd, timeout, forreading, !forreading);
	Trace("DoSingleSelect result:" << res << (res == 0 ? " was timeout" : ""));
	fHadTimeout = (0 == res);
	if (res < 0) {
		String logMsg;
		SystemLog::Error(logMsg
						 << "Pipe, select failed, error number: " << (long) errno
						 << " <" << SystemLog::LastSysError()
						 << "> return code " << res);
	}
	return res > 0;
}

bool Pipe::IsReadyForReading(long timeout)
{
	return IsReady(true, timeout);
}

bool Pipe::IsReadyForWriting(long timeout)
{
	return IsReady(false, timeout);
}

bool Pipe::ShutDownReading()
{
	StartTrace(Pipe.ShutDownReading);
	bool res = ShutDown(GetReadFd());
	fReadFd = -1;
	return res;
}

bool Pipe::ShutDownWriting()
{
	StartTrace(Pipe.ShutDownWriting);
	if ( fStream ) {
		fStream->flush();
	}
	bool res = ShutDown(GetWriteFd());
	fWriteFd = -1;
	return res;
}

#if defined(WIN32)
class PipeAcceptorCallBack : public AcceptorCallBack
{
public:
	PipeAcceptorCallBack()
		: fFd(-1)
	{ }
	~PipeAcceptorCallBack() { }
	void CallBack(Socket *socket) {
		StartTrace(Pipe.CallBack);
		if ( socket ) {
			socket->CloseOnDelete(false);
			fFd = socket->GetFd();
			Trace("fd:" << fFd);
			delete socket;
		}
	}
	long fFd;
};
#endif

bool Pipe::Open()
{
	StartTrace(Pipe.Open);
	bool bRet = false;
#if defined(WIN32)
	// use a local acceptor with INADDR_ANY
	PipeAcceptorCallBack *pCB = new PipeAcceptorCallBack();
	Acceptor aAcceptor("127.0.0.1", 0, 0, pCB);
	if ( aAcceptor.PrepareAcceptLoop() != 0 ) {
		SYSERROR("could not run Acceptor");
	} else {
		// get addr and port to connect to
		long lPort = aAcceptor.GetPort();
		Trace("port on which we accept:" << lPort);
		Connector aConnector("127.0.0.1", lPort);
		// run the connector now
		Socket *pWriteSocket = aConnector.Use();
		if ( pWriteSocket && pWriteSocket->SetNoDelay() ) {
			// first of all, tell not to close filedesc when deleting the Socket
			pWriteSocket->CloseOnDelete(false);
			// and finally run the Acceptor once to have a loopback connection to get the socket file descriptors
			if ( aAcceptor.RunAcceptLoop(true) == 0 ) {
				// lets get the ReadFd from the PipeAcceptorCallBack
				fReadFd = pCB->fFd;
				fWriteFd = pWriteSocket->GetFd();
				Trace("ReadFd:" << fReadFd << " WriteFd:" << fWriteFd);
				bRet = true;
			}
		}
	}
#else
	int fds[2];
	if (pipe(fds) >= 0)
//	if ( ::socketpair(AF_UNIX, SOCK_STREAM, 0, fds) >= 0 )
	{
		Trace("pipe() succeeded");
		fReadFd = fds[0];
		fWriteFd = fds[1];
		bRet = true;
	} else {
		String logMsg("pipe() creation failed:");
		logMsg << SystemLog::LastSysError();
		SystemLog::Error(logMsg);
		Trace(logMsg);
	}
#endif
	return bRet;
}
