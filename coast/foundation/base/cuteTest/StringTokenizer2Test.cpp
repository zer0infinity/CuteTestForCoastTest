/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringTokenizer2Test.h"

StringTokenizer2Test::StringTokenizer2Test () {
	fShort = "short";
	fLong = "a somewhat longer string";
}

void StringTokenizer2Test::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(StringTokenizer2Test, constrMethodsAll));
	s.push_back(CUTE_SMEMFUN(StringTokenizer2Test, getRemainder));
}

void StringTokenizer2Test::constrMethods0 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "0" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("0", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods1 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "1" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("1", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods2 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "2" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("2", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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
	ASSERT_EQUAL("3045670901", token);		// ????  Empty token if string begins with a delimiter BUT no empty token if the string ends with a delimiter
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("3045670901", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods3 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "3" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("3", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods4 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "4" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("4", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods5 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "5" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("5", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods6 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "6" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("6", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods7 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "7" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("7", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods8 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "8" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("8", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods9 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "9" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("9", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods10 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "01" );
	//ASSERT( st.fString ==  "leer 0 leer 1 2345 0 6789 0 leer 1 23 0 456 0 789 0 leer 1 23 0 4567 0 9 0 leer 1 2" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("01", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2345", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("6789", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("23", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("456", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("789", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("23", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("4567", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("9", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("2", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods11 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("01234506789012304560789010230456709012", "12" );
	ASSERT_EQUAL("01234506789012304560789010230456709012", st.fString);

	ASSERT_EQUAL("12", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("0", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("345067890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("3045607890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("304567090", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods12 ()
{
	String token;
	bool ret;

	StringTokenizer2 st(     "0123450678901230456078901230456709012", "23" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("23", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("01", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("450678901", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0456078901", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("045670901", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("045670901", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("045670901", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods13 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "012" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("012", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 0
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 1
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("345", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("6789", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 1
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("3", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("456", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("789", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 1
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("3", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("4567", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("9", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 1
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods14 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "123" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("123", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("0", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 3
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("45067890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 3
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("045607890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 3
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("04567090", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods15 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "111" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("111", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods16 ()
{
	String			token;
	bool			ret;
	unsigned int	i;

	StringTokenizer2 st("0123450678901230456078901230456709012", "0123456789" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("0123456789", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	for ( i = 0; i < strlen("0123450678901230456078901230456709012"); i++ ) {
		ret = st.NextToken(token);
		ASSERT_EQUAL("", token);
		ASSERT_EQUAL(true, ret);
	}

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods17 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "12345" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("12345", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("0", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 3
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 4
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 5
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("067890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 3
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 5
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("607890", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 3
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 5
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("67090", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);	// 2
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods18 ()
{
	String token;
	bool ret;

	StringTokenizer2 st("0123450678901230456078901230456709012", "abc" );
	ASSERT_EQUAL("0123450678901230456078901230456709012", st.fString);
	ASSERT_EQUAL("abc", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	// ASSERT_EQUAL("", token);
	// ASSERT_EQUAL(false, ret);  // ?????  Token found even if the string does not contain the delimiter-char
	ASSERT_EQUAL("0123450678901230456078901230456709012", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("0123450678901230456078901230456709012", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods19 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "", "abc" );

	ASSERT_EQUAL("", st.fString);
	ASSERT_EQUAL("abc", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
}

void StringTokenizer2Test::constrMethods20 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "abc", "" );

	ASSERT_EQUAL("abc", st.fString);
	ASSERT_EQUAL("", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

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

void StringTokenizer2Test::constrMethods21 ()
{
	String token;
	bool ret;

	StringTokenizer2 st( "abc", "abcd" );

	ASSERT_EQUAL("abc", st.fString);
	ASSERT_EQUAL("abcd", st.fDelimiters);
	ASSERT_EQUAL( st.fPos, 0L );

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(true, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);

	ret = st.NextToken(token);
	ASSERT_EQUAL("", token);
	ASSERT_EQUAL(false, ret);
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
	String remainder, token;//lint !e578

	// initial round, no parsing started yet
	remainder = st0.GetRemainder();
	ASSERT_EQUAL(".foo:bar.frm:", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(".foo:bar.frm:", remainder);

	// first round
	ASSERT( st0.NextToken(token) );
	ASSERT_EQUAL("", token);
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("foo:bar.frm:", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(".foo:bar.frm:", remainder);

	// second round
	ASSERT( st0.NextToken(token) );
	ASSERT_EQUAL("foo", token);
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("bar.frm:", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(":bar.frm:", remainder);

	// third round
	ASSERT( st0.NextToken(token) );
	ASSERT_EQUAL("bar", token);
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("frm:", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(".frm:", remainder);

	// fourth round
	ASSERT( st0.NextToken(token) );
	ASSERT_EQUAL("frm", token);
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(":", remainder);

	// fourth round
	ASSERT( !st0.NextToken(token) );
	remainder = st0.GetRemainder();
	ASSERT_EQUAL("", remainder);
	remainder = st0.GetRemainder(true);
	ASSERT_EQUAL(":", remainder);
}
