/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PipeExecutor_H
#define _PipeExecutor_H

#include "Anything.h"

//---- forward declaration -----------------------------------------------
class Pipe;

//---- PipeExecutor ----------------------------------------------------------
//!execute a program and connect its stdio to iostreams
/*! this is useful for implementing CGI and all other kinds of delegation
to existing programs.
CAUTION: many programs wait until stdin is closed before producing their
complete output, therfore use ShutDownWriting() to ensure the corresponding
pipe is closed. */
class PipeExecutor
{
public:
	//--- constructors
	/*! Create an execution endpoint that calls program cmd with environment env
		\param cmd the path to the program called
		\param env the environment to be used for calling
		\param wd the working directory to be used for calling
		\param lExecTimeout execution timeout passed to underlying Pipe [ms], if no output can be read within the givin time, the pipe(s) will be closed.
		\param bOpenStreamForStderr if set to true, stderr (fd=2) output will be captured. Call GetStderr() to get this stream. */
	PipeExecutor(const String &cmd, Anything env, const char *wd = ".", long lExecTimeout = 1000L, bool bOpenStreamForStderr = false);
	virtual ~PipeExecutor();

	//--- public api
	/*! Get iostream for reading/writing to called program, connected to programs stdin(0) and stdout(1)
		\return returns 0 if something went wrong */
	std::iostream *GetStream();

	/*! Get istream for reading stderr output of called program, only valid if bOpenStreamForStderr was set to true in ctor.
		\return returns 0 if something went wrong or bOpenStreamForStderr was set to false in ctor */
	std::istream *GetStderr();

	/*! Provide hook for clients to switch off stdin, so that they can
		read safely from stdout of the client program, otherwise
		things could block.<BR>
		<B>NOTE:</B> this is not fail safe (yet). writing might
		block, because the child process is blocked in writing
		as well and we haven't read anything yet and the OS pipe
		buffer is full. So whenever you write large portions
		of data to a child process try to read in-between if
		you get timeouts.
		\return true in case the write side could be closed successfully or false if something went wrong */
	bool ShutDownWriting();

	/*! Do the real thing, interpret cmd and look for program that
		corresponds to cmd first element, only use blank as delimiter in cmd
		\note no complete shell interpretation, eg. globbing!
		\return true if command execution was successful */
	bool Start();

	/*! Kill executable nicely first if not already terminated and wait for its termination.
		\note if tryhard is set to false this call blocks until child process dies
		\param signal kill signal to be used, if this fails we always SIGKILL it
		\param tryhard If set to false, we wait on child termination forever. Use true, to kill the bastard now.
		\return return the exit status of the child process or -1 if something went wrong */
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
	bool fOpenStreamForStderr;
	Pipe *fStderr;
	long fTimeout;

	class CgiEnv
	{
	public:
		CgiEnv(const Anything &env, Allocator *a);
		~CgiEnv();
#if defined(WIN32)
		void* GetEnv(int &length);
#else
		char** GetEnv();
#endif
	private:
		Allocator *fAlloc;
#if defined(WIN32)
		String fEnv;
#else
		Anything fEnv;
		char** fEnvPtrs;
#endif
	};

	class CgiParam
	{
	public:
		CgiParam(Anything param, Allocator *a);
		~CgiParam();
		char **GetParams();
	private:
		Allocator *fAlloc;
		char **fParams;
	};
};

#endif

