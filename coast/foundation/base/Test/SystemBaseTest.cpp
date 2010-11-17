/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include -----------------------------------------------------
#include "SystemBaseTest.h"

//--- module under test --------------------------------------------------------
#include "SystemBase.h"

using namespace Coast;

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used -------------------------------------------------
#include "DiffTimer.h"
#include "SystemLog.h"
#include "AnyIterators.h"

//--- c-library modules used ---------------------------------------------------
#include <fcntl.h>
#include <unistd.h>

//---- SystemBaseTest --------------------------------------------------------
SystemBaseTest::SystemBaseTest(TString tname)
	: TestCaseType(tname)
{
}

SystemBaseTest::~SystemBaseTest()
{
}

void SystemBaseTest::DoSingleSelectTest()
{
	StartTrace(SystemBaseTest.DoSingleSelectTest);
#if !defined(WIN32) // select not possible on non-socket handles on WIN32
	// assume writability of stdout
	int result = System::DoSingleSelect(STDOUT_FILENO, 100, false, true);
	assertEqual(1L, result);
	if ( result < 0 ) {
		SYSERROR("error in DoSingleSelect [" << SystemLog::LastSysError() << "]");
	}

	// just wait 100ms
	const long waittime = 1000L;
	DiffTimer dt(DiffTimer::eMilliseconds);//1ms accuracy
	// wait for stdin
	int iSSRet = System::DoSingleSelect(STDIN_FILENO, waittime, false, false);
	long difft = dt.Diff();
	Trace("time waited: " << difft << "ms, return code of function:" << iSSRet);
	difft -= waittime;
	Trace("difference to expected waittime of " << waittime << "ms : " << difft << "ms");
	assertEqual(0L, iSSRet);
	if ( iSSRet < 0 ) {
		SYSERROR("error in DoSingleSelect [" << SystemLog::LastSysError() << "]");
	}
	// need some tolerance on some systems, eg. older SunOS5.6
	t_assertm(difft >= -10, TString("assume waiting long enough >=-10ms, diff was:") << difft << "ms");
	t_assertm(difft < waittime / 5, (const char *)(String("assume 20% (20ms) accuracy, but was ") << difft));
	// sanity check negative value
#endif
	t_assert(System::DoSingleSelect(-1, 0, true, false) < 0);
	// timeout tested indirectly in socket test cases
	// cannot think of case forcing select really fail, i.e. return -1
}

void SystemBaseTest::MicroSleepTest()
{
	DiffTimer::eResolution resolution( DiffTimer::eMicroseconds );
	DiffTimer dt(resolution); // microsecond accuracy
	const long SLEEP = resolution / 10; // = 100000L; // 100ms
	t_assert(System::MicroSleep(SLEEP));
	long sleptdelta = dt.Diff() - SLEEP;
	t_assertm(SLEEP / 5 > abs(sleptdelta), (const char *)(String( "expected sleeping with 20% = 20'000 microsecs (0.02s) accuracy -- but was ") << sleptdelta << " microseconds"));
}

void SystemBaseTest::GetProcessEnvironmentTest()
{
	Anything env;
	System::GetProcessEnvironment(env);
	t_assert(env.IsDefined("PATH"));
	t_assert(!env.IsDefined("this_shouldnt_be_in_the_environment"));
	t_assert(env.GetSize() > 1);
}

void SystemBaseTest::allocFreeTests()
{
	void *vp = Storage::Current()->Calloc(32, sizeof(char));
	t_assert(vp != 0);
	Storage::Current()->Free(vp);
}

void SystemBaseTest::TimeTest ()
{
	time_t now = ::time(0);

	struct tm agmtime;
	agmtime.tm_year = 0;
	System::GmTime(&now, &agmtime);
	t_assert(agmtime.tm_year > 0);

	struct tm alocaltime;
	alocaltime.tm_year = 0;
	System::LocalTime(&now, &alocaltime);
	t_assert(alocaltime.tm_year > 0);

	assertEqual(agmtime.tm_sec, alocaltime.tm_sec);
	assertEqual(agmtime.tm_min, alocaltime.tm_min);
}

#if !defined(WIN32)
void SystemBaseTest::LockFileTest()
{
	StartTrace(SystemBaseTest.GetFileSizeTest);
	String lockFile(System::GetTempPath().Append(System::Sep()).Append("LockFileTest.lck"));
	bool ret = System::GetLockFileState(lockFile);
	t_assertm(false == ret, "expected Lockfile to be unlocked");
	t_assertm(true == System::GetLockFileState(lockFile), "expected LockFile to be locked");
	t_assertm(false == System::IO::unlink(lockFile), "expected unlinking LockFile to succeed.");
}
#endif

Test *SystemBaseTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SystemBaseTest, DoSingleSelectTest);
	ADD_CASE(testSuite, SystemBaseTest, GetProcessEnvironmentTest);
	ADD_CASE(testSuite, SystemBaseTest, allocFreeTests);
	ADD_CASE(testSuite, SystemBaseTest, MicroSleepTest);
	ADD_CASE(testSuite, SystemBaseTest, TimeTest);
#if !defined(WIN32)
	ADD_CASE(testSuite, SystemBaseTest, LockFileTest);
#endif
	return testSuite;
}
