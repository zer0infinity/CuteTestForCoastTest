/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Pipe_H
#define _Pipe_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOString.h"
#include "PipeStream.h"

//--- Pipe --------------------------------------------------------------------------
/*! represents a read/write pipe end point that is connected
It is a <B>communication end point</B> for read/write (anonymous) pipes.
It has an api that allows the manipulation of the pipe and the generation of an iostream which is a PipeStream. */
class EXPORTDECL_FOUNDATION Pipe
{
public:
	/*! constructor sets the variables
		\param readFd the pipe filedescriptor for the pipe reading endpoint
		\param writeFd the pipe filedescriptor for the pipe writing endpoint
		\param doClose flag that defines the behaviour of the destructor with regard to closing the pipe <BR>doClose == <B>true</B> closes the pipedescriptors in the destructor <BR>doClose == <B>false</B> leaves the filedescriptors open
		\param timeout timeout for pipe communication over the stream (default is 0.5 seconds)
		\pre pipe is a valid connected pipe file descriptor
		\post a usable pipe object which allocates the stream on demand */
	Pipe(int readFd, int writeFd, bool doClose = true, long timeout = 500 /* half a second */);
	/*! constructor creating an anonymous pipe with pipe()
		\note you have to shutdown both ends or close the filedescriptors, the destructor won't do it for you!
		\note pipe() might fail, in this case the Pipe object is unusable
		\note retrieving a iostream where both endpoints are connected to the same pipe() is not very usable. usually you have to fork a sub-process that takes one end of the pipe and closes the other one. the original process will do that the other way around. Then the processes can communicate via the pipe.
		For attaching stdin and stdout two such Pipes might be needed that later are used on one side with <BR>
		<PRE>Pipe(pin.GetReadFd(),pout.GetWriteFd()); </PRE><BR>
		where pin and out are constructed with this constructor.
		\param timeout timeout for pipe communication over the stream (default is 0.5 seconds)
		\post a usable pipe object which allocates the stream on demand */
	Pipe(long timeout = 500 /* half a second */);

	/*! destructor
		\pre pipe contains a valid connected pipe file descriptor and potentially a stream object
		\post the stream is deleted and the pipefd is closed if fDoClose is true */
	~Pipe();

	/*! returns an iostream the iostream is allocated once and then cached
		\pre pipe contains at least one valid connected pipe file descriptor and potentially a stream object
		\post the stream is creaded if necessary, cached and returned
		\return pointer to the PipeStream object */
	PipeStream *GetStream();

	/*! accessor to the pipes reading fd, negative means closed
		\return filedescriptor of the read side */
	long GetReadFd();

	/*! accessor to the pipes writing fd, negative means closed
		\return filedescriptor of the write side */
	long GetWriteFd();

	/*! blocks the calling thread until the pipe is ready to accept data or the timeout expires
		if timeout is 0 it returns immediately, if -1 it blocks infinitely, if -2 it uses GetTimeout
		\param timeout defines the maximum waiting time in milliseconds
		\return true if the pipe can be written, false in case there was a timeout - check using GetTimeout() - or a socket error */
	bool IsReadyForWriting(long timeout = -2);

	/*! IsReadyForReading blocks the calling thread until the pipe is ready to provide data or the timeout expires
		if tiemout is 0 it returns immediately, if -1 it blocks infinitely, if -2 it uses GetTimeout
		\param timeout defines the maximum waiting time in milliseconds
		\return true if there is something to read, false in case there was a timeout - check using GetTimeout() - or a socket error */
	bool IsReadyForReading(long timeout = -2);

	/*! shuts the reading side down. The pipe can still be used for writing
		\return true if the close succeeded */
	bool ShutDownReading();

	/*! shuts the writing side down. The pipe can still be used for reading
		\return true if the close succeeded */
	bool ShutDownWriting();

	/*! sets the fDoClose flag
		\param doClose flag that defines the behaviour of the destructor with regard to closing the pipe
		doClose == <B>true</B> closes the pipedescriptors in the destructor
		doClose == <B>false</B> leaves the filedescriptors open */
	void CloseOnDelete(bool doClose = true)	{
		fDoClose = doClose;
	}

	/*! set the default timeout for pipe communication in milliseconds
		\param timeout defines the maximum waiting time in milliseconds
		\return default timeout which was set before */
	long SetTimeout(long timeout) 	{
		long lOldTimeout = fTimeout;
		fTimeout = timeout;
		return lOldTimeout;
	}

	/*! get the default timeout for pipe communication over streams
		\return current default timeout */
	long GetTimeout()				{
		return fTimeout;
	}

	/*! query if IsReadyxxx resulted in a timeout
		\return true if it was a timeout, false otherwise */
	bool HadTimeout() {
		return fHadTimeout;
	}

protected:
	/*! create the pipe
		\return true if both endpoints were created successfully */
	bool Open();

	/*! DoMakeStream allocates a pipe iostream, overwrite these method if you need something specific
		\return PipeStream object for reading and writing
		\note The PipeStream object will be deleted in the destructor */
	PipeStream *DoMakeStream();

	/*! IsReady bottleneck routine
		if tiemout is 0 it returns immediately, if -1 it blocks infinitely, if -2 it uses GetTimeout
		\param forreading flag passed to select if we want to check the socket for reading or writing
		\param timeout defines the maximum waiting time in milliseconds
		\return true if the corresponding socketfd is ready for reading or writing, false in case there was a timeout - check using GetTimeout() - or a socket error*/
	bool IsReady(bool forreading, long timeout = -2);

	/*! bottleneck routine that closes the given socketfd
		\param fd the filedescriptor to close
		\return false if the socketfd is < 0 or closeSocket failed, true in case the close was successful */
	bool ShutDown(long fd);

	//! the pipe file descriptors used for this pipe
	long fReadFd, fWriteFd;

	//! the iostream used to read and write to the pipe ( default is PipeStream )
	PipeStream *fStream;

	//! flag that determines whether the pipe will be closed in the destructor
	bool fDoClose;

	//! default timeout for pipe communication over streams
	long fTimeout;

	//! mark if select delivered a timeout
	bool fHadTimeout;

private:
	/*! this standard operator is private to block its use
		if it is not declared, the C++ compiler generates it automatically
		it is not implemented don't use it */
	Pipe(const Pipe &s);

	/*! this standard operator is private to block its use
		if it is not declared, the C++ compiler generates it automatically
		it is not implemented don't use it */
	Pipe &operator=(const Pipe &s);
};

#endif
