/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SystemBaseTest.h"

using namespace coast;

#include "TestSuite.h"
#include "DiffTimer.h"
#include "SystemLog.h"

SystemBaseTest::SystemBaseTest(TString tname)
	: TestCaseType(tname)
{
}

void SystemBaseTest::DoSingleSelectTest()
{
	StartTrace(SystemBaseTest.DoSingleSelectTest);
#if !defined(WIN32) // select not possible on non-socket handles on WIN32
	// assume writability of stdout
	int result = system::DoSingleSelect(STDOUT_FILENO, 100, false, true);
	assertEqual(1L, result);
	if ( result < 0 ) {
		SYSERROR("error in DoSingleSelect [" << SystemLog::LastSysError() << "]");
	}

	// just wait 100ms
	const long waittime = 1000L;
	DiffTimer dt(DiffTimer::eMilliseconds);//1ms accuracy
	// wait for stdin, only test cases where we really have a timeout (==0)
	int iSSRet = system::DoSingleSelect(STDIN_FILENO, waittime, false, false);
	if ( t_assertm(iSSRet >= 0, "expected select to timeout or succeed") ) {
		if ( iSSRet == 0L) {
			long difft = dt.Diff();
			Trace("time waited: " << difft << "ms, return code of function:" << iSSRet);
			difft -= waittime;
			Trace("difference to expected waittime of " << waittime << "ms : " << difft << "ms");
			// need some tolerance on some systems, eg. older SunOS5.6
			t_assertm(difft >= -10, TString("assume waiting long enough >=-10ms, diff was:") << difft << "ms");
			t_assertm(difft < waittime / 5, (const char *)(String("assume 20% (20ms) accuracy, but was ") << difft));
		}
	} else {
	  SYSERROR("error in DoSingleSelect [" << SystemLog::LastSysError() << "]");
	}
#endif
	// sanity check negative value
	t_assert(system::DoSingleSelect(-1, 0, true, false) < 0);
	// timeout tested indirectly in socket test cases
	// cannot think of case forcing select really fail, i.e. return -1
}

void SystemBaseTest::MicroSleepTest()
{
	DiffTimer::eResolution resolution( DiffTimer::eMicroseconds );
	DiffTimer dt(resolution); // microsecond accuracy
	const long SLEEP = resolution / 10; // = 100000L; // 100ms
	t_assert(system::MicroSleep(SLEEP));
	long sleptdelta = dt.Diff() - SLEEP;
	t_assertm(SLEEP / 5 > abs(sleptdelta), (const char *)(String( "expected sleeping with 20% = 20'000 microsecs (0.02s) accuracy -- but was ") << sleptdelta << " microseconds"));
}

void SystemBaseTest::GetProcessEnvironmentTest()
{
	Anything env;
	system::GetProcessEnvironment(env);
	t_assert(env.IsDefined("PATH"));
	t_assert(!env.IsDefined("this_shouldnt_be_in_the_environment"));
	t_assert(env.GetSize() > 1);
}

void SystemBaseTest::allocFreeTests()
{
	void *vp = coast::storage::Current()->Calloc(32, sizeof(char));
	t_assert(vp != 0);
	coast::storage::Current()->Free(vp);
}

void SystemBaseTest::TimeTest ()
{
	time_t now = ::time(0);

	struct tm agmtime;
	agmtime.tm_year = 0;
	system::GmTime(&now, &agmtime);
	t_assert(agmtime.tm_year > 0);

	struct tm alocaltime;
	alocaltime.tm_year = 0;
	system::LocalTime(&now, &alocaltime);
	t_assert(alocaltime.tm_year > 0);

	assertEqual(agmtime.tm_sec, alocaltime.tm_sec);
	assertEqual(agmtime.tm_min, alocaltime.tm_min);
}

#if !defined(WIN32)
void SystemBaseTest::LockFileTest()
{
	StartTrace(SystemBaseTest.GetFileSizeTest);
	String lockFile(system::GetTempPath().Append(system::Sep()).Append("LockFileTest.lck"));
	bool ret = system::GetLockFileState(lockFile);
	t_assertm(false == ret, "expected Lockfile to be unlocked");
	t_assertm(true == system::GetLockFileState(lockFile), "expected LockFile to be locked");
	t_assertm(false == system::io::unlink(lockFile), "expected unlinking LockFile to succeed.");
}
#endif

void SystemBaseTest::SnPrintf_ReturnsBytesOfContentWrittenWithoutTerminatingZero() {
	{
		const int bufSize = 64;
		char buf[bufSize] = {'X'};
		int bytesWritten = coast::system::SnPrintf(buf, bufSize, "%s", "123456789");
		assertEqual(9, bytesWritten);
		assertCharPtrEqual("123456789", buf);
		assertEqual(buf[bytesWritten], '\0');
	}
	{
		const int bufSize = 10;
		char buf[bufSize] = {'X'};
		int bytesWritten = coast::system::SnPrintf(buf, bufSize, "%s", "123456789");
		assertEqual(9, bytesWritten);
		assertCharPtrEqual("123456789", buf);
		assertEqual(buf[bytesWritten], '\0');
	}
}

void SystemBaseTest::SnPrintf_ReturnsBytesRequiredWithoutTerminatingZero() {
	{
		const int bufSize = 9;
		char buf[bufSize] = {'X'};
		int bytesRequired = coast::system::SnPrintf(buf, bufSize, "%s", "12345678901234567890");
		assertEqual(20, bytesRequired);
		assertCharPtrEqual("12345678", buf);
		assertEqual(buf[bufSize-1], '\0');
	}
}

void SystemBaseTest::SnPrintf_WritesTerminatingZeroEvenWhenTruncatingBuffer() {
	{
		const int bufSize = 9;
		char buf[bufSize] = {0};
		int bytesRequired = coast::system::SnPrintf(buf, bufSize, "%s", "123456789");
		assertEqual(9, bytesRequired);
		assertCharPtrEqual("12345678", buf);
		assertEqual(buf[bufSize-1], '\0');
	}
}

Test *SystemBaseTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SystemBaseTest, DoSingleSelectTest);
	ADD_CASE(testSuite, SystemBaseTest, GetProcessEnvironmentTest);
	ADD_CASE(testSuite, SystemBaseTest, allocFreeTests);
	ADD_CASE(testSuite, SystemBaseTest, MicroSleepTest);
	ADD_CASE(testSuite, SystemBaseTest, TimeTest);
	ADD_CASE(testSuite, SystemBaseTest, SnPrintf_ReturnsBytesOfContentWrittenWithoutTerminatingZero);
	ADD_CASE(testSuite, SystemBaseTest, SnPrintf_ReturnsBytesRequiredWithoutTerminatingZero);
	ADD_CASE(testSuite, SystemBaseTest, SnPrintf_WritesTerminatingZeroEvenWhenTruncatingBuffer);
#if !defined(WIN32)
	ADD_CASE(testSuite, SystemBaseTest, LockFileTest);
#endif
	return testSuite;
}
