/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "PipeStreamTest.h"

//--- module under test --------------------------------------------------------
#include "PipeStream.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------
#include "Pipe.h"
#include "Dbg.h"

//---- PipeStreamTest ----------------------------------------------------------------
PipeStreamTest::PipeStreamTest(TString className)
	: TestCase(className)
{
	StartTrace(PipeStreamTest.Ctor);
}

PipeStreamTest::~PipeStreamTest()
{
	StartTrace(PipeStreamTest.Dtor);
}

// setup for this TestCase
void PipeStreamTest::setUp ()
{
	StartTrace(PipeStreamTest.setUp);
} // setUp

void PipeStreamTest::tearDown ()
{
	StartTrace(PipeStreamTest.tearDown);
} // tearDown

void PipeStreamTest::SimpleWriteandRead()
{
	StartTrace(PipeStreamTest.SimpleWriteandRead);

	Pipe pipi;
	iostream *stream = pipi.GetStream();

	t_assert(stream != NULL);
	(*stream) << "hallo" << flush;
	String h;
	(*stream) >> h;
	assertEqual("hallo", h);
	pipi.ShutDownWriting();
	(*stream) << "hallo" << flush;
	t_assert(!stream->good()); // timeout ?
	stream->clear();
}

void PipeStreamTest::MoreWriteandRead()
{
	StartTrace(PipeStreamTest.MoreWriteandRead);

	Pipe pipi(5000L);
	iostream *stream = pipi.GetStream();

	t_assert(stream != NULL);
	if (stream) {
		pipi.SetTimeout(0);
		t_assert(pipi.IsReadyForWriting());
		(*stream) << "hallo" << endl; // this might set IsReadyForWriting false
		(*stream) << "peter" << flush;
		stream->clear();
		String h;
		(*stream) >> h;
		assertEqual("hallo", h);
		pipi.ShutDownWriting();
		(*stream) << "hallo" << flush;
		t_assert(!stream->good()); // timeout ?
		stream->clear();
	}
}

void PipeStreamTest::LoopWriteandRead()
{
	StartTrace(PipeStreamTest.LoopWriteandRead);

	Pipe pipi;
	iostream *stream = pipi.GetStream();
	const long cBufSz = 4000;
	String buf(cBufSz);
	buf << "hallo2\n";

	t_assert(stream != NULL);
	if (stream) {
		pipi.SetTimeout(0);
		t_assert(pipi.IsReadyForWriting());
		do {
			stream->write(buf, cBufSz);
		} while (stream->good());

		t_assert(!pipi.IsReadyForWriting());
		stream->clear();
		String h(cBufSz);
		String expected( "hallo2\n" );
		while (stream->read((char *)(const char *)h, cBufSz)) {
			assertEqual(expected, h);
		}

		// now flush again

		t_assert(pipi.IsReadyForWriting());
		(*stream) << flush;
		while (stream->read((char *)(const char *)h, cBufSz)) {
			assertEqual(expected, h);
		}
		pipi.ShutDownWriting();
		(*stream) << "hallo" << flush;
		t_assert(stream->fail() != 0); // timeout ?
		stream->clear();
	}
}

void PipeStreamTest::PipeAndFork()
{
	StartTrace(PipeStreamTest.PipeAndFork);
}

// builds up a suite of testcases, add a line for each testmethod
Test *PipeStreamTest::suite ()
{
	StartTrace(PipeStreamTest.suite);
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(PipeStreamTest, SimpleWriteandRead));
	testSuite->addTest (NEW_CASE(PipeStreamTest, MoreWriteandRead));
	testSuite->addTest (NEW_CASE(PipeStreamTest, LoopWriteandRead));
	testSuite->addTest (NEW_CASE(PipeStreamTest, PipeAndFork));

	return testSuite;
}
