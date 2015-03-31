/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SystemBase.h"
#include "SystemLog.h"
#include "Tracer.h"
#include "Anything.h"
#include <cerrno>
#include <cstdarg>
#include <cstdio>
#include <fcntl.h>
#include <poll.h>
#if defined(__APPLE__)
#include <crt_externs.h>
#endif
#if defined(WIN32)
#include <io.h>
#include <direct.h>
#else
#include <sys/utsname.h>
#if defined(__sun)
extern char **environ;
#endif
#endif

namespace coast {
	namespace system {

		// avoid that fd is shared by forked processes
		void SetCloseOnExec(int fd)
		{
		#if !defined(WIN32)
			if (fd >= 0 ) {
				int options = fcntl(fd, F_GETFD);
				if (options >= 0) {
					fcntl(fd, F_SETFD, options | FD_CLOEXEC);
				}
			}
		#endif
		}

		int DoSingleSelect(int fd, long timeout, bool bRead, bool bWrite)
		{
			StartTrace1(System.DoSingleSelect, "testing fd:" << static_cast<long>(fd) << "(" << (bRead ? "r" : "-") << (bWrite ? "w" : "-") << ") with timeout:" << timeout << "ms");
			if (fd < 0 ) {
				return -1;
			}
			pollfd fds[1];
			fds[0].fd= fd;
			fds[0].events= (bRead?POLLIN:0)|(bWrite?POLLOUT:0);
			return ::poll( fds, 1, timeout);
		}

		bool IsReadyForReading(int fd, long timeout) {
			return 0 < DoSingleSelect(fd, timeout, true, false);
		}

		bool IsReadyForWriting(int fd, long timeout) {
			return 0 < DoSingleSelect(fd, timeout, false, true);
		}

		bool MicroSleep(long sleepTime)
		{
		#if defined(WIN32)
			Sleep(sleepTime / 1000L);
			return true;
		#else
			// alternative is to use Posix's nanosleep(struct timespec*,struct timespec*)
			struct timeval tv;
			const long MILLION = 1000000L;
			tv.tv_sec = sleepTime / MILLION;
			tv.tv_usec = (sleepTime % MILLION);
			return 0 == select(0, 0, 0, 0, &tv);
		#endif
		}

		int GetSystemError()
		{
		#if defined(WIN32)
			// either it was a regular system error or a windows socket error, sometimes errno is set
			// too, but we can't rely on that...
			if (GetLastError()) {
				return GetLastError();
			}
			if (WSAGetLastError()) {
				return WSAGetLastError();
			}
		#endif
			return errno;
		}

		bool SyscallWasInterrupted()
		{
		#if defined(WIN32)
			if (GetLastError() == ERROR_INVALID_AT_INTERRUPT_TIME) { // according to cygwin error code mapping
				return true;
			}
			if (WSAGetLastError () == WSAEINTR) {
				return true;
			}
		#endif
			return ( errno == EINTR ) || (errno == EAGAIN);
		}

		struct tm *LocalTime( const time_t *timer, struct tm *res ) {
		#if defined(WIN32)
			//!@FIXME not 100% thread safe, localtime uses a global variable to store its content
			// subsequent calls will overwrite the value
			(*res) = (*::localtime(timer));
			return res;
		#else
			return ::localtime_r(timer, res);
		#endif
		}

		struct tm *GmTime( const time_t *timer, struct tm *res ) {
		#if defined(WIN32)
			//!@FIXME not 100% thread safe, localtime uses a global variable to store its content
			// subsequent calls will overwrite the value
			(*res) = (*::localtime(timer));
			return res;
		#else
			return ::gmtime_r(timer, res);
		#endif
		}

		void AscTime( const struct tm *pTime, String &strTime )
		{
		#if defined WIN32
			strTime = asctime( pTime );
		#else
			char timeBuf[255];
			asctime_r(pTime, timeBuf);
			strTime = timeBuf;
		#endif
		}

		String EnvGet(const char *variable)
		{
			if (variable) {
				return String(getenv(variable));
			} else {
				return String();
			}
		}

		#if defined(WIN32)
		// the following code was taken from ACE
		bool Uname(Anything &anyInfo)
		{
			StartTrace(System.Uname);
			bool bRet = false;
			anyInfo["sysname"] = "Win32";

			OSVERSIONINFO vinfo;
			vinfo.dwOSVersionInfoSize = sizeof(OSVERSIONINFO);
			::GetVersionEx (&vinfo);

			SYSTEM_INFO sinfo;
			::GetSystemInfo(&sinfo);

			String processor, subtype;
			if ( vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT ) {
				// Get information from the two structures
				String os("Windows "), build("Build ");
				if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_NT) {
					os << "NT";
				} else {
					os << "CE";
				}
				anyInfo["release"] = os << (long)vinfo.dwMajorVersion << "." << (long)vinfo.dwMinorVersion;
				anyInfo["version"] = build << (long)vinfo.dwBuildNumber << " " << vinfo.szCSDVersion;

				WORD arch = sinfo.wProcessorArchitecture;
				switch (arch) {
					case PROCESSOR_ARCHITECTURE_INTEL:
						processor = "Intel";
						if (sinfo.wProcessorLevel == 3) {
							subtype = "80386";
						} else if (sinfo.wProcessorLevel == 4) {
							subtype = "80486";
						} else if (sinfo.wProcessorLevel == 5) {
							subtype = "Pentium";
						} else if (sinfo.wProcessorLevel == 6) {
							subtype = "Pentium Pro";
						} else if (sinfo.wProcessorLevel == 7) {
							subtype = "Pentium II";
						}
						break;
					case PROCESSOR_ARCHITECTURE_MIPS:
						processor = "MIPS";
						subtype = "R4000";
						break;
					case PROCESSOR_ARCHITECTURE_ALPHA:
						processor = "Alpha";
						subtype = sinfo.wProcessorLevel;
						break;
					case PROCESSOR_ARCHITECTURE_PPC:
						processor = "PPC";
						if (sinfo.wProcessorLevel == 1) {
							subtype = "601";
						} else if (sinfo.wProcessorLevel == 3) {
							subtype = "603";
						} else if (sinfo.wProcessorLevel == 4) {
							subtype = "604";
						} else if (sinfo.wProcessorLevel == 6) {
							subtype = "603+";
						} else if (sinfo.wProcessorLevel == 9) {
							subtype = "804+";
						} else if (sinfo.wProcessorLevel == 20) {
							subtype = "620";
						}
						break;
		#if defined(PROCESSOR_ARCHITECTURE_IA64)
					case PROCESSOR_ARCHITECTURE_IA64:
						processor = "Itanium";
						subtype = sinfo.wProcessorLevel;
						break;
		#endif
					case PROCESSOR_ARCHITECTURE_UNKNOWN:
					default:
						break;
				}
			} else if (vinfo.dwPlatformId == VER_PLATFORM_WIN32_WINDOWS) {
				String release, reladd;
				if (vinfo.dwMajorVersion == 4 && vinfo.dwMinorVersion == 0) {
					release = "Windows 95";
					if (vinfo.szCSDVersion[1] == 'C') {
						reladd = " OSR2";
					}
				} else if (vinfo.dwMajorVersion == 4 && vinfo.dwMinorVersion == 10) {
					release = "Windows 98";
					if (vinfo.szCSDVersion[1] == 'A') {
						reladd = " SE";
					}
				} else if (vinfo.dwMajorVersion == 4 && vinfo.dwMinorVersion == 90) {
					release = "Windows Me";
				}
				if ( release.Length() ) {
					anyInfo["release"] = release << reladd;
				}
				anyInfo["version"] = (long)LOWORD(vinfo.dwBuildNumber);

				if (sinfo.dwProcessorType == PROCESSOR_INTEL_386) {
					processor = "Intel 80386";
				} else if (sinfo.dwProcessorType == PROCESSOR_INTEL_486) {
					processor = "Intel 80486";
				} else if (sinfo.dwProcessorType == PROCESSOR_INTEL_PENTIUM) {
					processor = "Intel Pentium";
				}
			}
			if ( processor.Length() ) {
				anyInfo["machine"] = processor << (subtype.Length() ? " " : "") << subtype;
			}

			String nodename;
			bRet = ( HostName(nodename) && bRet );
			if ( bRet ) {
				anyInfo["nodename"] = nodename;
			}
			return bRet;
		}
		#else
		bool Uname(Anything &anyInfo)
		{
			StartTrace(System.Uname);
			struct utsname name;
			int iRet = ::uname(&name);
			if ( iRet < 0 ) {
				String strErr("uname returned:");
				strErr << static_cast<long>(iRet);
				SYSERROR(strErr);
			} else {
				anyInfo["sysname"] = name.sysname;
				anyInfo["nodename"] = name.nodename;
				anyInfo["release"] = name.release;
				anyInfo["version"] = name.version;
				anyInfo["machine"] = name.machine;
			}
			TraceAny(anyInfo, "uname retcode:" << static_cast<long>(iRet));
			return ( iRet >= 0);
		}
		#endif /* WIN32 */

		bool HostName(String &name)
		{
			StartTrace(System.HostName);
			bool bRet = false;
		#if defined(WIN32)
			const DWORD dSz = 256;
			DWORD lBufSz = dSz;
			char strBuf[dSz];
			bRet = ( ::GetComputerNameA(strBuf, &lBufSz) != 0);
			if ( bRet ) {
				name = strBuf;
			}
		#else
			Anything host_info;
			if ( ( bRet = Uname(host_info) ) ) {
				name = host_info["nodename"].AsString("unknown");
			}
		#endif
			return bRet;
		}

		pid_t getpid()
		{
		#if defined(WIN32)
			return GetCurrentProcessId();
		#else
			return ::getpid();
		#endif
		}

		uid_t getuid()
		{
		#if defined(WIN32)
			return (uid_t) - 1L;
		#else
			return ::getuid();
		#endif
		}

		long Fork()
		{
		#if defined(WIN32)
			return -1;
		#elif defined(__sun)
			return fork1();
		#else
			return fork();
		#endif
		}

		int SnPrintf(char * buf, size_t buf_size, const char *format, ...) {
#if defined(WIN32)
#define vsnprintf	_vsnprintf
#endif
		    va_list args;
		    va_start(args, format);
			int charsWrittenOrRequired = ::vsnprintf(buf, buf_size, format, args);
		    va_end(args);
#if defined(WIN32)
		    // according to http://msdn.microsoft.com/en-us/library/2ts7cx93.aspx
		    // If len < count, len characters are stored in buffer, a null-terminator is appended, and len is returned.
		    // so far so good, but from the next lines on, we stick to our promise of always zero terminating the buffer
		    // If len == count, len characters are stored in buffer, no null-terminator is appended, and len is returned.
		    // If len > count, count characters are stored in buffer, no null-terminator is appended, and a negative value is returned.
			if ( charsWrittenOrRequired >= buf_size ) {
				buf[buf_size-1] = '\0';
			}
#undef vsnprintf
#endif
			return charsWrittenOrRequired;
		}

		namespace {
			bool CheckErrnoForRangeError() {
				if (errno == ERANGE) {
					errno = 0;
					return false;
				}
				return true;
			}
		}

		bool StrToL(long & value, const char* str, int base) {
			using namespace std;
			value = strtol(str, 0, base);
			return CheckErrnoForRangeError();
		}

		bool StrToD(double& value, const char* str) {
			using namespace std;
			value = strtod(str, 0);
			return CheckErrnoForRangeError();
		}

		bool StrToULL(unsigned long long & value, const char* str, int base) {
			using namespace std;
			value = strtoull(str, 0, base);
			return CheckErrnoForRangeError();
		}

		bool StrToLL(long long & value, const char* str, int base) {
			using namespace std;
			value = strtoll(str, 0, base);
			return CheckErrnoForRangeError();
		}

		bool StrToUL(unsigned long & value, const char* str, int base) {
			using namespace std;
			value = strtoul(str, 0, base);
			return CheckErrnoForRangeError();
		}

#if !defined(WIN32)
		// locking would actually work on WIN32 but unlocking is not as easy because open files can't
		// be deleted. cygwin remembers the fd to close the file before unlinking.
		bool GetLockFileState(const char *lockFileName)
		{
			StartTrace(System.CreateLockFile);
			// O_EXCL must be set to guarantee atomic lock file creation.
			int fd = open(lockFileName, O_WRONLY | O_CREAT | O_EXCL, S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH);
			if (fd < 0 && errno == EEXIST) {
				// the file already exist; another process is
				// holding the lock
				return true;
			}
			if (fd < 0) {
				SYSERROR("Getting lock state failed. Reason: [" << SystemLog::LastSysError() << "]");
				// What to do in case of an error? Let's be conservative and consider the lockfile as locked.
				return true;
			}
			// if we got here, we own the lock or had an error
			return false;
		}
#endif

	}
}

void coast::system::GetProcessEnvironment(Anything &anyEnv)
{
	StartTrace(System.GetProcessEnvironment);
	char **envp =
#if defined(__APPLE__)
			* _NSGetEnviron()
#else
			environ
#endif
			;
	for (long i = 0; envp && envp[i] ; ++i) {
		String entry(envp[i]);
		Trace("entry = <" << entry << ">");
		long equalsign = entry.StrChr('=');
		if (equalsign >= 0) {
			String key = entry.SubString(0L, equalsign);
			Trace("key = <" << key << ">");
			String value = entry.SubString(equalsign + 1);
			Trace("value = <" << value << ">");
			if (key.Length() > 0) {
				anyEnv[key] = value;
			}
		}
	}
}
