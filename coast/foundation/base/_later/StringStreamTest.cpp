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

#include "StringStreamTest.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "SystemLog.h"

using namespace coast;

#include <limits>

typedef std::numeric_limits<long long> ll_limits;
typedef std::numeric_limits<unsigned long long> ull_limits;

const char * const StringStreamTest::fgcContent = "This is the content of a Test String\n"
		"it consists of several lines of simple Text\n"
		"and will be written to a string using the StringStream iostream\n";

void StringStreamTest::OperatorShiftLeftWithReadBuf() {
	String testContent("hank");
	IStringStream is(testContent);
	ASSERT(is.good());
	OStringStream oss;
	ASSERT(oss.good());
	ASSERT(is.rdbuf());
	oss.operator <<(is.rdbuf());
	ASSERT(oss.good());
	ASSERT_EQUAL(testContent, oss.str());
}

void StringStreamTest::SimpleWrite()
// what: check a few properties of a simple DoScramble-DoUnscramble cycle
{
	// note this can be a problem if more than one process runs this test
	OStringStream os;
	ASSERT(os.good() != 0);
	os << fgcContent << std::flush;
	ASSERT_EQUAL(fgcContent, os.str());
	ASSERT(os.good() != 0); // some error should occur, eof or fail or bad
	ASSERT_EQUAL(0, os.rdstate()); // some error should occur, eof or fail or bad
	// should check if file really exists just with low level syscalls and fcts
	ASSERT_EQUAL(strlen(fgcContent), os.str().Length());
	// now check for simple stream output with put
	OStringStream os2;
	os2.put(' ').put('H').write("allo", 4);
	os2.flush();
	String testit;
	testit.Append(" Hallo");
	ASSERT_EQUAL(testit, os2.str());
	ASSERT_EQUAL(testit.Length(), os2.str().Length());

} // SimpleWrite

void StringStreamTest::SimpleRead()
// what:
{
	String s(fgcContent);
	IStringStream is(s);
	ASSERT(is.good() != 0);
	const char *pc = fgcContent;
	char c;
	while ((!!is.get(c)) && (c == *pc++))
		; // compare content
	ASSERT_EQUAL(pc - fgcContent, strlen(fgcContent)); // have we reached the end?
	ASSERT(is.eof() != 0);
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
	ASSERT(os.good() != 0);
	os << fgcContent; // put something into it.
	os.seekp(searchpos); // force enlargement
	os << fgcContent << std::flush;
	long len = os.str().Length();
	ASSERT_EQUAL(searchpos + strlen(fgcContent), len);

} // SimpleSeek

void StringStreamTest::SimpleOStringStreamWriteTest()
// what:
{
	String sample("A pretty good thing: ");
	OStringStream os(&sample);

	String a("It works now");

	os << a;

	ASSERT_EQUAL("A pretty good thing: It works now", sample); //BIA: was kommt hier raus ??

} // SimpleSeek

void StringStreamTest::WriteAnyOnString() {
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

	ASSERT_EQUAL('}', expected[expected.Length() - 1L]);
	ASSERT_EQUAL(expected, out);
	ASSERT_EQUAL(expected.Length(), out.Length());

} // WriteAnyOnString

void StringStreamTest::ReadFromAndWriteToStringTest() {
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
	ASSERT_EQUAL(orig, kopie);
}

void StringStreamTest::ReadFromAndWriteToAnythingTest() {
	Anything a;
	std::istream *ifp = system::OpenStream("Test", "any");
	if (ifp == 0) {
		String logMsg;
		SystemLog::Error(logMsg << "can't open file Test.any");
		ASSERTM( (const char *) logMsg,0);
		return;
	}
	a.Import(*ifp);
	StringStream inOut;
	a.PrintOn(inOut, false);
	inOut.flush();
	Anything b;
	b.Import(inOut);

	ASSERT_EQUAL(a["Results"].GetSize(), b["Results"].GetSize());

	delete ifp;
}

void StringStreamTest::ReadFromAndWriteToAnythingTest2() {
	Anything a;
	std::istream *ifp = system::OpenStream("Test", "any");
	if (ifp == 0) {
		String logMsg;
		SystemLog::Error(logMsg << "can't open file Test.any");
		ASSERTM( (const char *) logMsg,0);
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

	ASSERT_EQUAL(a["Results"].GetSize(), b["Results"].GetSize());
	ASSERT_EQUAL(a2["Results"].GetSize(), c["Results"].GetSize());

	delete ifp;
}

// check formatting problems:
void StringStreamTest::testformat(const String &source, long expect, bool mustfail, const String &msg) {
	IStringStream is(source);
	long l = -1;
	is >> l;
	ASSERT_EQUALM( msg,expect, l);
	ASSERTM( msg,(mustfail != 0) == (is.fail() != 0));
}

void StringStreamTest::testformat(const String &source, double expect, bool mustfail, const String &msg) {
	IStringStream is(source);

	double l = -1;
	is >> l;
	ASSERT_EQUAL_DELTAM(expect, l, 0.0000001, msg);
	ASSERTM( msg,(mustfail != 0) == (is.fail() != 0));
}

void StringStreamTest::FormatTests() {
	testformat("1212", 1212L, false, "test good long conversion");
	testformat("gaga", -1L, true, "test bad long conversion");
	testformat("1212.0", 1212.0, false, "test good double conversion");
	testformat("1.05e4", 10500.0, false, "test good double conversion");
	testformat("gaga", -1.0, true, "test bad double conversion");
}

void StringStreamTest::OperatorShiftLeftWithLongLong() {
	OStringStream ostr, ostr2;
	ostr << ll_limits::max();
	ASSERT_EQUAL("9223372036854775807", ostr.str());
	ostr2 << ll_limits::min();
	ASSERT_EQUAL("-9223372036854775808", ostr2.str());
}

void StringStreamTest::OperatorShiftLeftWithUnsignedLongLong() {
	OStringStream ostr, ostr2;
	ostr << UINT64_LITERAL(0xFFFFFFFFFFFFFFFF);
	ASSERT_EQUAL("18446744073709551615", ostr.str());
}

void StringStreamTest::OperatorShiftRightWithLongLong() {
	{
		StringStream stream;
		stream << ll_limits::max() << std::flush;
		l_long llVal = INT64_LITERAL(-3);
		stream >> llVal;
		ASSERT(ll_limits::max() == llVal);
	}
	{
		StringStream stream;
		stream << ll_limits::min() << std::flush;
		l_long llVal = INT64_LITERAL(+5);
		stream >> llVal;
		ASSERT(ll_limits::min() == llVal);
	}
}

void StringStreamTest::OperatorShiftRightWithUnsignedLongLong() {
	{
		StringStream stream;
		stream << ull_limits::max() << std::flush;
		ul_long ullVal = UINT64_LITERAL(1111);
		stream >> ullVal;
		ASSERT(ull_limits::max() == ullVal);
	}
}

void StringStreamTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(StringStreamTest, SimpleWrite));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, FormatTests));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, SimpleRead));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, SimpleAppend));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, SimpleAtEnd));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, SimpleSeek));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, SimpleOStringStreamWriteTest));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, WriteAnyOnString));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, ReadFromAndWriteToStringTest));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, ReadFromAndWriteToAnythingTest));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, OperatorShiftLeftWithReadBuf));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, OperatorShiftLeftWithLongLong));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, OperatorShiftLeftWithUnsignedLongLong));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, OperatorShiftRightWithLongLong));
	s.push_back(CUTE_SMEMFUN(StringStreamTest, OperatorShiftRightWithUnsignedLongLong));
}

