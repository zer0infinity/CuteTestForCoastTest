/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SocketStream.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Socket.h"
#include "System.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#if !defined(WIN32)
#include <sys/socket.h> // used for send and recv
#include <errno.h>
#endif

void *iosITOSocket::operator new(size_t size, Allocator *a)
{
	StartTrace(iosITOSocket.operatornew);
	if (a) {
		Trace("allocating with allocator:[" << (long)a << "]");
		return a->Calloc(1, size);
	} else {
		Trace("using global new");
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void iosITOSocket::operator delete(void *d, Allocator *a)
{
	StartTrace(iosITOSocket.operatordelete);
	if (d) {
		if (a) {
			Trace("deleting with allocator:[" << (long)a << "]");
			a->Free(d);
		} else {
			Trace("deleting with global delete");
			::operator delete(d);
		}
	}
}
#endif

void iosITOSocket::operator delete(void *d)
{
	StartTrace(iosITOSocket.operatordelete);
	if (d) {
		Allocator *a = ((iosITOSocket *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		iosITOSocket::operator delete(d, a);
#else
		if (a) {
			Trace("deleting with allocator:[" << (long)a << "]");
			a->Free(d);
		} else {
			Trace("deleting with global delete");
			::operator delete(d);
		}
#endif
	}
}

iosITOSocket::iosITOSocket(Socket *s, long timeout, long sockbufsz, int mode )
	: fSocketBuf(s, timeout, sockbufsz, mode)
	, fAllocator(s->GetAllocator())
{
	init(&fSocketBuf);
}

//---- SocketStream -------------------------------------------------------------------
SocketStream::SocketStream(Socket *s, long timeout, long sockbufsz)
	: iosITOSocket(s, timeout, sockbufsz, ios::in | ios::out)
#if defined(ONLY_STD_IOSTREAM)
	, iostream(&fSocketBuf)
#endif
{
}

void SocketStreamBuf::SetTimeout(long t)
{
	if (fSocket) {
		fSocket->SetTimeout(t);
	}
}

long SocketStreamBuf::GetTimeout()
{
	return fSocket ? fSocket->GetTimeout() : 0L;
}

//---- TimeoutModifier -------------------------------------------------------------------
TimeoutModifier::TimeoutModifier(SocketStream *Ios, long timeout) : fStream(Ios)
{
	if (fStream) {
		fOriginalTimeout = fStream->rdbuf()->GetTimeout();
		fStream->rdbuf()->SetTimeout(timeout);
	}
}

TimeoutModifier::~TimeoutModifier()
{
	if (fStream) {
		fStream->rdbuf()->SetTimeout(fOriginalTimeout);
	}
}

//---- SocketStreamBuf -------------------------------------------------------------------
SocketStreamBuf::SocketStreamBuf(Socket *psocket, long timeout, long sockbufsz, int mode)
	: fReadBufStorage(mode &ios::in ? sockbufsz : 0L)
	, fWriteBufStorage(mode &ios::out ? sockbufsz : 0L)
	, fSocket(psocket)
	, fReadCount(0)
	, fWriteCount(0)
{
	SetTimeout(timeout);
	xinit();
}

SocketStreamBuf::SocketStreamBuf(const SocketStreamBuf &ssbuf)
	: fReadBufStorage(ssbuf.fReadBufStorage.Capacity())
	, fWriteBufStorage(ssbuf.fWriteBufStorage.Capacity())
	, fSocket(ssbuf.fSocket)
	, fReadCount(ssbuf.fReadCount)
	, fWriteCount(ssbuf.fWriteCount)
{
	int mode = 0;
	if (fReadBufStorage.Capacity() > 0) {
		mode |= ios::in;
	}
	if (fWriteBufStorage.Capacity() > 0) {
		mode |= ios::out;
	}
	xinit();
}

void SocketStreamBuf::xinit()
{
	//setb(start(), end(), 0); // holding area
	setg(0, 0, 0);
	setp(0, 0);
}

SocketStreamBuf::~SocketStreamBuf()
{
	sync(); // clear the buffer
	setg(0, 0, 0);
	setp(0, 0);
}

int SocketStreamBuf::overflow( int c )
{
	if (!pptr()) {
		setp(startw(), endw());	// reinitialize put area
	} else {
		Assert(pptr() - pbase() <=  fWriteBufStorage.Capacity());

		if (sync() == EOF) {	// write full buffer
			return EOF;
		}

		setp(startw(), endw());	// reinitialize put area
	}

	if (c != EOF && (pptr() < epptr())) // guard against recursion
#if defined(__GNUG__) && !defined(ONLY_STD_IOSTREAM) /* only gnu defined xput_char() std uses sputc() */
		xput_char(c);	// without check
#else
		sputc(c);
#endif
	return 0L;  // return 0 if successful
} // overflow

int SocketStreamBuf::underflow()
{
	long count = 0;

	if (gptr() < egptr()) { //(in_avail())
		// data is still available
		return (int)(unsigned char) * gptr();
	}

	if (pptr() - pbase() > 0) {
		if (sync() == EOF) {
			return EOF;
		}
	}

	if ((count = DoRead(startr(), fReadBufStorage.Capacity())) <= 0) {
		return EOF;    // might mean eofbit or failbit or badbit
	} else {
		AddReadCount( count );
	}

	setg(startr(), startr(), startr() + count);
	return (int)(unsigned char) * gptr();
} // underflow

int SocketStreamBuf::sync()
{
	long count;

	if ( ( (count = pptr() - pbase()) > 0 ) && ( DoWrite(pbase(), count) == EOF )) {
		return (EOF);
	}
	if (pptr()) {
		setp(startw(), endw());    // reinitialize put area
	}
	return 0; // no error
} // sync

SocketStreamBuf::pos_type SocketStreamBuf::seekpos(SocketStreamBuf::pos_type p, SocketStreamBuf::openmode mode)
{
	sync();
	return pos_type(0);
} // seekpos

SocketStreamBuf::pos_type SocketStreamBuf::seekoff(SocketStreamBuf::off_type of, SocketStreamBuf::seekdir dir, SocketStreamBuf::openmode mode)
{
	sync();
	return pos_type(0);
} // seekoff

void SocketStreamBuf::AddReadCount(long bytes)
{
	StartTrace(SocketStreamBuf.AddReadCount);
	fReadCount += bytes;
}

void SocketStreamBuf::AddWriteCount(long bytes)
{
	StartTrace(SocketStreamBuf.AddWriteCount);
	fWriteCount += bytes;
}

long SocketStreamBuf::DoWrite(const char *buf, long len)
{
	long bytesSent = 0;

	iostream *Ios = fSocket ? fSocket->GetStream() : 0; // neeed for errorhandling

	while (len > bytesSent &&  Ios && Ios->good()) {
		long nout = 0;
		if ( fSocket->IsReadyForWriting() ) {
			do {
				nout = send(fSocket->GetFd(), (char *)buf + bytesSent, len - bytesSent, 0);
			} while (nout < 0 && System::SyscallWasInterrupted());
			if (nout > 0) {
				bytesSent += nout;
				continue;
			}
		} else if (fSocket->HadTimeout()) {
			Ios->clear(ios::failbit);
			break;
		}
		String logMsg("socket on send: ");
		logMsg << fSocket->GetFd()
			   << " failed - socket error number "
			   << (long) SOCKET_ERROR
			   << " <" << SysLog::LastSysError() << ">" << " transmitted: " << bytesSent;

		SysLog::Error(logMsg);
		Ios->clear(ios::badbit);
	}

	if ( bytesSent > 0 ) {
		AddWriteCount( bytesSent );
	}

	return (Ios && Ios->good()) ? bytesSent : EOF;
}

long SocketStreamBuf::DoRead(char *buf, long len) const
{
	long bytesRead = EOF; // assume EOF is negative

	if (fSocket) {
		iostream *Ios = fSocket->GetStream();
		if (fSocket->IsReadyForReading() ) {
			do {
				bytesRead = recv(fSocket->GetFd(), buf, len, 0);
			} while (bytesRead < 0 && System::SyscallWasInterrupted());

			if ( bytesRead < 0 ) {
				String msg("Socket Error: <");
				msg << (long)errno << ">=" << SysLog::SysErrorMsg(errno);
				SysLog::Error(msg);
				Ios->clear(ios::badbit);
			} else if ( bytesRead == 0 ) {
				String msg("0 bytes read from ");
				msg << fSocket->GetFd();
				SysLog::Info(msg);
				// socket is closed, stream recognizes this via
				// streambuf::underflow() returning eof, if no more bytes are available
			}
		} else {
			Ios->clear(fSocket->HadTimeout() ? ios::failbit : ios::badbit);
		}
	}
	return bytesRead;
}

ostream  &operator<<(ostream &os, SocketStreamBuf *ssbuf)
{
	StartTrace(ostream.socketstreambuf);
	if (ssbuf) {
		const int bufSz = 4096;
		char buf[bufSz];
		long totBytesRead = 0;

		long bytesRead = 0;
		do {
			bytesRead = ssbuf->DoRead(buf, bufSz);
			if ( bytesRead > 0 ) {
				totBytesRead += bytesRead;
				SubTrace(content, buf);
				os.write(buf, bytesRead);
			} else if (bytesRead < 0) {
				String logMsg("Socket error on recv: ");
				logMsg << (long) errno;
				SysLog::Error( logMsg );
			} // if
			else {
				SysLog::Error( "Socket closed on recv: " );
			}
		} while ( bytesRead > 0 );

		if ( totBytesRead > 0 ) {
			ssbuf->AddReadCount( totBytesRead );
		}
		os.flush();
	}
	return os;
}
