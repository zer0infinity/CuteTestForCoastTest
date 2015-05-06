/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SystemBaseTest.h"

using namespace coast;

#include "DiffTimer.h"
#include "SystemLog.h"

void SystemBaseTest::DoSingleSelectTest()
{
	StartTrace(SystemBaseTest.DoSingleSelectTest);
#if !defined(WIN32) // select not possible on non-socket handles on WIN32
	// assume writability of stdout
	int result = system::DoSingleSelect(STDOUT_FILENO, 100, false, true);
	ASSERT_EQUAL(1L, result);
	if ( result < 0 ) {
		SYSERROR("error in DoSingleSelect [" << SystemLog::LastSysError() << "]");
	}

	// just wait 100ms
	const long waittime = 1000L;
	DiffTimer dt(DiffTimer::eMilliseconds);//1ms accuracy
	// wait for stdin, only test cases where we really have a timeout (==0)
	int iSSRet = system::DoSingleSelect(STDIN_FILENO, waittime, false, false);
	ASSERTM("expected select to timeout or succeed", iSSRet >= 0);
	if (iSSRet == 0L) {
		long difft = dt.Diff();
		Trace("time waited: " << difft << "ms, return code of function:" << iSSRet);
		difft -= waittime;
		Trace("difference to expected waittime of " << waittime << "ms : " << difft << "ms");
		// need some tolerance on some systems, eg. older SunOS5.6
		std::string message("assume waiting long enough >=-10ms, diff was: ");
		message += difft;
		message += "ms";
		ASSERTM(message, difft >= -10);
		message += "assume 20% (20ms) accuracy, but was ";
		message += difft;
		ASSERTM(message, difft < waittime / 5);
	} else {
		SYSERROR("error in DoSingleSelect [" << SystemLog::LastSysError() << "]");
	}
#endif
	// sanity check negative value
	ASSERT(system::DoSingleSelect(-1, 0, true, false) < 0);
	// timeout tested indirectly in socket test cases
	// cannot think of case forcing select really fail, i.e. return -1
}

void SystemBaseTest::MicroSleepTest()
{
	DiffTimer::eResolution resolution( DiffTimer::eMicroseconds );
	DiffTimer dt(resolution); // microsecond accuracy
	const long SLEEP = resolution / 10; // = 100000L; // 100ms
	ASSERT(system::MicroSleep(SLEEP));
	long sleptdelta = dt.Diff() - SLEEP;
	ASSERTM( (const char *)(String( "expected sleeping with 20% = 20'000 microsecs (0.02s) accuracy -- but was ") << sleptdelta << " microseconds"),SLEEP / 5 > abs(sleptdelta));
}

void SystemBaseTest::GetProcessEnvironmentTest()
{
	Anything env;
	system::GetProcessEnvironment(env);
	ASSERT(env.IsDefined("PATH"));
	ASSERT(!env.IsDefined("this_shouldnt_be_in_the_environment"));
	ASSERT(env.GetSize() > 1);
}

void SystemBaseTest::allocFreeTests()
{
	void *vp = coast::storage::Current()->Calloc(32, sizeof(char));
	ASSERT(vp != 0);
	coast::storage::Current()->Free(vp);
}

void SystemBaseTest::TimeTest ()
{
	time_t now = ::time(0);

	struct tm agmtime;
	agmtime.tm_year = 0;
	system::GmTime(&now, &agmtime);
	ASSERT(agmtime.tm_year > 0);

	struct tm alocaltime;
	alocaltime.tm_year = 0;
	system::LocalTime(&now, &alocaltime);
	ASSERT(alocaltime.tm_year > 0);

	ASSERT_EQUAL(agmtime.tm_sec, alocaltime.tm_sec);
	ASSERT_EQUAL(agmtime.tm_min, alocaltime.tm_min);
}

#if !defined(WIN32)
void SystemBaseTest::LockFileTest()
{
	StartTrace(SystemBaseTest.GetFileSizeTest);
	String lockFile(system::GetTempPath().Append(system::Sep()).Append("LockFileTest.lck"));
	bool ret = system::GetLockFileState(lockFile);
	ASSERTM( "expected Lockfile to be unlocked",false == ret);
	ASSERTM( "expected LockFile to be locked",true == system::GetLockFileState(lockFile));
	ASSERTM( "expected unlinking LockFile to succeed.",false == system::io::unlink(lockFile));
}
#endif

void SystemBaseTest::SnPrintf_ReturnsBytesOfContentWrittenWithoutTerminatingZero() {
	{
		const int bufSize = 64;
		char buf[bufSize] = {'X'};
		int bytesWritten = coast::system::SnPrintf(buf, bufSize, "%s", "123456789");
		ASSERT_EQUAL(9, bytesWritten);
		ASSERT_EQUAL("123456789", buf);
		ASSERT_EQUAL(buf[bytesWritten], '\0');
	}
	{
		const int bufSize = 10;
		char buf[bufSize] = {'X'};
		int bytesWritten = coast::system::SnPrintf(buf, bufSize, "%s", "123456789");
		ASSERT_EQUAL(9, bytesWritten);
		ASSERT_EQUAL("123456789", buf);
		ASSERT_EQUAL(buf[bytesWritten], '\0');
	}
}

void SystemBaseTest::SnPrintf_ReturnsBytesRequiredWithoutTerminatingZero() {
	{
		const int bufSize = 9;
		char buf[bufSize] = {'X'};
		int bytesRequired = coast::system::SnPrintf(buf, bufSize, "%s", "12345678901234567890");
		ASSERT_EQUAL(20, bytesRequired);
		ASSERT_EQUAL("12345678", buf);
		ASSERT_EQUAL(buf[bufSize-1], '\0');
	}
}

void SystemBaseTest::SnPrintf_WritesTerminatingZeroEvenWhenTruncatingBuffer() {
	{
		const int bufSize = 9;
		char buf[bufSize] = {0};
		int bytesRequired = coast::system::SnPrintf(buf, bufSize, "%s", "123456789");
		ASSERT_EQUAL(9, bytesRequired);
		ASSERT_EQUAL("12345678", buf);
		ASSERT_EQUAL(buf[bufSize-1], '\0');
	}
}

void SystemBaseTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, DoSingleSelectTest));
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, GetProcessEnvironmentTest));
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, allocFreeTests));
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, MicroSleepTest));
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, TimeTest));
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, SnPrintf_ReturnsBytesOfContentWrittenWithoutTerminatingZero));
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, SnPrintf_ReturnsBytesRequiredWithoutTerminatingZero));
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, SnPrintf_WritesTerminatingZeroEvenWhenTruncatingBuffer));
#if !defined(WIN32)
	s.push_back(CUTE_SMEMFUN(SystemBaseTest, LockFileTest));
#endif
}
