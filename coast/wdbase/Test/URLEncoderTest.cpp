/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "URLEncoderTest.h"
#include "URLEncoder.h"
#include "TestSuite.h"
#include "Dbg.h"
#include <stdlib.h>

//---- URLEncoderTest ----------------------------------------------------------------
Test *URLEncoderTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, URLEncoderTest, EncodeDecodeTest);
	ADD_CASE(testSuite, URLEncoderTest, EncodeTest);
	ADD_CASE(testSuite, URLEncoderTest, DecodeTest);
	return testSuite;
}

URLEncoderTest::URLEncoderTest(TString tname)
	: TestCaseType(tname)
{
}

URLEncoderTest::~URLEncoderTest()
{
}

void URLEncoderTest::EncodeTest()
{
	URLEncoder encoder("encoder");
	String Request = "Dies ist\n \"%&?/\\#{}einTestString";
	String Answer;
	encoder.DoEncode(Answer, Request);
	assertEqual( "Dies%20ist%0A%20%22%25&%3F%2F%5C%23%7B%7DeinTestString", Answer );
}

void URLEncoderTest::DecodeTest()
{
	URLEncoder encoder("encoder");
	String Request = "Dies%20ist%0A%20%22%25%26%3F%2F%5C%23%7B%7DeinTestString";
	String Answer, expected;
	encoder.DoDecode(Answer, Request);
	expected = "Dies ist\n ";
	expected.Append('"').Append("%&?/\\#{}einTestString");
	assertEqual( expected, Answer );
	Request = "Dies+ist%0A+%22%25%26%3F%2F%5C%23%7B%7Dein+Test+String";
	encoder.DoDecode(Answer, Request);
	expected = "Dies ist\n ";
	expected.Append('"').Append("%&?/\\#{}ein Test String");
	assertEqual( expected, Answer );
}

void URLEncoderTest::EncodeDecodeTest()
{
	StartTrace(URLEncoderTest.EncodeDecodeTest);
	URLEncoder encoder("encoder");

	int i;

	String EmptySt, expected;
	String EncodedString;
	String DecodedString;
	String OriginalString;

	// Test 1:
	// checkif all HEX-values can be en-/decoded
	// check for identity of original and decoded string
	// check the lengths

	// setup original
	for (  i = 0; i < 256; i++ ) {
		OriginalString.Append( (char)i );
	}

	// encode decode with URLEncoder
	// and changes '+' to ' ' in decoded string
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString);
	expected = OriginalString;
	// '+' gets converted to a space also
	expected.PutAt(long('+'), ' ');
	// assert strings are equal
	assertEqualRaw(expected, DecodedString);
	assertEqual( expected.Length(), DecodedString.Length() );

	assertEqual( expected, DecodedString );
	assertEqual( expected.Length(), DecodedString.Length() );

	// Test 3
	// reset strings
	OriginalString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	// encode decode with URLEncoder
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	// check whether the Strings are identical
	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	assertCharPtrEqual( "ABCDEFGHIJKLMNOPQRSTUVWXYZ", EncodedString );

	//Test 4
	// Only Unprintable Chars
	OriginalString = EmptySt;
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	for ( i = 0; i < 10; i++ ) {
		OriginalString.Append( (char)i );
	}

	// encode decode with URLEncoder
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	// check whether the Strings are identical
	t_assert( (memcmp( (const char *)OriginalString, (const char *)DecodedString, OriginalString.Length() ) == 0) );
	t_assert( OriginalString.Length() == DecodedString.Length() );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	assertCharPtrEqual( "%00%01%02%03%04%05%06%07%08%09", EncodedString );

	// One Printable Char, unscrambled
	OriginalString = "M";
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	// check whether the Strings are identical
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	assertCharPtrEqual( "M", EncodedString );

	// One Unprintable Char, unscrambled
	OriginalString = EmptySt;
	OriginalString.Append( (char)5 );

	OriginalString = OriginalString;
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	// String::IntPrintOn:  05     -> \x05
	// Coast::URLUtils::urlEncode: \x05   -> %5Cx05
	// Coast::URLUtils::urlEncode: %5Cx05 -> %255Cx05
	assertCharPtrEqual( "%05", EncodedString );

	// Only Printable Chars, scrambled
	OriginalString = "ABC";
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	// w\"ABC\"
	assertCharPtrEqual( "ABC", EncodedString );

	// a particularly nasty sequence of non-printables and printables
	OriginalString = "Anfang%252B+%2BEnde";
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	encoder.DoEncode( EncodedString, OriginalString);
	assertEqual( "Anfang%25252B+%252BEnde", EncodedString);
	encoder.DoDecode( DecodedString, EncodedString );
	assertEqual( "Anfang%252B %2BEnde", DecodedString);

	// Only Unprintable Chars, scrambled
	OriginalString = EmptySt;
	for (  i = 0; i < 3; i++ ) {
		OriginalString.Append( (char)i );
	}
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	assertCharPtrEqual( "%00%01%02", EncodedString );

	// One Printable Char, scrambled
	OriginalString = "M";
	OriginalString = OriginalString;
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	assertCharPtrEqual( "M", EncodedString );

	//           1         2         3         4         5         6
	// 0123456789012345678901234567890123456789012345678901234567890123456789
	// ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-$

	// One Unprintable Char, scrambled
	OriginalString = EmptySt;
	OriginalString.Append( (char)5 );

	OriginalString = OriginalString;
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode( EncodedString, OriginalString);
	encoder.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	assertCharPtrEqual( "%05", EncodedString );
}
