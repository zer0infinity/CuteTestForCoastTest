/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- c-library modules used ---------------------------------------------------
#include <stdlib.h>

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Threads.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test ----------------------------------------------------------
#include "Base64.h"

//--- interface include --------------------------------------------------------
#include "Base64Test.h"

//---- Base64Test ----------------------------------------------------------------
Test *Base64Test::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, Base64Test, EncodeDecodeTest);
	ADD_CASE(testSuite, Base64Test, EncodeDecodeOriginalTest);

	return testSuite;

}
Base64Test::Base64Test(TString tname) : TestCaseType(tname)
{
}

Base64Test::~Base64Test()
{
}

int Base64Test::CalcEncodedLength( int Decodedlength )
{
	return( ( Decodedlength / 3 ) * 4 + ( ( Decodedlength % 3 ) > 0 ? 4 : 0 ) );
}

void Base64Test::EncodeDecodeTest()
{
	Base64 base64("base64");
	String Empty;
	String OriginalString;
	String EncodedString;
	String DecodedString;
	int i;

	// Test 1:
	// checkif all HEX-values can be en-/decoded
	// check for identity of original and decoded string
	// check the lengths

	for ( i = 0; i < 256; i++ ) {
		OriginalString.Append( (char)i );
	}

	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );

	// Encode, Decode, check whether the Strings are identical
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );

	// Tests 2 .. 8:
	// Add char by char,
	// check for identity of original and decoded string
	// check the encoded string,
	// check the lengths

	// Test 2
	OriginalString = Empty;
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)0 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AA==", EncodedString );

	// Test 3
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)1 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAE=", EncodedString );
	// Test 4
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)2 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAEC", EncodedString );
	// Test 5
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)3 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAw==", EncodedString );

	// Test 6
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)4 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAwQ=", EncodedString );
	// Test 7
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)5 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAwQF", EncodedString );

	// Test 8
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)6 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAwQFBg==", EncodedString );

	// Test 9
	// reset strings
	OriginalString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	EncodedString = Empty;
	DecodedString = Empty;

	// encode decode with base64
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	// check whether the Strings are identical
	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	assertCharPtrEqual( "QUJDREVGR0hJSktMTU5PUFFSU1RVVldYWVo=", EncodedString );

	//Test 10
	// Only Unprintable Chars, unscrambled
	OriginalString = Empty;
	EncodedString = Empty;
	DecodedString = Empty;

	for ( i = 0; i < 10; i++ ) {
		OriginalString.Append( (char)i );
	}

	// encode decode with standard encoder (base64)
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	// check whether the Strings are identical
	t_assert( (memcmp( (const char *)OriginalString, (const char *)DecodedString, OriginalString.Length() ) == 0) );
	t_assert( OriginalString.Length() == DecodedString.Length() );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	// String::IntPrintOn:  01     -> \x01
	// Coast::URLUtils::urlEncode: \x01   -> %5Cx01
	// Coast::URLUtils::urlEncode: %5Cx01 -> %255Cx01
	assertCharPtrEqual( "AAECAwQFBgcICQ==", EncodedString );

	// One Printable Char, unscrambled
	OriginalString = "M";
	EncodedString = Empty;
	DecodedString = Empty;

	// check whether the Strings are identical
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	assertCharPtrEqual( "TQ==", EncodedString );

	// One Unprintable Char, unscrambled
	OriginalString = Empty;
	OriginalString.Append( (char)5 );

	OriginalString = OriginalString;
	EncodedString = Empty;
	DecodedString = Empty;
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	// String::IntPrintOn:  05     -> \x05
	// Coast::URLUtils::urlEncode: \x05   -> %5Cx05
	// Coast::URLUtils::urlEncode: %5Cx05 -> %255Cx05
	assertCharPtrEqual( "BQ==", EncodedString );

	// Only Printable Chars, scrambled
	OriginalString = "ABC";
	EncodedString = Empty;
	DecodedString = Empty;
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	// w\"ABC\"
	assertCharPtrEqual( "QUJD", EncodedString );

	// a particularly nasty sequence of non-printables and printables
	OriginalString = "Anfang%252B+%2BEnde";
	EncodedString = Empty;
	DecodedString = Empty;

	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	assertEqual( OriginalString, DecodedString );
	assertCharPtrEqual( "QW5mYW5nJTI1MkIrJTJCRW5kZQ==", EncodedString );

	// Only Unprintable Chars, scrambled
	OriginalString = Empty;
	for (  i = 0; i < 3; i++ ) {
		OriginalString.Append( (char)i );
	}
	EncodedString = Empty;
	DecodedString = Empty;
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	// String::IntPrintOn:  01     -> \x01
	// Coast::URLUtils::urlEncode: \x01   -> %5Cx01
	// Coast::URLUtils::urlEncode: %5Cx01 -> %255Cx01

	assertCharPtrEqual( "AAEC", EncodedString );

	// One Printable Char, scrambled
	OriginalString = "M";
	OriginalString = OriginalString;
	EncodedString = Empty;
	DecodedString = Empty;
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );
	assertCharPtrEqual( "TQ==", EncodedString );

	//           1         2         3         4         5         6
	// 0123456789012345678901234567890123456789012345678901234567890123456789
	// ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-$

	// One Unprintable Char, scrambled
	OriginalString = Empty;
	OriginalString.Append( (char)5 );

	OriginalString = OriginalString;
	EncodedString = Empty;
	DecodedString = Empty;
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	assertCharPtrEqual( OriginalString, DecodedString );
	t_assert( OriginalString == DecodedString );

	assertCharPtrEqual( "BQ==", EncodedString );
}

void Base64Test::EncodeDecodeOriginalTest()
{
	Base64Regular base64("base64");
	String Empty;
	String OriginalString;
	String EncodedString;
	String DecodedString;

	// Test 1:
	// checkif all HEX-values can be en-/decoded
	// check for identity of original and decoded string
	// check the lengths

	for ( int i = 0; i < 256; i++ ) {
		OriginalString.Append( (char)i );
	}

	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );

	// Encode, Decode, check whether the Strings are identical
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );

	// Tests 2 .. 8:
	// Add char by char,
	// check for identity of original and decoded string
	// check the encoded string,
	// check the lengths

	// Test 2
	OriginalString = Empty;
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)0 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AA==", EncodedString );

	// Test 3
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)1 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAE=", EncodedString );

	// Test 4
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)2 );
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAEC", EncodedString );
	// Test 5
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)3 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAw==", EncodedString );

	// Test 6
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)4 );
	base64.DoEncode( EncodedString, OriginalString);
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAwQ=", EncodedString );
	// Test 7
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)5 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAwQF", EncodedString );

	// Test 8
	EncodedString  = Empty;
	DecodedString  = Empty;
	OriginalString.Append( (char)6 );
	base64.DoEncode( EncodedString, OriginalString );
	base64.DoDecode( DecodedString, EncodedString );
	t_assert( 	memcmp( (const char *)OriginalString,
						(const char *)DecodedString,
						OriginalString.Length() ) == 0 );
	t_assert( OriginalString.Length() == DecodedString.Length() );
	t_assert( EncodedString.Length() == CalcEncodedLength( OriginalString.Length() ) );
	assertCharPtrEqual( "AAECAwQFBg==", EncodedString );
}
