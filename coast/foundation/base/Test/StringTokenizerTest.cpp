/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestSuite.h"
#include "StringTokenizerTest.h"

StringTokenizerTest::StringTokenizerTest (TString tname) : TestCaseType(tname)
{
}

void StringTokenizerTest::setUp ()
{
	fShort = "short";
	fLong = "a somewhat longer string";
}

Test *StringTokenizerTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, StringTokenizerTest, constructors);
	ADD_CASE(testSuite, StringTokenizerTest, constrMethodsAll);
	ADD_CASE(testSuite, StringTokenizerTest, nextTokenNormal);
	ADD_CASE(testSuite, StringTokenizerTest, nextTokenEmpty);
	ADD_CASE(testSuite, StringTokenizerTest, getRemainder);
	ADD_CASE(testSuite, StringTokenizerTest, resetTest);

	return testSuite;
}
void StringTokenizerTest::constrMethods0 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '0' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '0' );

	ret = st.NextToken(token);
	assertEqual("", token);	// ????  Empty token if String begins with a delimiter
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("12345", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("6789", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("123", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("456", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("789", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("123", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("4567", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("9", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("12", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("12", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("12", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods1 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '1' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '1' );

	ret = st.NextToken(token);
	assertEqual("0", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("2345067890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("23045607890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("2304567090", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("2", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("2", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("2", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods2 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '2' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '2' );

	ret = st.NextToken(token);
	assertEqual("01", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("3450678901", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("30456078901", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("3045670901", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);		// PT: new version has correct behavior
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods3 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '3' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '3' );

	ret = st.NextToken(token);
	assertEqual("012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("4506789012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("04560789012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0456709012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0456709012", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("0456709012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods4 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '4' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '4' );

	ret = st.NextToken(token);
	assertEqual("0123", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("50678901230", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("56078901230", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("56709012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("56709012", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("56709012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods5 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '5' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '5' );

	ret = st.NextToken(token);
	assertEqual("01234", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("06789012304", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("60789012304", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("6709012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("6709012", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("6709012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods6 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '6' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '6' );

	ret = st.NextToken(token);
	assertEqual("0123450", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("7890123045", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("07890123045", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("709012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("709012", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("709012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods7 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '7' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '7' );

	ret = st.NextToken(token);
	assertEqual("01234506", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("89012304560", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("8901230456", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("09012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("09012", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("09012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods8 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '8' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '8' );

	ret = st.NextToken(token);
	assertEqual("012345067", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("90123045607", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("901230456709012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("901230456709012", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("901230456709012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods9 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '9' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '9' );

	ret = st.NextToken(token);
	assertEqual("0123450678", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("01230456078", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0123045670", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("012", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods10 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", 'a' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == 'a' );

	ret = st.NextToken(token);
	// assertEqual("", token);
	// assertEqual(false, ret);  // ?????  Token found even if the string does not contain the delimiter-char
	assertEqual("0123450678901230456078901230456709012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0123450678901230456078901230456709012", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods11 ()
{
	String token;
	bool ret;

	StringTokenizer st( "", 'a' );

	assertEqual( st.fString, "" );
	t_assert( st.fDelimiter ==  'a' );

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethods12 ()
{
	String token;
	bool ret;

	StringTokenizer st( "abc", 0x0 );

	assertEqual( st.fString, "abc" );
	t_assert( st.fDelimiter ==  0x0);

	ret = st.NextToken(token);
	assertEqual("abc", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("abc", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("abc", token);
	assertEqual(false, ret);
}

void StringTokenizerTest::constrMethodsAll ()
{
	constrMethods0 ();
	constrMethods1 ();
	constrMethods2 ();
	constrMethods3 ();
	constrMethods4 ();
	constrMethods5 ();
	constrMethods6 ();
	constrMethods7 ();
	constrMethods8 ();
	constrMethods9 ();
	constrMethods10 ();
	constrMethods11 ();
	constrMethods12 ();
}

void StringTokenizerTest::constructors ()
{
	// StringTokenizer tests
	// normal use
	StringTokenizer st0("foo:bar", ':');

	assertEqual( st0.fString, "foo:bar" );
	t_assert( st0.fDelimiter == ':' );
}

void StringTokenizerTest::nextTokenNormal ()
{

	StringTokenizer st0("foo:bar", ':');

	// precondition
	assertEqual( st0.fString, "foo:bar" );
	t_assert( st0.fDelimiter == ':' );

	String token;
	bool ret;

	// first round
	ret = st0.NextToken(token);
	assertEqual("foo", token);
	assertEqual(true, ret);

	// second round
	ret = st0.NextToken(token);
	assertEqual("bar", token);
	assertEqual(true, ret);

	// third round
	ret = st0.NextToken(token);
	assertEqual("bar", token);
	assertEqual(false, ret);

}

void StringTokenizerTest::nextTokenEmpty ()
{

	StringTokenizer st0("", ':');

	// precondition
	assertEqual( st0.fString, "" );
	t_assert( st0.fDelimiter == ':' );

	String token;
	bool ret;

	// first round
	ret = st0.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

}

void StringTokenizerTest::resetTest ()
{

	StringTokenizer st0("foo:bar", ':');

	// precondition
	assertEqual( st0.fString, "foo:bar" );
	t_assert( st0.fDelimiter == ':' );

	String token;
	bool ret;

	// first round
	ret = st0.NextToken(token);
	assertEqual("foo", token);
	assertEqual(true, ret);

	// second round
	ret = st0.NextToken(token);
	assertEqual("bar", token);
	assertEqual(true, ret);

	// third round
	ret = st0.NextToken(token);
	assertEqual("bar", token);
	assertEqual(false, ret);

	st0.Reset();

	// first round
	ret = st0.NextToken(token);
	assertEqual("foo", token);
	assertEqual(true, ret);

	// second round
	ret = st0.NextToken(token);
	assertEqual("bar", token);
	assertEqual(true, ret);

	// third round
	ret = st0.NextToken(token);
	assertEqual("bar", token);
	assertEqual(false, ret);

}

void StringTokenizerTest::getRemainder()
{
	StringTokenizer st0("foo:bar:", ':');

	String remainder, token;//lint !e578

	// first round
	remainder = st0.GetRemainder();
	assertEqual("foo:bar:", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual("foo:bar:", remainder);

	// second round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	assertEqual("bar:", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(":bar:", remainder);

	// third round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	assertEqual("", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(":", remainder);

	// fourth round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	assertEqual("", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual("", remainder);
}
