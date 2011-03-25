/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "PipeStreamTest.h"
#include "PipeStream.h"
#include "TestSuite.h"
#include "Pipe.h"
#include "Dbg.h"

//---- PipeStreamTest ----------------------------------------------------------------
PipeStreamTest::PipeStreamTest(TString className)
	: TestCaseType(className)
{
	StartTrace(PipeStreamTest.Ctor);
}

PipeStreamTest::~PipeStreamTest()
{
	StartTrace(PipeStreamTest.Dtor);
}

void PipeStreamTest::SimpleWriteandRead()
{
	StartTrace(PipeStreamTest.SimpleWriteandRead);

	Pipe pipi;
	std::iostream *stream = pipi.GetStream();

	t_assert(stream != NULL);
	(*stream) << "hallo" << std::flush;
	String h;
	(*stream) >> h;
	assertEqual("hallo", h);
	pipi.ShutDownWriting();
	(*stream) << "hallo" << std::flush;
	t_assert(!stream->good()); // timeout ?
	stream->clear();
}

void PipeStreamTest::MoreWriteandRead()
{
	StartTrace(PipeStreamTest.MoreWriteandRead);

	Pipe pipi(5000L);
	std::iostream *stream = pipi.GetStream();

	t_assert(stream != NULL);
	if (stream) {
		pipi.SetTimeout(0);
		t_assert(pipi.IsReadyForWriting());
		(*stream) << "hallo" << std::endl; // this might set IsReadyForWriting false
		(*stream) << "peter" << std::flush;
		stream->clear();
		String h;
		(*stream) >> h;
		assertEqual("hallo", h);
		pipi.ShutDownWriting();
		(*stream) << "hallo" << std::flush;
		t_assert(!stream->good()); // timeout ?
		stream->clear();
	}
}

void PipeStreamTest::LoopWriteandRead()
{
	StartTrace(PipeStreamTest.LoopWriteandRead);

	Pipe pipi;
	std::iostream *stream = pipi.GetStream();
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
		(*stream) << std::flush;
		while (stream->read((char *)(const char *)h, cBufSz)) {
			assertEqual(expected, h);
		}
		pipi.ShutDownWriting();
		(*stream) << "hallo" << std::flush;
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

	ADD_CASE(testSuite, PipeStreamTest, SimpleWriteandRead);
	ADD_CASE(testSuite, PipeStreamTest, MoreWriteandRead);
	ADD_CASE(testSuite, PipeStreamTest, LoopWriteandRead);
	ADD_CASE(testSuite, PipeStreamTest, PipeAndFork);

	return testSuite;
}
