/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- interface include --------------------------------------------------------
#include "StringTokenizerTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"

//--- c-library modules used ---------------------------------------------------

StringTokenizerTest::StringTokenizerTest (TString tname) : TestCase(tname)
{
}

void StringTokenizerTest::setUp ()
{
	fShort = "short";
	fLong = "a somewhat longer string";
}

Test *StringTokenizerTest::worksuite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(StringTokenizerTest, constructors));

	return testSuite;
}

Test *StringTokenizerTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(StringTokenizerTest, constructors));
	testSuite->addTest (NEW_CASE(StringTokenizerTest, constrMethodsAll));
	testSuite->addTest (NEW_CASE(StringTokenizerTest, nextTokenNormal));
	testSuite->addTest (NEW_CASE(StringTokenizerTest, nextTokenEmpty));
	testSuite->addTest (NEW_CASE(StringTokenizerTest, getRemainder));
	testSuite->addTest (NEW_CASE(StringTokenizerTest, resetTest));

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
	t_assert( token == "" );	// ????  Empty token if String begins with a delimiter
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "12345" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "6789" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "123" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "456" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "789" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "123" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "4567" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "9" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "12" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "12" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "12" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods1 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '1' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '1' );

	ret = st.NextToken(token);
	t_assert( token == "0" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "2345067890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "23045607890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "2304567090" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "2" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "2" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "2" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods2 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '2' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '2' );

	ret = st.NextToken(token);
	t_assert( token == "01" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "3450678901" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "30456078901" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "3045670901" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );		// PT: new version has correct behavior
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods3 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '3' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '3' );

	ret = st.NextToken(token);
	t_assert( token == "012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "4506789012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "04560789012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0456709012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0456709012" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "0456709012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods4 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '4' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '4' );

	ret = st.NextToken(token);
	t_assert( token == "0123" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "50678901230" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "56078901230" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "56709012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "56709012" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "56709012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods5 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '5' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '5' );

	ret = st.NextToken(token);
	t_assert( token == "01234" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "06789012304" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "60789012304" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "6709012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "6709012" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "6709012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods6 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '6' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '6' );

	ret = st.NextToken(token);
	t_assert( token == "0123450" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "7890123045" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "07890123045" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "709012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "709012" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "709012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods7 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '7' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '7' );

	ret = st.NextToken(token);
	t_assert( token == "01234506" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "89012304560" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "8901230456" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "09012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "09012" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "09012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods8 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '8' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '8' );

	ret = st.NextToken(token);
	t_assert( token == "012345067" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "90123045607" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "901230456709012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "901230456709012" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "901230456709012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods9 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", '9' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == '9' );

	ret = st.NextToken(token);
	t_assert( token == "0123450678" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "01230456078" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0123045670" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "012" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods10 ()
{
	String token;
	bool ret;

	StringTokenizer st("0123450678901230456078901230456709012", 'a' );
	assertEqual( st.fString, "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiter == 'a' );

	ret = st.NextToken(token);
	// t_assert( token == "" );
	// t_assert( ret == false );  // ?????  Token found even if the string does not contain the delimiter-char
	t_assert( token == "0123450678901230456078901230456709012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0123450678901230456078901230456709012" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods11 ()
{
	String token;
	bool ret;

	StringTokenizer st( "", 'a' );

	assertEqual( st.fString, "" );
	t_assert( st.fDelimiter ==  'a' );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizerTest::constrMethods12 ()
{
	String token;
	bool ret;

	StringTokenizer st( "abc", 0x0 );

	assertEqual( st.fString, "abc" );
	t_assert( st.fDelimiter ==  0x0);

	ret = st.NextToken(token);
	t_assert( token == "abc" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "abc" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "abc" );
	t_assert( ret == false );
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
	t_assert( token == "foo" );
	t_assert( ret == true );

	// second round
	ret = st0.NextToken(token);
	t_assert( token == "bar" );
	t_assert( ret == true );

	// third round
	ret = st0.NextToken(token);
	t_assert( token == "bar" );
	t_assert( ret == false );

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
	t_assert( token == "" );
	t_assert( ret == false );

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
	t_assert( token == "foo" );
	t_assert( ret == true );

	// second round
	ret = st0.NextToken(token);
	t_assert( token == "bar" );
	t_assert( ret == true );

	// third round
	ret = st0.NextToken(token);
	t_assert( token == "bar" );
	t_assert( ret == false );

	st0.Reset();

	// first round
	ret = st0.NextToken(token);
	t_assert( token == "foo" );
	t_assert( ret == true );

	// second round
	ret = st0.NextToken(token);
	t_assert( token == "bar" );
	t_assert( ret == true );

	// third round
	ret = st0.NextToken(token);
	t_assert( token == "bar" );
	t_assert( ret == false );

}

void StringTokenizerTest::getRemainder()
{
	StringTokenizer st0("foo:bar:", ':');

	String remainder, token;

	// first round
	remainder = st0.GetRemainder();
	t_assert( remainder == "foo:bar:" );
	remainder = st0.GetRemainder(true);
	t_assert( remainder == "foo:bar:" );

	// second round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	t_assert( remainder == "bar:" );
	remainder = st0.GetRemainder(true);
	t_assert( remainder == ":bar:" );

	// third round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	t_assert( remainder == "" );
	remainder = st0.GetRemainder(true);
	t_assert( remainder == ":" );

	// fourth round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	t_assert( remainder == "" );
	remainder = st0.GetRemainder(true);
	t_assert( remainder == "" );
}
