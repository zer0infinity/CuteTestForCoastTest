/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Resolver.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Socket.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <ctype.h> // for isdigit test..

#if !defined(WIN32)
#include <arpa/nameser.h>	// for inet_ntop gethostbyname etc.
#include <arpa/inet.h>		// for inet_ntop gethostbyname etc.
#include <resolv.h>			// for inet_ntop gethostbyname etc.
#include <netdb.h>			// for gethostbyname etc.
#include <sys/socket.h>		// for gethostbyname etc.
#include <netinet/in.h>		// for gethostbyname etc.
#else
#include <io.h>
#endif

#define	INET_ADDRSTRLEN		16	// "ddd.ddd.ddd.ddd\0"
/* Define following even if IPv6 not supported, so we can always allocate
   an adequately-sized buffer, without #ifdefs in the code. */
#ifndef INET6_ADDRSTRLEN
/* $$.Ic INET6_ADDRSTRLEN$$ */
#define	INET6_ADDRSTRLEN	46	/* max size of IPv6 address string:
"xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:xxxx" or
"xxxx:xxxx:xxxx:xxxx:xxxx:xxxx:ddd.ddd.ddd.ddd\0"
1234567890123456789012345678901234567890123456 */
#endif

#if defined(__sun)
#define SystemSpecific(className)		_NAME2_(Sun, className)
#elif defined(WIN32)
#define SystemSpecific(className)		_NAME2_(Win32, className)
#elif defined(__APPLE__)
#define SystemSpecific(className)		_NAME2_(Apple, className)
#else //we suggest Linux (for posix)
#define SystemSpecific(className)		_NAME2_(Linux, className)
#endif

class EXPORTDECL_FOUNDATION SystemSpecific(Resolver): public Resolver
{
public:
	SystemSpecific(Resolver)() {}
	~SystemSpecific(Resolver)() {}

	virtual bool DNS2IP(String &ipAddress, const String &dnsName);
	virtual bool IP2DNS(String &dnsName, const String &ipAddress, unsigned long addr);
};

//---- Resolver ----------------------------------------------------------------

String Resolver::DNS2IPAddress( const String &dnsName )
{
	StartTrace(Resolver.DNS2IPAddress);
	// no need to change Server IP address if localhost (?)
	if ( (dnsName.Length() <= 0) || isdigit( dnsName.At(0) ) ) {
		return dnsName;
	} else {
		String ipAddress;
		SystemSpecific(Resolver) sysResolver;

		if ( sysResolver.DNS2IP(ipAddress, dnsName) ) {
			return ipAddress;
		}

		String logMsg("Resolving of DNS Name<");
		logMsg << dnsName << "> failed";
		SysLog::Error(logMsg);
	}
	return "127.0.0.1";
}

String Resolver::IPAddress2DNS( const String &ipAddress )
{
	unsigned long addr = EndPoint::MakeInetAddr(ipAddress);
	String dnsName("localhost");

	SystemSpecific(Resolver) sysResolver;

	if ( !sysResolver.IP2DNS(dnsName, ipAddress, addr) ) {
		String logMsg("Resolving of IPAddress <");
		logMsg << ipAddress << "> failed";
		SysLog::Error(logMsg);
	}
	dnsName.ToLower();
	return dnsName;
}

Resolver::Resolver() {}
Resolver::~Resolver() {}

//--- system dependent subclasses ----
#if defined(__sun)
extern "C" const char *inet_ntop __P((int af, const void *src, char *dst, size_t s));

bool SunResolver::DNS2IP(String &ipAddress, const String &dnsName)
{
	struct hostent he;
	const int bufSz = 8192;
	char buf[bufSz];
	int err = 0;

	if (gethostbyname_r(dnsName, &he, buf, bufSz, &err)) {
		char **pptr;
		char str[INET6_ADDRSTRLEN];
		pptr = he.h_addr_list;
		ipAddress = inet_ntop(he.h_addrtype, *pptr, str, sizeof(str));
		return true;
	}
	return false;
}

bool SunResolver::IP2DNS(String &dnsName, const String &ipAddress, unsigned long addr)
{
	struct hostent he;
	int err;

	const int bufSz = 8192;
	char buf[bufSz];

	if (gethostbyaddr_r((char *)&addr, sizeof(addr), AF_INET, &he, buf, bufSz, &err)) {
		dnsName = he.h_name;
		return true;
	}
	return false;
}

#elif defined(WIN32)

bool Win32Resolver::DNS2IP(String &ipAddress, const String &dnsName)
{
	struct hostent   *ptrHe;

	if ( ptrHe = gethostbyname(dnsName)) { // in windows thread scope
		struct in_addr *iaddr = (struct in_addr *) * ptrHe->h_addr_list;
		ipAddress = inet_ntoa(*iaddr);
		return true;
	}
	// for tracing possible errors
	// long error= WSAGetLastError ();

	return false;
}

bool Win32Resolver::IP2DNS(String &dnsName, const String &ipAddress, unsigned long addr)
{
	struct hostent *hePtr;
	if (hePtr = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET)) {
		dnsName = (hePtr->h_name);
		return true;
	}
	return false;
}

#elif defined(__APPLE__)

bool AppleResolver::DNS2IP(String &ipAddress, const String &dnsName)
{
	struct hostent   *ptrHe;
	int error_num = 0;

	if (ptrHe = getipnodebyname(dnsName, AF_INET, AI_DEFAULT, &error_num)) {
		struct in_addr *iaddr = (struct in_addr *) * ptrHe->h_addr_list;
		ipAddress = inet_ntoa(*iaddr);
		freehostent(ptrHe);
		return true;
	}

	return false;
}

bool AppleResolver::IP2DNS(String &dnsName, const String &ipAddress, unsigned long addr)
{
	struct hostent *ptrHe;
	int error_num = 0;
	if (ptrHe = getipnodebyaddr((char *)&addr, sizeof(addr), AF_INET, &error_num)) {
		dnsName = (ptrHe->h_name);
		freehostent(ptrHe);
		return true;
	}
	return false;
}

#else

bool LinuxResolver::DNS2IP(String &ipAddress, const String &dnsName)
{
	struct hostent he;
	struct hostent *res;
	const int bufSz = 8192;
	char buf[bufSz];
	int err;

	if (gethostbyname_r(dnsName, &he, buf, bufSz, &res, &err) == 0) {
		char **pptr;
		char str[INET6_ADDRSTRLEN];
		pptr = he.h_addr_list;
		ipAddress = inet_ntop(he.h_addrtype, *pptr, str, sizeof(str));
		return true;
	}
	return false;
}

bool LinuxResolver::IP2DNS(String &dnsName, const String &ipAddress, unsigned long addr)
{
	struct hostent he;
	struct hostent *res = 0;
	int err = 0;

	const int bufSz = 8192;
	char buf[bufSz];

	int result = gethostbyaddr_r((char *)&addr, sizeof(addr), AF_INET, &he, buf, bufSz, &res, &err);
	if ( result == 0 && err == NETDB_SUCCESS) {
		dnsName = res->h_name;
		return true;
	}
	return false;
}

#endif

