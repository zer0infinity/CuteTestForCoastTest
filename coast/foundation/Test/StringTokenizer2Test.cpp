/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringTokenizer2Test.h"
//--- c-library modules used ---------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "System.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

StringTokenizer2Test::StringTokenizer2Test (TString tname) : TestCase(tname)
{
}

void StringTokenizer2Test::setUp ()
{
	fShort = "short";
	fLong = "a somewhat longer string";
}

Test *StringTokenizer2Test::worksuite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(StringTokenizer2Test, constrMethodsAll));
	testSuite->addTest (NEW_CASE(StringTokenizer2Test, getRemainder));

	return testSuite;
}

Test *StringTokenizer2Test::suite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(StringTokenizer2Test, constrMethodsAll));

	return testSuite;
}

void StringTokenizer2Test::constrMethods0 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "0" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "0" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods1 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "1" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "1" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods2 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "2" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "2" );
	t_assert( st.fPos == 0L );

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
	t_assert( token == "3045670901" );		// ????  Empty token if string begins with a delimiter BUT no empty token if the string ends with a delimiter
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "3045670901" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods3 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "3" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "3" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods4 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "4" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "4" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods5 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "5" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "5" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods6 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "6" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "6" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods7 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "7" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "7" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods8 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "8" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "8" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods9 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "9" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "9" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods10 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "01" );
	//t_assert( st.fString ==  "leer 0 leer 1 2345 0 6789 0 leer 1 23 0 456 0 789 0 leer 1 23 0 4567 0 9 0 leer 1 2" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "01" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "2345" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "6789" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "23" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "456" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "789" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "23" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "4567" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "9" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "2" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "2" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods11 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("01234506789012304560789010230456709012", "12" );
	t_assert( st.fString ==  "01234506789012304560789010230456709012" );

	t_assert( st.fDelimiters == "12" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "0" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "345067890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "3045607890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "304567090" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods12 ()
{
	String token;
	bool ret;

	StringTokenizer2 st(     "0123450678901230456078901230456709012", "23" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "23" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "01" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "450678901" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0456078901" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "045670901" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "045670901" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "045670901" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods13 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "012" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "012" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 0
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 1
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "345" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "6789" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 1
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "3" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "456" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "789" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 1
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "3" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "4567" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "9" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 1
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods14 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "123" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "123" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "0" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 3
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "45067890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 3
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "045607890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 3
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "04567090" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods15 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "111" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "111" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods16 ()
{
	String			token;
	bool			ret;
	unsigned int	i;

	StringTokenizer2 st("0123450678901230456078901230456709012", "0123456789" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "0123456789" );
	t_assert( st.fPos == 0L );

	for ( i = 0; i < strlen("0123450678901230456078901230456709012"); i++ ) {
		ret = st.NextToken(token);
		t_assert( token == "" );
		t_assert( ret == true );
	}

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods17 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "12345" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "12345" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "0" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 3
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 4
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 5
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "067890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 3
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 5
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "607890" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 3
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 5
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "67090" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );	// 2
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods18 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "abc" );
	t_assert( st.fString ==  "0123450678901230456078901230456709012" );
	t_assert( st.fDelimiters == "abc" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	// t_assert( token == "" );
	// t_assert( ret == false );  // ?????  Token found even if the string does not contain the delimiter-char
	t_assert( token == "0123450678901230456078901230456709012" );
	t_assert( ret == true );

	ret = st.NextToken(token);
	t_assert( token == "0123450678901230456078901230456709012" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods19 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "", "abc" );

	t_assert( st.fString     ==  "" );
	t_assert( st.fDelimiters ==  "abc" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
}

void StringTokenizer2Test::constrMethods20 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "abc", "" );

	t_assert( st.fString     ==  "abc" );
	t_assert( st.fDelimiters ==  "" );
	t_assert( st.fPos == 0L );

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

void StringTokenizer2Test::constrMethods21 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "abc", "abcd" );

	t_assert( st.fString     ==  "abc" );
	t_assert( st.fDelimiters ==  "abcd" );
	t_assert( st.fPos == 0L );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( token == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( token == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( token == true );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );

	ret = st.NextToken(token);
	t_assert( token == "" );
	t_assert( ret == false );
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

	// first round
	remainder = st0.GetRemainder();
	t_assert( remainder == "foo:bar.frm:" );
	remainder = st0.GetRemainder(true);
	t_assert( remainder == "foo:bar.frm:" );

	// second round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	t_assert( remainder == "bar.frm:" );
	remainder = st0.GetRemainder(true);
	t_assert( remainder == ":bar.frm:" );

	// third round
	st0.NextToken(token);
	remainder = st0.GetRemainder();
	t_assert( remainder == "frm:" );
	remainder = st0.GetRemainder(true);
	t_assert( remainder == ".frm:" );

	// fourth round
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

