/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PipeStream.h"
#include "SystemLog.h"
#include "SystemBase.h"
#include "Pipe.h"
#include "Dbg.h"
#include "Socket.h"

using namespace Coast;

#include <errno.h>
#if defined(WIN32)
#include <io.h>
#endif

//---- PipeStream -------------------------------------------------------------------
PipeStream::PipeStream(Pipe *s, long timeout, long sockbufsz)
	: iosCoastPipe(s, timeout, sockbufsz, std::ios::in | std::ios::out)
	, std::iostream(&fPipeBuf)
{
}

//---- PipeTimeoutModifier -------------------------------------------------------------------
PipeTimeoutModifier::PipeTimeoutModifier(PipeStream *ioStream, long timeout)
	: fStream(ioStream)
{
	fOriginalTimeout = fStream->rdbuf()->GetTimeout();
	fStream->rdbuf()->SetTimeout(timeout);
}

PipeTimeoutModifier::~PipeTimeoutModifier()
{
	fStream->rdbuf()->SetTimeout(fOriginalTimeout);
}

//---- PipeStreamBuf -------------------------------------------------------------------
PipeStreamBuf::PipeStreamBuf(Pipe *myPipe, long timeout, long sockbufsz, int mode)
	: fReadBufStorage(mode &std::ios::in ? sockbufsz : 0L)
	, fWriteBufStorage(mode &std::ios::out ? sockbufsz : 0L)
	, fPipe(myPipe)
	, fTimeout(0)
	, fReadCount(0)
	, fWriteCount(0)
{
	StartTrace(PipeStreamBuf.PipeStreamBuf);
	if (fPipe) {
		// set to non-blocking IO
		Socket::SetToNonBlocking(fPipe->GetReadFd());
		Socket::SetToNonBlocking(fPipe->GetWriteFd());
		fPipe->SetTimeout(timeout);
	}
	xinit();
}

PipeStreamBuf::PipeStreamBuf(const PipeStreamBuf &ssbuf)
	: fReadBufStorage(ssbuf.fReadBufStorage.Capacity())
	, fWriteBufStorage(ssbuf.fWriteBufStorage.Capacity())
	, fPipe(ssbuf.fPipe)//lint !e1554
	, fTimeout(ssbuf.fTimeout)
	, fReadCount(ssbuf.fReadCount)
	, fWriteCount(ssbuf.fWriteCount)
{//lint !e1538
	StartTrace1(PipeStreamBuf.PipeStreamBuf, "copy");
	int mode = 0;
	if (fReadBufStorage.Capacity() > 0) {
		mode |= std::ios::in;
	}
	if (fWriteBufStorage.Capacity() > 0) {
		mode |= std::ios::out;
	}
	xinit();
}//lint !e1566//lint !e550//lint !e438

void PipeStreamBuf::xinit()
{
	StartTrace(PipeStreamBuf.xinit);
	//setb(start(), end(), 0); // holding area
	setg(0, 0, 0);
	setp(0, 0);
}

PipeStreamBuf::~PipeStreamBuf()
{
	StartTrace(PipeStreamBuf.~PipeStreamBuf);
	PipeStreamBuf::sync(); // clear the buffer
	setg(0, 0, 0);
	setp(0, 0);
}//lint !e1579

int PipeStreamBuf::overflow( int c )
{
	StartTrace1(PipeStreamBuf.overflow, "char [" << (char)c << "] val: " << (long)c);
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
		sputc(c);
	return 0L;  // return 0 if successful
}

int PipeStreamBuf::underflow()
{
	StartTrace(PipeStreamBuf.underflow);
	int count;

	if (gptr() < egptr()) { //(in_avail())
		// data is still available
		return (int)(unsigned char) * gptr();
	}

//	if (pptr()-pbase() > 0)
//	{
//		if (sync()==EOF)
//		    return EOF;
//	}

	if ((count = DoRead(startr(), fReadBufStorage.Capacity())) <= 0) {
		return EOF;    // might mean eofbit or failbit or badbit
	}

	setg(startr(), startr(), startr() + count);
	return (int)(unsigned char) * gptr();
}

int PipeStreamBuf::sync()
{
	StartTrace(PipeStreamBuf.sync);
	long count;

	if ( ( (count = pptr() - pbase()) > 0 ) && ( DoWrite(pbase(), count)) == EOF ) {
		return (EOF);
	}
	if (pptr()) {
		setp(startw(), endw());	// reinitialize put area
//!@FIXME PS this is not always OK for pipes, is it?
	}
	return 0; // no error
}

PipeStreamBuf::pos_type PipeStreamBuf::seekpos(PipeStreamBuf::pos_type p, PipeStreamBuf::openmode mode)
{
	StartTrace1(PipeStreamBuf.seekpos, (long)p);
	sync();
	return pos_type(0);
}

PipeStreamBuf::pos_type PipeStreamBuf::seekoff(PipeStreamBuf::off_type of, PipeStreamBuf::seekdir dir, PipeStreamBuf::openmode mode)
{
	StartTrace1(PipeStreamBuf.seekoff, (long)of);
	sync();
	return pos_type(0);
}

long PipeStreamBuf::DoWrite(const char *bufPtr, long bytes2Send)
{
	StartTrace1(PipeStreamBuf.DoWrite, "bytes2Send:" << bytes2Send);
	long bytesSent = 0;
	std::iostream *Ios = fPipe ? fPipe->GetStream() : 0; // neeed for errorhandling
	long wfd = fPipe ? fPipe->GetWriteFd() : -1;

	if (wfd >= 0) {
		// this also ensures wfd is valid
		while ( bytes2Send > bytesSent && Ios && Ios->good() ) {
			long nout = 0;
			do {
				nout = Socket::write(wfd, (char *)bufPtr + bytesSent, bytes2Send - bytesSent);
				Trace("bytes written:" << nout);
			} while (nout < 0 && System::SyscallWasInterrupted() && fPipe->IsReadyForWriting());//lint !e613
			if (nout > 0) {
				bytesSent += nout;
				Trace("sent:" << bytesSent << " bytes");
				continue;
			}
			if (fPipe->HadTimeout()) {//lint !e613
				Ios->clear(std::ios::failbit);
				Trace("timeout");
				break;
			}
			String logMsg("write on pipe: ");
			logMsg << (long)wfd << " failed <" << SystemLog::LastSysError() << ">" << " transmitted: " << bytesSent;
			SystemLog::Error(logMsg);
			Ios->clear(std::ios::badbit);
		}
	} else if (Ios) {
		Ios->clear(std::ios::badbit | std::ios::eofbit | std::ios::failbit);
	}
	return (Ios && Ios->good()) ? bytesSent : EOF;
}

long PipeStreamBuf::DoRead(char *buf, long len) const
{
	StartTrace1(PipeStreamBuf.DoRead, "buflen:" << len);
	long bytesRead = EOF;

	if (fPipe) {
		std::iostream *ioStream = fPipe->GetStream();
		if ( fPipe->IsReadyForReading() ) {
			do {
				bytesRead = Socket::read(fPipe->GetReadFd(), buf, len);
				Trace("bytes read:" << bytesRead);
			} while (bytesRead < 0 && System::SyscallWasInterrupted());

			if ( bytesRead < 0 ) {
				ioStream->clear(std::ios::badbit);
				Trace("something went wrong");
			} else if (0 == bytesRead) {
				ioStream->clear(std::ios::eofbit);
				Trace("nothing read, eof");
			}
		} else {
			Trace("timeout or nothing to read");
			ioStream->clear(fPipe->HadTimeout() ? std::ios::failbit : std::ios::badbit);
		}
	}
	Trace("bytesRead:" << bytesRead);
	return bytesRead;
}

std::ostream  &operator<<(std::ostream &os, PipeStreamBuf *ssbuf)
{
	StartTrace(PipeStreamBuf.operator << );
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
				Trace("read another " << bytesRead << " bytes");
			} else if (bytesRead < 0) {
				String logMsg("Pipe error on read: ");
				logMsg << (long) errno;
				SystemLog::Error( logMsg );
			} // if
			else {
				SystemLog::Error( "Pipe closed on read: " );
			}
		} while ( bytesRead > 0 );

		os.flush();
		Trace("total bytes read: " << totBytesRead);
		(void) totBytesRead;
	}
	return os;
}
