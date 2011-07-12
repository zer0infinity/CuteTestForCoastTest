/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PipeTest.h"
#include "TestSuite.h"
#include "Pipe.h"
#include "Socket.h"
#include "Tracer.h"

#if defined(WIN32)
#include <io.h>
#endif

void PipeTest::simpleBlockingTest() {
	StartTrace(PipeTest.simpleBlockingTest);

	int p[2];
#if defined(WIN32)
	Pipe aPipe(100);
	p[0] = aPipe.GetReadFd();
	p[1] = aPipe.GetWriteFd();
	Trace("readfd:" << p[0] << " writefd:" << p[1]);
	if ( aPipe.GetReadFd() > 0 && aPipe.GetWriteFd() > 0 )
#else
	if (pipe(p) >= 0)
#endif
	{
		Pipe pipi(p[0], p[1], true, 100); // timeout 0.1 sec
		assertEqual((long)p[0], pipi.GetReadFd());
		assertEqual((long)p[1], pipi.GetWriteFd());

		t_assert(!pipi.IsReadyForReading(10));
		t_assert(pipi.HadTimeout()); // expect a time out
		t_assert(pipi.IsReadyForWriting(0));
		// write a lot of stuff
		const char *buf = "0123456789ABCDEF"; // 16 chars
		// enable blocking IO on write side
		t_assert(Socket::SetToNonBlocking(pipi.GetWriteFd(), false));
		t_assert(pipi.IsReadyForWriting(0));
		long byteswritten = 0, lMaxCount = 2;
		Trace("before while loop");
		while (pipi.IsReadyForWriting(10) && lMaxCount--) {
			// happens only once...
			byteswritten += (long) Socket::write(pipi.GetWriteFd(), buf, 16);
			Trace("bytes written so far:" << byteswritten);
		}
		//// foo: to be corrected in future
		////		assertEqualm(16, byteswritten, "expected exactly 16 bytes written");
		////		t_assert(!pipi.IsReadyForWriting(0)); // pipe blocked if not completely empty
		t_assert(pipi.IsReadyForReading(0));
		String str(17);
		long bytesread = Socket::read(pipi.GetReadFd(), (char *) (const char *) str, 16);
		assertEqual(16, bytesread);
		assertCharPtrEqual(buf, str);

		t_assert(pipi.IsReadyForWriting(0)); // now we should be able to write again
		// write something
		assertEqual(5L, (long)Socket::write(pipi.GetWriteFd(), "hallo", 5));
	} else {//lint !e550
		t_assertm(false, "couldn't create anonymous pipe");
	}
}

void PipeTest::simpleConstructorTest() {
	StartTrace(PipeTest.simpleConstructorTest);

	int p[2];
#if defined(WIN32)
	Pipe aPipe(100);
	p[0] = aPipe.GetReadFd();
	p[1] = aPipe.GetWriteFd();
	Trace("readfd:" << p[0] << " writefd:" << p[1]);
	if ( aPipe.GetReadFd() > 0 && aPipe.GetWriteFd() > 0 )
#else
	if (pipe(p) >= 0)
#endif
	{
		Pipe pipi(p[0], p[1], true, 100); // timeout 0.1 sec

		assertEqual((long)p[0], pipi.GetReadFd());
		assertEqual((long)p[1], pipi.GetWriteFd());

		t_assert(!pipi.IsReadyForReading(10));
		t_assert(pipi.HadTimeout()); // expect a time out
		t_assert(pipi.IsReadyForWriting(0));
		// write something
		assertEqual(5L, (long)Socket::write(pipi.GetWriteFd(), "hallo", 5));
		t_assert(pipi.IsReadyForReading(0));
		t_assert(!pipi.HadTimeout());
		// shut down writing
		t_assert(pipi.ShutDownWriting());
		assertEqual(-1L, pipi.GetWriteFd());
		t_assert(!pipi.IsReadyForWriting(0));
		t_assert(Socket::write(pipi.GetWriteFd(), "hallo", 5) < 0);
		t_assert(pipi.IsReadyForReading(0));

		char hallo[6]; // PS: never do this in real code!!!
		long bytesread = Socket::read(pipi.GetReadFd(), hallo, 6);
		assertEqual(5, bytesread);
		hallo[5] = '\0';
		assertCharPtrEqual("hallo", hallo);

		assertEqual(0, Socket::read(pipi.GetReadFd(), hallo, 6)); // EOF

		pipi.ShutDownReading();
		assertEqual(-1L, pipi.GetReadFd());
		t_assert(!pipi.IsReadyForReading(0));
	} else {
		t_assertm(false, "couldn't create anonymous pipe");
	}
}

void PipeTest::defaultConstructorTest() {
	StartTrace(PipeTest.defaultConstructorTest);

	Pipe pipi; // timeout 0.5 sec

	t_assert(-1 < pipi.GetReadFd());
	t_assert(-1 < pipi.GetWriteFd());

	t_assert(!pipi.IsReadyForReading(10));
	t_assert(pipi.HadTimeout()); // expect a time out
	t_assert(pipi.IsReadyForWriting(0));
	// write something
	assertEqual(5L, (long)Socket::write(pipi.GetWriteFd(), "hallo", 5));
	t_assert(pipi.IsReadyForReading(0));

	char hallo[6]; // PS: never do this in real code!!!
	long bytesread = Socket::read(pipi.GetReadFd(), hallo, 6);
	assertEqual(5, bytesread);
	hallo[5] = '\0';
	assertCharPtrEqual("hallo", hallo);

	t_assert(pipi.IsReadyForWriting(10));

	t_assert(Socket::SetToNonBlocking(pipi.GetReadFd()));
	bytesread = Socket::read(pipi.GetReadFd(), hallo, 6);
	assertEqual(-1, bytesread);

	pipi.ShutDownReading();
	assertEqual(-1L, pipi.GetReadFd());
	t_assert(!pipi.IsReadyForReading(0));

	pipi.ShutDownWriting();
	assertEqual(-1L, pipi.GetWriteFd());
	t_assert(!pipi.IsReadyForWriting(0));
}

// builds up a suite of testcases, add a line for each testmethod
Test *PipeTest::suite() {
	StartTrace(PipeTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, PipeTest, simpleConstructorTest);
	ADD_CASE(testSuite, PipeTest, defaultConstructorTest);
	ADD_CASE(testSuite, PipeTest, simpleBlockingTest);

	return testSuite;
}
