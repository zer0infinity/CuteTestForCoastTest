/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PipeExecutor.h"
#include "SystemBase.h"
#include "SystemFile.h"
#include "SystemLog.h"
#include "Pipe.h"
#include "TimeStamp.h"
#include "Tracer.h"
#include <cstring>
#if !defined(WIN32)
#include <sys/wait.h>
#else
#include <process.h>
#include <io.h>
#endif
#include <fcntl.h>
#include <errno.h>
#if defined(sun) || defined(__APPLE__)
#include <signal.h>	/* for kill() */
#endif

using namespace Coast;
PipeExecutor::PipeExecutor(const String &cmd, Anything env, const char *wd, long lExecTimeout, bool bOpenStreamForStderr)
	: fPipe(0)
	, fChildPid(0)
	, fCommand(cmd)
	, fEnv(env)
	, fWorkingDir(wd)
	, fOpenStreamForStderr(bOpenStreamForStderr)
	, fStderr(0)
	, fTimeout(lExecTimeout)
{
	StartTrace(PipeExecutor.PipeExecutor);
	Trace("executing cmd:[" << cmd << "] with path:[" << wd << "]");
	TraceAny(env, "Environment is:");
}

PipeExecutor::~PipeExecutor()
{
#if defined(WIN32)
	if (fChildPid > 0) {
		_cwait(NULL, fChildPid, NULL);    // clean up zombies
	}
#else
	if (fChildPid > 0) {
		waitpid(fChildPid, 0, 0);    // clean up zombies
	}
#endif
	if (fPipe) {
		delete fPipe;    // this ultimatively closes stream and pipe
	}

	if (fStderr) {
		delete fStderr;
	}
}

std::iostream *PipeExecutor::GetStream()
{
	StartTrace(PipeExecutor.GetStream);
	return fPipe ? fPipe->GetStream() : 0;
}

std::istream *PipeExecutor::GetStderr()
{
	StartTrace(PipeExecutor.GetStderr);
	return fStderr ? fStderr->GetStream() : 0;
}

bool PipeExecutor::Start()
{
	StartTrace(PipeExecutor.Start);
	Anything parm;
	if (ParseParam(fCommand, parm)) {
		return ForkAndRun(parm, fEnv);
	}
	return false;
}

#if defined(WIN32)
long PipeExecutor::TerminateChild(int termSignal, bool tryhard)
{
	StartTrace(PipeExecutor.TerminateChild);
	Trace("terminating child pid =" << static_cast<long>(fChildPid) << " with signal = " << static_cast<long>(termSignal) << " trying " << (tryhard ? "hard" : "soft"));
	if (fChildPid > 0) {
		DWORD exitcode;
		for (;;) {
			if ( GetExitCodeProcess((HANDLE)fChildPid, &exitcode) != 0 ) {
				Trace("exitcode:" << exitcode);
				if (exitcode == STILL_ACTIVE) {
					if ( tryhard ) {
						// we cannot use ExitProcess here because we need to specify a PID
						TerminateProcess((HANDLE)fChildPid, 0);
					}
					continue;
				}
				break;
			}
		}
		CloseHandle((HANDLE)fChildPid);
		return exitcode;
	}
	return -1; // something went wrong
}
#else
long PipeExecutor::TerminateChild(int termSignal, bool tryhard)
{
	StartTrace(PipeExecutor.TerminateChild);
	Trace("terminating child pid =" << static_cast<long>(fChildPid) << " with signal = " << static_cast<long>(termSignal) << " trying " << (tryhard ? "hard" : "soft"));
	while ( fChildPid > 0 ) {
		int wstat = 1;
		int wres = waitpid(fChildPid, &wstat, (tryhard ? WNOHANG : 0) | WUNTRACED);
		Trace("waitpid delivers : " << long(wres) << " and stat " << long(wstat));
		if (wres < 0) {
			Trace("errno after wait:" << static_cast<long>(errno) << " " << SystemLog::SysErrorMsg(errno));
		}
		if (wres == fChildPid) {
			Trace("found child");
			return WEXITSTATUS(wstat); // already dead!
		} else if (wres <= 0 && tryhard) {
			if ( kill(fChildPid, termSignal) < 0 ) {
				Trace("kill failed, errno =" << static_cast<long>(errno) << " " << SystemLog::SysErrorMsg(errno));
				if (errno == ESRCH) {
					// already gone and removed?
				} else if (errno == EPERM) {
					// something wrong
				}
				return -1;
			} else {
				// wait again, only try hard if we didn't already send SIGKILL
				// the bastard isn't dead or not there
				// shoot at him
				termSignal = SIGKILL;
			}
		}
		// give some time to react, 50ms
		System::MicroSleep(50000);
	}
	return -1; // something went wrong
}
#endif
bool PipeExecutor::ShutDownWriting()
{
	StartTrace(PipeExecutor.ShutDownWriting);
	if (fPipe) {
		return fPipe->ShutDownWriting();
	}
	return false;
}

bool PipeExecutor::ParseParam(String cmd, Anything &param)
{
	StartTrace(PipeExecutor.ParseParam);
	Trace("Cmd:<" << cmd << ">");
	StringTokenizer2 st(cmd, " \t\n");
	param = Anything(); // make it empty first;
	String next;
	while (st.NextToken(next)) {
		param.Append(next);
	}
	TraceAny(param, "Params: ");
	return true;
}

PipeExecutor::CgiEnv::CgiEnv(const Anything &env, Allocator *a)
	: fAlloc(a)
	, fEnv(a)
#if !defined(WIN32)
	, fEnvPtrs(0)
#endif
{
#if defined(WIN32)
	long i = 0;
	for (long sz = env.GetSize(); i < sz; ++i) {
		fEnv.Append(env.SlotName(i)).Append("=").Append(env[i].AsString()).Append('\0');
	}
	fEnv.Append('\0');
#else
	// tracing not fork-safe
	fEnvPtrs = static_cast<char **>(fAlloc->Calloc(env.GetSize() + 1, sizeof(char *)));
	if (fEnvPtrs) {
		long i = 0;
		for (long sz = env.GetSize(); i < sz; ++i) {
			String s = env.SlotName(i);
			s << "=" << env[i].AsCharPtr();
			fEnv[i] = s;
			fEnvPtrs[i] = (char *)fEnv[i].AsCharPtr(0);
		}
		fEnvPtrs[i] = 0;
	}
#endif
}

PipeExecutor::CgiEnv::~CgiEnv() {
#if !defined(WIN32)
	fAlloc->Free(fEnvPtrs);
#endif
}

#if defined(WIN32)
void *PipeExecutor::CgiEnv::GetEnv(int &length) {
	length = fEnv.Length();
	return reinterpret_cast<void*>(const_cast<char*>(fEnv.cstr()));
}
#else
char **PipeExecutor::CgiEnv::GetEnv() {
	return fEnvPtrs;
}
#endif

PipeExecutor::CgiParam::CgiParam(Anything param, Allocator *a)
	: fAlloc(a)
	, fParams(0)
{
	// tracing not fork-safe
	fParams = static_cast<char **>(fAlloc->Calloc(param.GetSize() + 1, sizeof(char *)));
	if (fParams) {
		long i = 0;
		for (long sz = param.GetSize(); i < sz; ++i) {
			fParams[i] = (char *)param[i].AsCharPtr(0);
		}
		fParams[i] = 0;
	}
}

PipeExecutor::CgiParam::~CgiParam()
{
	fAlloc->Free(fParams);
}

char **PipeExecutor::CgiParam::GetParams()
{
	return fParams;
}

#if !defined(WIN32)
bool PipeExecutor::SetupChildPipes(Pipe &stdChildPipeIn, Pipe &stdChildPipeOut)
{
	// tracing not fork-safe
	bool ok = true;
	stdChildPipeIn.ShutDownWriting();
	stdChildPipeOut.ShutDownReading();

	ok = (0 == dup2(stdChildPipeIn.GetReadFd(), 0)) && ok;
	ok = (1 == dup2(stdChildPipeOut.GetWriteFd(), 1)) && ok;
	if (fStderr) {
		fStderr->ShutDownReading();
		ok = (2 == dup2(fStderr->GetWriteFd(), 2)) && ok;
	}
	// do not close the following descriptors across a spawn or exec call
	ok = (0 == fcntl(0, F_SETFD, ~FD_CLOEXEC)) && ok;
	ok = (0 == fcntl(1, F_SETFD, ~FD_CLOEXEC)) && ok;
	ok = (0 == fcntl(2, F_SETFD, ~FD_CLOEXEC)) && ok;
	return ok;
}
#endif

bool PipeExecutor::ForkAndRun(Anything parm, Anything env)
{
	StartTrace(PipeExecutor.ForkAndRun);
	if (fPipe) {
		return false; // we can do it only once
	}
	// check if executable can be found and is accessible
	if ( System::IO::access(parm[0L].AsCharPtr(), X_OK) == 0 ) {
		// should implement search path here? better within CGI module
		Pipe inp(fTimeout);
		Pipe outp(fTimeout);
		if (inp.GetReadFd() >= 0 && inp.GetWriteFd() >= 0 &&
			outp.GetReadFd() >= 0 && outp.GetWriteFd() >= 0 ) {
			if (fOpenStreamForStderr) {
				fStderr = new Pipe(fTimeout);
			}

			Allocator *alloc = Coast::Storage::Current();
			TraceAny(env, "env");
			CgiEnv cgiEnv(env, alloc);
			TraceAny(parm, "parm");
			CgiParam cgiParams(parm, alloc);

#if !defined(WIN32)
			// now fork, and clean up
			String strTime( TimeStamp::Now().AsStringWithZ() );
			if (0 == (fChildPid = System::Fork())) {
				// I am the child
				// careful - only fork-safe stuff allowed here
				// Tracing is probably not fork-safe
				if (SetupChildPipes(inp, outp) && System::ChangeDir(fWorkingDir)) {
					// do all allocation from parent so that we can use vfork
					// on platforms where it is desirable
					char **p = cgiParams.GetParams();
					char **e = cgiEnv.GetEnv();
					// execve will not return on success, it will return -1 in case of a failure
					execve(p[0], p, e);//lint !e613
				}
				// oops we failed. terminate the process
				// don't use strings - allocators not fork-safe
				int iError( System::GetSystemError() );
				char buff[1024];
				int len = snprintf(buff, sizeof(buff),
								   "%s exec of program %s in dir %s failed with code %d: %s\n",
								   (const char *)strTime,
								   parm[0L].AsCharPtr(),
								   (const char *)fWorkingDir,
								   iError,
								   strerror(iError));
				ssize_t written = write(1, buff, len);
				written = write(2, buff, len);
				(void)written;
				SystemLog::Error(buff);
				_exit(EXIT_FAILURE); // point of no return for child process.....
			} else if (fChildPid > 0) {
				Trace("forked pipe-exec-child with pid:" << fChildPid);
				inp.ShutDownReading(); // we write to it
				outp.ShutDownWriting();// we read from it
				if (fStderr) {
					fStderr->ShutDownWriting();
					fStderr->CloseOnDelete();
				}
				fPipe = new Pipe(outp.GetReadFd(), inp.GetWriteFd(), true, fTimeout);
			} else {
				// we failed to fork
				String msg(strTime);
				msg.Append(" fork failed for child pid ").Append( static_cast<long>(fChildPid) ).Append(" with sysmsg [").Append( SystemLog::LastSysError() ).Append(']');
				SystemLog::Error(msg);
			}
#else
			// now create new process
			{
				BOOL fRet;
				LPCSTR lpApplicationName = NULL;
				LPTSTR lpCommandLine;
				DWORD dwCreationFlags = NORMAL_PRIORITY_CLASS;
				STARTUPINFO aStartupInfo;
				PROCESS_INFORMATION aProcessInformation;
				SECURITY_ATTRIBUTES sa;

				sa.nLength = sizeof (SECURITY_ATTRIBUTES);
				sa.lpSecurityDescriptor = NULL;
				sa.bInheritHandle = false;

				memset(&aProcessInformation, 0, sizeof (PROCESS_INFORMATION));

				memset(&aStartupInfo, 0, sizeof (STARTUPINFO));
				aStartupInfo.cb = sizeof (STARTUPINFO);
				aStartupInfo.dwFlags = STARTF_USESTDHANDLES;

				// we use only writing side for the executing process
				aStartupInfo.hStdInput  = (void *)inp.GetReadFd();
				aStartupInfo.hStdOutput = (void *)outp.GetWriteFd();
				if (fStderr) {
					aStartupInfo.hStdError  = (void *)fStderr->GetWriteFd();
				} else {
					aStartupInfo.hStdError  = GetStdHandle(STD_ERROR_HANDLE);
				}

				lpCommandLine = const_cast<char*>(fCommand.cstr());
				int length = 0;
				fRet = CreateProcess(lpApplicationName, lpCommandLine, &sa, &sa,
						true, dwCreationFlags, cgiEnv.GetEnv(length), (const char *)fWorkingDir, &aStartupInfo, &aProcessInformation);
				if (fRet) {
					CloseHandle(aProcessInformation.hThread);
					fChildPid = (int)aProcessInformation.hProcess;
				} else {
					SystemLog::Error(String("PipeExecutor failed, error number: ")
									 << System::GetSystemError()
									 << " <" << SystemLog::LastSysError()
									 << "> return code " << fRet);
				}
			}
			inp.ShutDownReading(); // we write to it
			outp.ShutDownWriting();// we read from it
			if (fStderr) {
				fStderr->ShutDownWriting();
				fStderr->CloseOnDelete();
			}
			fPipe = new Pipe(outp.GetReadFd(), inp.GetWriteFd(), true, fTimeout);
#endif
		}
		if (0 == fPipe) {
			// ensure pipes are closed
			inp.ShutDownReading();
			inp.ShutDownWriting();
			outp.ShutDownReading();
			outp.ShutDownWriting();
			if (fStderr) {
				fStderr->CloseOnDelete();
				delete fStderr;
				fStderr = 0;
			}
		}
	} else {
		SystemLog::Error(String("error in PipeExecutor::ForkAndRun (") << SystemLog::LastSysError() << ")");
	}
	return (0 != fPipe); // we failed
}
