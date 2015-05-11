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

#include "StringTokenizerTest.h"

StringTokenizerTest::StringTokenizerTest() : fShort("short"), fLong("a somewhat longer string") {
}

void StringTokenizerTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(StringTokenizerTest, constructors));
	s.push_back(CUTE_SMEMFUN(StringTokenizerTest, constrMethodsAll));
	s.push_back(CUTE_SMEMFUN(StringTokenizerTest, nextTokenNormal));
	s.push_back(CUTE_SMEMFUN(StringTokenizerTest, nextTokenEmpty));
	s.push_back(CUTE_SMEMFUN(StringTokenizerTest, getRemainder));
	s.push_back(CUTE_SMEMFUN(StringTokenizerTest, resetTest));
}

void StringTokenizerTest::constrMethods0 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '0' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '0' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// ????  Empty token if String begins with a delimiter
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("12345", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("6789", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("123", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("456", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("789", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("123", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("4567", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("9", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("12", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("12", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("12", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods1 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '1' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '1' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("0", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2345067890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("23045607890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2304567090", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods2 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '2' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '2' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("01", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("3450678901", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("30456078901", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("3045670901", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);		// PT: new version has correct behavior
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods3 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '3' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '3' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("4506789012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("04560789012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0456709012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0456709012", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0456709012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods4 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '4' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '4' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("0123", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("50678901230", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("56078901230", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("56709012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("56709012", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("56709012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods5 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '5' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '5' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("01234", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("06789012304", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("60789012304", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("6709012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("6709012", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("6709012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods6 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '6' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '6' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("0123450", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("7890123045", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("07890123045", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("709012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("709012", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("709012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods7 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '7' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '7' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("01234506", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("89012304560", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("8901230456", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("09012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("09012", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("09012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods8 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '8' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '8' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("012345067", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("90123045607", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("901230456709012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("901230456709012", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("901230456709012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods9 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '9' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == '9' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("0123450678", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("01230456078", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0123045670", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("012", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods10 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", 'a' );
	ASSERT_EQUAL( st.fString, "0123450678901230456078901230456709012" );
	ASSERT( st.fDelimiter == 'a' );

	ret = st.NextToken(token);
	// ASSERT_EQUAL("", token);
	// ASSERT_EQUAL(false, ret);  // ?????  Token found even if the string does not contain the delimiter-char
	ASSERT_EQUAL("0123450678901230456078901230456709012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0123450678901230456078901230456709012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods11 ()
{
	String token;
	bool ret;

	StringTokenizer st( "", 'a' );

	ASSERT_EQUAL( st.fString, "" );
	ASSERT( st.fDelimiter ==  'a' );

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizerTest::constrMethods12 ()
{
	String token;
	bool ret;

	StringTokenizer st( "abc", 0x0 );

	ASSERT_EQUAL( st.fString, "abc" );
	ASSERT( st.fDelimiter ==  0x0);

	ret = st.NextToken(token);
	ASSERT_EQUAL("abc", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("abc", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("abc", token);
	ASSERT_EQUAL(false, ret);
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

	ASSERT_EQUAL( st0.fString, "foo:bar" );
	ASSERT( st0.fDelimiter == ':' );
}

void StringTokenizerTest::nextTokenNormal ()
{

	StringTokenizer st0("foo:bar", ':');

	// precondition
	ASSERT_EQUAL( st0.fString, "foo:bar" );
	ASSERT( st0.fDelimiter == ':' );

	String token;
	bool ret;

	// first round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("foo", token);
	ASSERT_EQUAL(true, ret);

	// second round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("bar", token);
	ASSERT_EQUAL(true, ret);

	// third round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("bar", token);
	ASSERT_EQUAL(false, ret);

}

void StringTokenizerTest::nextTokenEmpty ()
{

	StringTokenizer st0("", ':');

	// precondition
	ASSERT_EQUAL( st0.fString, "" );
	ASSERT( st0.fDelimiter == ':' );

	String token;
	bool ret;

	// first round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

}

void StringTokenizerTest::resetTest ()
{

	StringTokenizer st0("foo:bar", ':');

	// precondition
	ASSERT_EQUAL( st0.fString, "foo:bar" );
	ASSERT( st0.fDelimiter == ':' );

	String token;
	bool ret;

	// first round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("foo", token);
	ASSERT_EQUAL(true, ret);

	// second round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("bar", token);
	ASSERT_EQUAL(true, ret);

	// third round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("bar", token);
	ASSERT_EQUAL(false, ret);

	st0.Reset();

	// first round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("foo", token);
	ASSERT_EQUAL(true, ret);

	// second round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("bar", token);
	ASSERT_EQUAL(true, ret);

	// third round
	ret = st0.NextToken(token);
	ASSERT_EQUAL("bar", token);
	ASSERT_EQUAL(false, ret);

}

void StringTokenizerTest::getRemainder()
{
	StringTokenizer st0("foo:bar:", ':');

	String remainder, token;//lint !e578

	// first round
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("foo:bar:", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL("foo:bar:", remainder);

	// second round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("bar:", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(":bar:", remainder);

	// third round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(":", remainder);

	// fourth round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL("", remainder);
}
