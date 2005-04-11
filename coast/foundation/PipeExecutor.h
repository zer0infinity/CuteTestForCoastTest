/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeExecutor_H
#define _PipeExecutor_H

#include "config_foundation.h"
#include "Anything.h"

//---- forward declaration -----------------------------------------------
class EXPORTDECL_FOUNDATION Pipe;

//---- PipeExecutor ----------------------------------------------------------
//!execute a program and connect its stdio to iostreams
//! this is useful for implementing CGI and all other kinds of delegation
//! to existing programs.
//! CAUTION: many programs wait until stdin is closed before producing their
//! complete output, therfore use CloseWriteStream() to ensure the corresponding
//! pipe is closed.

class EXPORTDECL_FOUNDATION PipeExecutor
{
public:
	//--- constructors
	//!create an execution endpoint that calls program cmd with environment
	//! env.
	//! \param cmd the path to the program called
	//! \param env the environment to be used for calling
	//! \param wd the working directory to be used for calling
	//! \param to timeout passed to underlying Pipe s
	//! \param redirectstderr also capture stderr (fd=2) with a Pipe
	PipeExecutor(const String &cmd, Anything env, const char *wd = ".", long to = 1000L, bool redirectstderr = false);
	~PipeExecutor();

	//--- public api
	//!return iostream for reading/writing to called program, connected to programs stdin(0) and stdout(1)
	//! \return returns 0 if something went wrong
	iostream *GetStream();

	//!return istream for reading stderr output of called program
	//! \return returns 0 if something went wrong
	istream *GetStderr();

	//!provide hook for clients to switch off stdin, so that they can
	//! read safely from stdout of the client program, otherwise
	//! things could block.<BR>
	//! <B>NOTE:</B> this is not fail safe (yet). writing might
	//! block, because the child process is blocked in writing
	//! as well and we haven't read anything yet and the OS pipe
	//! buffer is full. So whenever you write large portions
	//! of data to a child process try to read in-between if
	//! you get timeouts.
	bool ShutDownWriting();

	//!do the real thing, interpret cmd and look for program that
	//! corresponds to cmd first element, only use blank as delimiter in cmd
	//! no complete shell interpretation
	bool Start();

	//!wait for the bastard and kill if not dead bastard and wait for its termination
	//! \param signal the signal to be used, if failing tries always SIGKILL
	//! automagically
	//! \param tryhard if true, do not wait on wait AND kill the bastard
	//! <B>caution:</B> if false this call blocks until child process dies
	//! \return return the exit status of the child process or -1 if something is wrong
	long TerminateChild(int signal = 15 /*SIGTERM*/, bool tryhard = true);

protected:
	friend class PipeExecutorTest;
	PipeExecutor()
		: fPipe(0), fChildPid(-1), fStderr(0) {} // empty default ctor for testing purposes only
	//--- subclass api
	//!do the real work

	//!param: param - contains stuff to call at position 0 and other
	//parameters as following entries.
	//!param: env - the well-known environment, useful for CGI programs...
	bool ForkAndRun(Anything param, Anything env);

	//!tokenize command string to a list of strings returned in param
	bool ParseParam(String cmd, Anything &param);

	//!auxiliary function setting up the file descriptors for the forked child process
	//! closes the required endpoints
#if !defined(WIN32)
	bool SetupChildPipes(Pipe &sin, Pipe &sout);
#endif
	//--- member variables declaration
	//names correspond to the view of the child process.
	Pipe *fPipe;
	int fChildPid;
	String fCommand;
	Anything fEnv;
	String fWorkingDir;
	bool fRedirectStderr;
	Pipe *fStderr;
	long fTimeout;

	class EXPORTDECL_FOUNDATION CgiEnv
	{
	public:
		CgiEnv(Anything &env, Allocator &a);
		~CgiEnv();
		char **GetEnv();
	private:
		Allocator	&fAlloc;
		char **fEnv;
	};

	class EXPORTDECL_FOUNDATION CgiParam
	{
	public:
		CgiParam(Anything param, Allocator &a);
		~CgiParam();
		char **GetParams();
	private:
		Allocator	&fAlloc;
		char **fParams;
	};
};

#endif

