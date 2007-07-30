/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringStreamTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "StringStream.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SysLog.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#else
#include <limits.h>
#endif

//--- c-library modules used ---------------------------------------------------

const char *const StringStreamTest::fgcContent =
	"This is the content of a Test String\n"
	"it consists of several lines of simple Text\n"
	"and will be written to a string using the StringStream iostream\n";

StringStreamTest::StringStreamTest(TString tname) : TestCaseType(tname)
{};

StringStreamTest::~StringStreamTest() {}

void StringStreamTest::OperatorShiftLeftWithReadBuf()
{
	String testContent("hank");
	IStringStream is(testContent);
	if (t_assert(is.good())) {
		OStringStream oss;
		t_assert(oss.good());
		t_assert(is.rdbuf());
		oss.operator << (is.rdbuf());
		t_assert(oss.good());
		assertEqual(testContent, oss.str());
	}
}

void StringStreamTest::SimpleWrite()
// what: check a few properties of a simple DoScramble-DoUnscramble cycle
{
	// note this can be a problem if more than one process runs this test
	OStringStream os;
	t_assert(os.good() != 0);
	os << fgcContent << flush;
	assertCharPtrEqual(fgcContent, os.str());
	t_assert(os.good() != 0); // some error should occur, eof or fail or bad
	assertEqual(0, os.rdstate()); // some error should occur, eof or fail or bad
	// should check if file really exists just with low level syscalls and fcts
	assertEqual(strlen(fgcContent), os.str().Length());
	// now check for simple stream output with put
	OStringStream os2;
	os2.put(' ').put('H').write("allo", 4);
	os2.flush();
	String testit;
	testit.Append(" Hallo");
	assertCharPtrEqual(testit, os2.str());
	assertEqual(testit.Length(), os2.str().Length());

} // SimpleWrite

void StringStreamTest::SimpleRead()
// what:
{
	String s(fgcContent);
	IStringStream is(s);
	t_assert(is.good() != 0);
	const char *pc = fgcContent;
	char c;
	while ((!!is.get(c)) && (c == *pc++))
		; // compare content
	assertEqual(pc - fgcContent, strlen(fgcContent)); // have we reached the end?
	t_assert(is.eof() != 0);
} // SimpleRead

void StringStreamTest::SimpleAppend()
// what:
{

} // SimpleAppend

void StringStreamTest::SimpleAtEnd()
// what:
{

} // SimpleAtEnd

void StringStreamTest::SimpleSeek()
// what:
{
	const long searchpos = 10000; // larger as a memory page
	OStringStream os;
	t_assert(os.good() != 0);
	os << fgcContent; // put something into it.
	os.seekp(searchpos); // force enlargement
	os << fgcContent << flush;
	long len = os.str().Length();
	assertEqual(searchpos + strlen(fgcContent), len);

} // SimpleSeek

void StringStreamTest::SimpleOStringStreamWriteTest()
// what:
{
	String sample("A pretty good thing: ");
	OStringStream os(&sample);

	String a("It works now");

	os << a;

	assertEqual("A pretty good thing: It works now", sample); //BIA: was kommt hier raus ??

} // SimpleSeek

void StringStreamTest::WriteAnyOnString()
{
	Anything any;
	String expected("{\n");

	for (long i = 0; i < 200; i++) {
		String s;
		s << i;
		expected << "  /\"" << i << "\" ";

		if (i % 2 == 0) {
			any[s] = i;
			expected << i << "\n";
		} else {
			any[s] = s;
			expected << "\"" << i << "\"\n";
		} // if
	} // for
	expected << "}";

	String out;
	OStringStream os(&out);

	os << any;
	os.flush();

	assertEqual( '}', expected[expected.Length() - 1L] );
	assertEqual(expected, out);
	assertEqual(expected.Length(), out.Length());

} // WriteAnyOnString

void StringStreamTest::ReadFromAndWriteToStringTest()
{
	String orig("{/Total-{/Time-103/Nr-1000000/Sum-90/Max-1/Min-1/Error-0}"
				"/Results-{{/Nr-100000/Sum-9/Max-1/Min-1/Error-0}"
				"{/Nr-100000/Sum-9/Max-1/Min-1/Error-0}{/Nr-100000"
				"/Sum-9/Max-1/Min-1/Error-0}{/Nr-100000/Sum-9/Max-1"
				"/Min-1/Error-0}{/Nr-100000/Sum-9/Max-1/Min-1/Error-0}"
				"{/Nr-100000/Sum-9/Max-1/Min-1/Error-0}{/Nr-100000/Sum-9"
				"/Max-1/Min-1/Error-0}{/Nr-100000/Sum-9/Max-1/Min-1/Error-0}"
				"{/Nr-100000/Sum-9/Max-1/Min-1/Error-0}{/Nr-100000/Sum-9/Max-1/Min-1/Error-0}}}");
	StringStream inOut;
	inOut << orig;
	inOut.flush();
	String kopie;
	inOut >> kopie;
	inOut.flush();
	assertEqual(orig, kopie);
}

void StringStreamTest::ReadFromAndWriteToAnythingTest()
{
	Anything a;
	istream *ifp = System::OpenStream("Test", "any");
	if (ifp == 0) {
		String logMsg;
		SysLog::Error(logMsg << "can't open file Test.any");
		t_assert(0);
		return;
	}
	a.Import(*ifp);
	StringStream inOut;
	a.PrintOn(inOut, false);
	inOut.flush();
	Anything b;
	b.Import(inOut);

	assertEqual(a["Results"].GetSize(), b["Results"].GetSize());

	delete ifp;
}

void StringStreamTest::ReadFromAndWriteToAnythingTest2()
{
	Anything a;
	istream *ifp = System::OpenStream("Test", "any");
	if (ifp == 0) {
		String logMsg;
		SysLog::Error(logMsg << "can't open file Test.any");
		t_assert(0);
		return;
	}
	a.Import(*ifp);

	Anything a2 = a.DeepClone();
	a2["Results"]["dummy"];

	// write to StringStream
	StringStream inOut;
	a.PrintOn(inOut, false);
	inOut.flush();

	// read from StringStream
	Anything b;
	b.Import(inOut);

	// write again
	a2.PrintOn(inOut, false);
	inOut.flush();

	// read again from StringStream
	Anything c;
	c.Import(inOut);

	assertEqual(a["Results"].GetSize(), b["Results"].GetSize());
	assertEqual(a2["Results"].GetSize(), c["Results"].GetSize());

	delete ifp;
}

// check formatting problems:
void StringStreamTest::testformat(const String &source, long expect, bool mustfail, const TString &msg)
{
	IStringStream is(source);
	long l = -1;
	is >> l;
	assertEqualm(expect, l, msg);
	t_assertm((mustfail != 0) == (is.fail() != 0), msg);
}

void StringStreamTest::testformat(const String &source, double expect, bool mustfail, const TString &msg)
{
	IStringStream is(source);

	double l = -1;
	is >> l;
	assertDoublesEqualm(expect, l, 0.0000001, msg);
	t_assertm((mustfail != 0) == (is.fail() != 0), msg);
}

void StringStreamTest::FormatTests()
{
	testformat("1212", 1212L, false, "test good long conversion");
	testformat("gaga", -1L, true, "test bad long conversion");
	testformat("1212.0", 1212.0, false, "test good double conversion");
	testformat("1.05e4", 10500.0, false, "test good double conversion");
	testformat("gaga", -1.0, true, "test bad double conversion");
}

void StringStreamTest::OperatorShiftLeftWithLongLong()
{
	OStringStream ostr, ostr2;
	ostr << LLONG_MAX;
	assertCharPtrEqual("9223372036854775807", ostr.str());
	ostr2 << LLONG_MIN;
	assertCharPtrEqual("-9223372036854775808", ostr2.str());
}

void StringStreamTest::OperatorShiftLeftWithUnsignedLongLong()
{
	OStringStream ostr, ostr2;
	ostr << UINT64_LITERAL(0xFFFFFFFFFFFFFFFF);
	assertCharPtrEqual("18446744073709551615", ostr.str());
}

void StringStreamTest::OperatorShiftRightWithLongLong()
{
	{
		StringStream stream;
		stream << LLONG_MAX << flush;
		l_long llVal = INT64_LITERAL(-3);
		stream >> llVal;
		t_assert(LLONG_MAX == llVal);
	}
	{
		StringStream stream;
		stream << LLONG_MIN << flush;
		l_long llVal = INT64_LITERAL(+5);
		stream >> llVal;
		t_assert(LLONG_MIN == llVal);
	}
}

void StringStreamTest::OperatorShiftRightWithUnsignedLongLong()
{
	{
		StringStream stream;
		stream << ULLONG_MAX << flush;
		ul_long ullVal = UINT64_LITERAL(1111);
		stream >> ullVal;
		t_assert(ULLONG_MAX == ullVal);
	}
}

Test *StringStreamTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StringStreamTest, SimpleWrite);
	ADD_CASE(testSuite, StringStreamTest, FormatTests);
	ADD_CASE(testSuite, StringStreamTest, SimpleRead);
	ADD_CASE(testSuite, StringStreamTest, SimpleAppend);
	ADD_CASE(testSuite, StringStreamTest, SimpleAtEnd);
	ADD_CASE(testSuite, StringStreamTest, SimpleSeek);
	ADD_CASE(testSuite, StringStreamTest, SimpleOStringStreamWriteTest);
	ADD_CASE(testSuite, StringStreamTest, WriteAnyOnString);
	ADD_CASE(testSuite, StringStreamTest, ReadFromAndWriteToStringTest);
	ADD_CASE(testSuite, StringStreamTest, ReadFromAndWriteToAnythingTest);
	ADD_CASE(testSuite, StringStreamTest, OperatorShiftLeftWithReadBuf);
	ADD_CASE(testSuite, StringStreamTest, OperatorShiftLeftWithLongLong);
	ADD_CASE(testSuite, StringStreamTest, OperatorShiftLeftWithUnsignedLongLong);
	ADD_CASE(testSuite, StringStreamTest, OperatorShiftRightWithLongLong);
	ADD_CASE(testSuite, StringStreamTest, OperatorShiftRightWithUnsignedLongLong);
	return testSuite;
}
