/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringTokenizer2Test.h"

//--- module under test --------------------------------------------------------

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------

//--- c-library modules used ---------------------------------------------------

StringTokenizer2Test::StringTokenizer2Test (TString tname)
	: TestCaseType(tname)
{
}

void StringTokenizer2Test::setUp ()
{
	fShort = "short";
	fLong = "a somewhat longer string";
}

Test *StringTokenizer2Test::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StringTokenizer2Test, constrMethodsAll);
	ADD_CASE(testSuite, StringTokenizer2Test, getRemainder);
	return testSuite;
}

void StringTokenizer2Test::constrMethods0 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "0" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("0", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods1 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "1" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("1", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods2 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "2" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("2", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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
	assertEqual("3045670901", token);		// ????  Empty token if string begins with a delimiter BUT no empty token if the string ends with a delimiter
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("3045670901", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods3 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "3" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("3", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods4 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "4" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("4", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods5 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "5" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("5", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods6 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "6" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("6", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods7 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "7" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("7", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods8 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "8" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("8", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods9 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "9" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("9", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods10 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "01" );
	//t_assert( st.fString ==  "leer 0 leer 1 2345 0 6789 0 leer 1 23 0 456 0 789 0 leer 1 23 0 4567 0 9 0 leer 1 2" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("01", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("2345", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("6789", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("23", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("456", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("789", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("23", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("4567", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("9", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("2", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("2", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods11 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("01234506789012304560789010230456709012", "12" );
	assertEqual("01234506789012304560789010230456709012", st.fString);

	assertEqual("12", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("0", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("345067890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("3045607890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("304567090", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods12 ()
{
	String token;
	bool ret;

	StringTokenizer2 st(     "0123450678901230456078901230456709012", "23" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("23", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("01", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("450678901", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0456078901", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("045670901", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("045670901", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("045670901", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods13 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "012" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("012", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("", token);	// 0
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 1
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("345", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("6789", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 1
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("3", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("456", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("789", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 1
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("3", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("4567", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("9", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 1
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods14 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "123" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("123", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("0", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 3
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("45067890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 3
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("045607890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 3
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("04567090", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods15 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "111" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("111", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods16 ()
{
	String			token;
	bool			ret;
	unsigned int	i;

	StringTokenizer2 st("0123450678901230456078901230456709012", "0123456789" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("0123456789", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	for ( i = 0; i < strlen("0123450678901230456078901230456709012"); i++ ) {
		ret = st.NextToken(token);
		assertEqual("", token);
		assertEqual(true, ret);
	}

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods17 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "12345" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("12345", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("0", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 3
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 4
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 5
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("067890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 3
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 5
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("607890", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 3
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 5
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("67090", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);	// 2
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods18 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "abc" );
	assertEqual("0123450678901230456078901230456709012", st.fString);
	assertEqual("abc", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	// assertEqual("", token);
	// assertEqual(false, ret);  // ?????  Token found even if the string does not contain the delimiter-char
	assertEqual("0123450678901230456078901230456709012", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("0123450678901230456078901230456709012", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods19 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "", "abc" );

	assertEqual("", st.fString);
	assertEqual("abc", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethods20 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "abc", "" );

	assertEqual("abc", st.fString);
	assertEqual("", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

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

void StringTokenizer2Test::constrMethods21 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "abc", "abcd" );

	assertEqual("abc", st.fString);
	assertEqual("abcd", st.fDelimiters);
	assertCompare( st.fPos, equal_to, 0L );

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(true, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);

	ret = st.NextToken(token);
	assertEqual("", token);
	assertEqual(false, ret);
}

void StringTokenizer2Test::constrMethodsAll ()
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
	constrMethods13 ();
	constrMethods14 ();
	constrMethods15 ();
	constrMethods16 ();
	constrMethods17 ();
	constrMethods18 ();
	constrMethods19 ();
	constrMethods20 ();
	constrMethods21 ();
}

void StringTokenizer2Test::getRemainder()
{
	StringTokenizer2 st0(".foo:bar.frm:", ".:");
	String remainder, token;

	// initial round, no parsing started yet
	remainder = st0.GetRemainder();
	assertEqual(".foo:bar.frm:", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(".foo:bar.frm:", remainder);

	// first round
	t_assert( st0.NextToken(token) );
	assertEqual("", token);
	remainder = st0.GetRemainder();
	assertEqual("foo:bar.frm:", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(".foo:bar.frm:", remainder);

	// second round
	t_assert( st0.NextToken(token) );
	assertEqual("foo", token);
	remainder = st0.GetRemainder();
	assertEqual("bar.frm:", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(":bar.frm:", remainder);

	// third round
	t_assert( st0.NextToken(token) );
	assertEqual("bar", token);
	remainder = st0.GetRemainder();
	assertEqual("frm:", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(".frm:", remainder);

	// fourth round
	t_assert( st0.NextToken(token) );
	assertEqual("frm", token);
	remainder = st0.GetRemainder();
	assertEqual("", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(":", remainder);

	// fourth round
	t_assert( !st0.NextToken(token) );
	remainder = st0.GetRemainder();
	assertEqual("", remainder);
	remainder = st0.GetRemainder(true);
	assertEqual(":", remainder);
}
