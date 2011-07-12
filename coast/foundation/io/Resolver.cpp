/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Resolver.h"
#include "SystemLog.h"
#include "Socket.h"
#include "Tracer.h"
#if !defined(WIN32)
#include <arpa/inet.h>		// for inet_ntop gethostbyname etc.
#include <netdb.h>			// for gethostbyname etc.
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

class SystemSpecific(Resolver)
{
	Anything fAliases;
	String fCanonicalName;

	void extractFromHostent( const hostent& result ) {
		StartTrace(Resolver.extractFromHostent);
		fCanonicalName = result.h_name;
		String strAlias(32L);
		for (char **q=result.h_aliases; *q != 0; ++q) {
			strAlias = *q;
			fAliases.Append(strAlias);
			if ( strAlias.StrChr('.') > 0 && strAlias.Contains(fCanonicalName) >= 0 ) {
				Trace("fqdn found [" << strAlias << "]");
				fCanonicalName=strAlias;
			}
		}
		TraceAny(fAliases, "names for given ip");
		Trace("resolved name [" << fCanonicalName << "]");
	}

public:
	SystemSpecific(Resolver)() {}
	~SystemSpecific(Resolver)() {}

	const String& getCanonicalName() const { return fCanonicalName; }
	ROAnything getAliases() const { return fAliases; }

	bool DNS2IP(String &ipAddress, const String &dnsName);
	bool IP2DNS(const String &ipAddress, unsigned long addr);
};

//---- Resolver ----------------------------------------------------------------

String Resolver::DNS2IPAddress( const String &dnsName, const String &dflt )
{
	StartTrace1(Resolver.DNS2IPAddress, "dns [" << dnsName << "]");
	// no need to change Server IP address if localhost (?)
	if ( (dnsName.Length() <= 0) || isdigit( dnsName.At(0) ) ) {
		return dnsName;
	} else {
		String ipAddress;
		SystemSpecific(Resolver) sysResolver;

		if ( sysResolver.DNS2IP(ipAddress, dnsName) ) {
			Trace("resolved ip [" << ipAddress << "]");
			return ipAddress;
		}

		String logMsg("Resolving of DNS Name<");
		logMsg << dnsName << "> failed";
		SystemLog::Error(logMsg);
	}
	return dflt;
}

String Resolver::IPAddress2DNS( const String &ipAddress, const String &dflt )
{
	StartTrace1(Resolver.IPAddress2DNS, "ip [" << ipAddress << "]");
	unsigned long addr = EndPoint::MakeInetAddr(ipAddress);

	SystemSpecific(Resolver) sysResolver;

	if ( !sysResolver.IP2DNS(ipAddress, addr) ) {
		String logMsg("Resolving of IPAddress <");
		logMsg << ipAddress << "> failed";
		SystemLog::Error(logMsg);
		return dflt;
	}
	Trace("resolved name [" << sysResolver.getCanonicalName() << "]");
	return sysResolver.getCanonicalName();
}

//--- system dependent subclasses ----
#if defined(__sun)

bool SunResolver::DNS2IP(String &ipAddress, const String &dnsName)
{
	StartTrace1(Resolver.DNS2IP, "<sun> dns [" << dnsName << "]");
	struct hostent he;
	const int bufSz = 8192;
	char buf[bufSz] = { 0 };
	int err = 0;

	if (gethostbyname_r(dnsName, &he, buf, bufSz, &err)) {
		char **pptr;
		char str[INET6_ADDRSTRLEN] = { 0 };
		pptr = he.h_addr_list;
		ipAddress = inet_ntop(he.h_addrtype, *pptr, str, sizeof(str));
		return true;
	}
	return false;
}

bool SunResolver::IP2DNS(const String &ipAddress, unsigned long addr)
{
	StartTrace1(Resolver.IP2DNS, "<sun> ip [" << ipAddress << "]");
	struct hostent result = { 0 };
	int err = 0;

	const int bufSz = 8192;
	char buf[bufSz] = { 0 };

	struct hostent *hret = gethostbyaddr_r((char *)&addr, sizeof(addr), AF_INET, &result, buf, bufSz, &err);
	Trace("err:" << static_cast<long>(err));
	if (hret != 0) {
		extractFromHostent(result);
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

bool Win32Resolver::IP2DNS(const String &ipAddress, unsigned long addr)
{
	struct hostent *hePtr;
	if (hePtr = gethostbyaddr((char *)&addr, sizeof(addr), AF_INET)) {
		extractFromHostent(*hePtr);
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

bool AppleResolver::IP2DNS(const String &ipAddress, unsigned long addr)
{
	struct hostent *ptrHe;
	int error_num = 0;
	if (ptrHe = getipnodebyaddr((char *)&addr, sizeof(addr), AF_INET, &error_num)) {
		extractFromHostent(*ptrHe);
		freehostent(ptrHe);
		return true;
	}
	return false;
}

#else

bool LinuxResolver::DNS2IP(String &ipAddress, const String &dnsName)
{
	StartTrace1(Resolver.DNS2IP, "<linux> dns [" << dnsName << "]");
	struct hostent he;
	struct hostent *res=0;
	const int bufSz = 8192;
	char buf[bufSz] = {0};
	int err=0;

	if (gethostbyname_r(dnsName, &he, buf, bufSz, &res, &err) == 0) {
		Trace("err:" << static_cast<long>(err) << " res:" << reinterpret_cast<long>(res));
		// according to man page of gethostbyname_r, res != NULL can be taken as valid result
		if (res!=0) {
			char **pptr;
			char str[INET6_ADDRSTRLEN]={0};
			pptr = he.h_addr_list;
			ipAddress = inet_ntop(he.h_addrtype, *pptr, str, sizeof(str));
			return true;
		}
	}
	return false;
}

bool LinuxResolver::IP2DNS(const String &ipAddress, unsigned long addr)
{
	StartTrace1(Resolver.IP2DNS, "<linux> ip [" << ipAddress << "]");
	struct hostent he;
	struct hostent *res = 0;
	int err = 0;

	const int bufSz = 8192;
	char buf[bufSz];

	int result = gethostbyaddr_r((char *)&addr, sizeof(addr), AF_INET, &he, buf, bufSz, &res, &err);
	if ( result == 0 && err == NETDB_SUCCESS) {
		extractFromHostent(*res);
		return true;
	}
	return false;
}

#endif

