/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "UniqueIdGen.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "MD5.h"
#include "SystemAPI.h"
#include "System.h"
#include "DiffTimer.h"
#include "StringStream.h"

//--- c-library modules used ---------------------------------------------------
#include <cstring>
#if !defined(WIN32)
#include <sys/time.h>
#endif

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

//--- c-modules used -----------------------------------------------------------

//---- UniqueIdGen ----------------------------------------------------------------
UniqueIdGen::UniqueIdGen()
{
	StartTrace(UniqueIdGen.Ctor);
}

UniqueIdGen::~UniqueIdGen()
{
	StartTrace(UniqueIdGen.Dtor);
}

String UniqueIdGen::GetUniqueId(const String &additionalToken)
{
	StartTrace(UniqueIdGen.GetUniqueId);
	timeval tv;
	tv.tv_sec	= 0;
	tv.tv_usec	= 0;
	long seconds		= 0L;
	long nanoSeconds	= 0L;
	if ( 0 == gettimeofday(&tv, 0) ) {
		nanoSeconds = tv.tv_usec * 1000; // make it nanoseconds
		seconds		= tv.tv_sec;
	} else {
		seconds = time(0);	// fall back towards using time
	}
	unsigned long ms = 	(seconds * 1000000L) + nanoSeconds;
	srand48(GetHRTIME());
	unsigned long random = lrand48();
	unsigned long hrt = GetHRTIME();
	long hostid = ::gethostid();
	long pid = System::getpid();
	char buf[1024];
	memset(buf, '\0', 1024L);
	snprintf(buf, sizeof(buf), "%lu_%lu_%ld_%ld_%lu", ms, hrt, hostid, pid, random);
	String uniqueId(buf);
	uniqueId << "_" << additionalToken;
	Trace("uniqueId: [" << uniqueId << "]");
	MD5Signer md5("UniqueIdGen");
	String ret(HexHash(md5, uniqueId));
	Trace("Resulting UniqueId: [" << ret << "]");
	return ret;
}

String UniqueIdGen::HexHash(MD5Signer &md5, const char *s)
{
	StartTrace(UniqueIdGen.HexHash);
	String hash;
	md5.DoHash(s, hash);
	String buffer;
	OStringStream os(buffer);
	for (long i = 0 ; i < hash.Length(); i++) {
		os << hex <<  setw(2) << setfill('0') << (int)(unsigned char)hash[i];
	}
	return buffer;
}
