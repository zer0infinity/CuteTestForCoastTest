/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringTest.h"

//--- c-library modules used ---------------------------------------------------
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <float.h>

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "StringStream.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

StringTest::StringTest (TString tname) : TestCase(tname)
{
}

StringTest::~StringTest()
{
}

void StringTest::setUp ()
{
	fShort = "short";
	fLong = "a somewhat longer string";
}

Test *StringTest::worksuite ()
{
	TestSuite *testSuite = new TestSuite;
	return testSuite;
}

Test *StringTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(StringTest, constructors));
	testSuite->addTest (NEW_CASE(StringTest, appendsChar));
	testSuite->addTest (NEW_CASE(StringTest, appendsCharChain));
	testSuite->addTest (NEW_CASE(StringTest, appendsNullPtr));
	testSuite->addTest (NEW_CASE(StringTest, appendsString));
	testSuite->addTest (NEW_CASE(StringTest, appendsLong));
	testSuite->addTest (NEW_CASE(StringTest, appendsDouble));
	testSuite->addTest (NEW_CASE(StringTest, appendsAsHex));
	testSuite->addTest (NEW_CASE(StringTest, appendsTwoHexAsChar));
	testSuite->addTest (NEW_CASE(StringTest, appendsFile));       // ??????
	testSuite->addTest (NEW_CASE(StringTest, appendsWithDelimiter));
	testSuite->addTest (NEW_CASE(StringTest, methods));
	testSuite->addTest (NEW_CASE(StringTest, TestCapacity));
	testSuite->addTest (NEW_CASE(StringTest, GetLine));
	testSuite->addTest (NEW_CASE(StringTest, OptimizedConstructorOrAssignment));
	testSuite->addTest (NEW_CASE(StringTest, trimFrontEmpty));
	testSuite->addTest (NEW_CASE(StringTest, EmptyAllocatorTest));
	testSuite->addTest (NEW_CASE(StringTest, DumpAsHexTest));
	testSuite->addTest (NEW_CASE(StringTest, ReplaceTest));
	testSuite->addTest (NEW_CASE(StringTest, TestFirstCharOf));
	testSuite->addTest (NEW_CASE(StringTest, TestLastCharOf));
	testSuite->addTest (NEW_CASE(StringTest, TestContainsCharAbove));

	return testSuite;
}

void StringTest::constructors ()
{
	// Init a string without parameters
	String empty;
	t_assert (empty == "");
	t_assert (empty.Length() == 0);
	t_assert (empty.Capacity() >= 0);

	// Init a string only with the Capacity
	String emptyCap(50);
	t_assert (emptyCap == "");
	t_assert ( strcmp((const char *)emptyCap, "") == 0 );
	t_assert ( memcmp((const char *)emptyCap, "",
					  strlen((const char *)emptyCap) ) == 0 );
	t_assert (emptyCap.Length() == 0);
	t_assert (emptyCap.Capacity() >= 50);

	String emptyCap0(-1);
	t_assert (emptyCap0 == "");
	t_assert (emptyCap0.Length() == 0);
	t_assert(0 <= emptyCap0.Capacity());

	// Init a string only with a 'charChain' and the length
	String stringStrLen1( "string test 1 ", strlen("string test 1 ") );
	t_assert (stringStrLen1 == "string test 1 ");
	t_assert ( memcmp( (const char *)stringStrLen1, "string test 1 ", stringStrLen1.Length() ) == 0 );
	t_assert (stringStrLen1.Length() == (long)strlen("string test 1 "));
	t_assert (stringStrLen1.Capacity() >= stringStrLen1.Length());

	String stringStrLen2( "string test 2 ", strlen("string test 2 ") + 1 );
	t_assert (stringStrLen2 == "string test 2 ");
	t_assert ( memcmp( (const char *)stringStrLen2, "string test 2 ", stringStrLen2.Length() ) == 0 );
	t_assert ( memcmp( (const char *)stringStrLen2, "string test 2 ", strlen("string test 2 ") ) == 0 );
	t_assert (stringStrLen2.Length() == (long)strlen("string test 2 "));
	// const char * constructor, only read until the first \000
	t_assert (stringStrLen2.Capacity() >= stringStrLen2.Length());

	String stringStrLen3( "string test 3 ", strlen("string test 3 ") - 1 );
	t_assert (stringStrLen3 != "string test 3 ");
	t_assert (stringStrLen3 == "string test 3");
	t_assert (stringStrLen3.Length() <  (long)strlen("string test 3 "));
	t_assert (stringStrLen3.Length() == (long)strlen("string test 3"));
	t_assert (stringStrLen3.Capacity() >= stringStrLen3.Length());
	assertEqual(stringStrLen3.Length() + 1, stringStrLen3.Capacity());

	String stringStrLen4( "string test 4 ", -1 );
	t_assert (stringStrLen4 == "string test 4 ");
	t_assert (stringStrLen4.Length() == (long)strlen("string test 4 "));
	t_assert (stringStrLen4.Capacity() >= stringStrLen4.Length());
	assertEqual(stringStrLen4.Length() + 1, stringStrLen4.Capacity());

	// Init a string only with a 'charChain'
	String stringCharChain( "CharChain" );
	t_assert (stringCharChain == "CharChain");
	t_assert ( memcmp( (const char *)stringCharChain, "CharChain", stringCharChain.Length() ) == 0 );
	t_assert (stringCharChain.Length() == (long)strlen("CharChain"));
	t_assert (stringCharChain.Capacity() >= stringCharChain.Length());

	// Init a string with another string
	String s("string test 4");
	String stringStr0(s);
	t_assert (stringStr0 == s);
	t_assert (stringStr0.Length() == s.Length() );
	//t_assert (stringStr0.Capacity() == s.Capacity() );
	t_assert (stringStr0.Capacity() >= stringStr0.Length() );
	t_assert (s.Capacity() >= s.Length() );

	assertEqual(stringStr0.Length() + 1, stringStr0.Capacity());

	s = "string test 4+";		// this is an operator!!
	String stringStr01(s);
	t_assert (stringStr01 == s);
	t_assert (stringStr01.Length() == s.Length() );
	// t_assert (stringStr01.Capacity() == s.Capacity() );			// Op = does not copy the capacity
	t_assert (stringStr01.Capacity() >= stringStr01.Length() );
	t_assert (s.Capacity() >= s.Length() );

	String stringStr1(empty);
	t_assert (stringStr1 == empty);
	t_assert (stringStr1 == "");
	t_assert (stringStr1.Length() == empty.Length() );
	//PS? t_assert (stringStr1.Capacity() == empty.Capacity() );
	t_assert (stringStr1.Capacity() >= stringStr1.Length() );
	t_assert (empty.Capacity() >= empty.Length() );

	String stringStr2(emptyCap);
	t_assert (stringStr2 == emptyCap);
	t_assert (stringStr2 == "");
	t_assert (stringStr2.Length() == emptyCap.Length() );
	// t_assert (stringStr2.Capacity() == emptyCap.Capacity() );		// ???? capacity not copied
	t_assert (stringStr2.Length() == 0 );
	t_assert (stringStr2.Capacity() >= stringStr2.Length());
	//PS? t_assert (stringStr2.Capacity() == 0);  // Why is capacity 0 ????

	String one(1);
	t_assert (empty == one);

	String oneString("1");
	t_assert (oneString == "1");
	t_assert (oneString.Length() == 1);
	t_assert (fShort == "short");
	t_assert (fShort != "Short");  // == fails
}

void StringTest::appendsChar ()
{
	//Mechanismen zur Festlegung der Capacity sind mir unklar ????

	// Append von Buchstaben
	//----------------------
	String str0;
	str0.Append('a');
	//PS? t_assert ( str0.Capacity() == 4 );
	t_assert ( str0.Length() == 1 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 1 );
	t_assert ( memcmp((const char *)str0, "a", strlen( (const char *)str0)) == 0 );

	str0.Append('b');
	//PS? t_assert ( str0.Capacity() == 4 );
	t_assert ( str0.Length() == 2 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 2 );
	t_assert ( memcmp((const char *)str0, "ab", strlen( (const char *)str0)) == 0 );

	str0.Append('c');
	//PS? t_assert ( str0.Capacity() == 4 );
	t_assert ( str0.Length() == 3 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 3 );
	t_assert ( memcmp((const char *)str0, "abc", strlen( (const char *)str0)) == 0 );

	str0.Append('d');
	//PS? t_assert ( str0.Capacity() == 10 );
	t_assert ( str0.Length() == 4 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 4 );
	t_assert ( memcmp((const char *)str0, "abcd", strlen( (const char *)str0)) == 0 );

	str0.Append('e');
	//PS? t_assert ( str0.Capacity() == 10 );
	t_assert ( str0.Length() == 5 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 5 );
	t_assert ( memcmp((const char *)str0, "abcde", strlen( (const char *)str0)) == 0 );

	str0.Append('f');
	//PS? t_assert ( str0.Capacity() == 10 );
	t_assert ( str0.Length() == 6 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 6 );
	t_assert ( memcmp((const char *)str0, "abcdef", strlen( (const char *)str0)) == 0 );

	str0.Append('g');
	//PS? t_assert ( str0.Capacity() == 10 );
	t_assert ( str0.Length() == 7 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 7 );
	t_assert ( memcmp((const char *)str0, "abcdefg", strlen( (const char *)str0)) == 0 );

	str0.Append('h');
	//PS? t_assert ( str0.Capacity() == 10 );
	t_assert ( str0.Length() == 8 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 8 );
	t_assert ( memcmp((const char *)str0, "abcdefgh", strlen( (const char *)str0)) == 0 );

	str0.Append('i');
	//PS? t_assert ( str0.Capacity() == 10 );
	t_assert ( str0.Length() == 9 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 9 );
	t_assert ( memcmp((const char *)str0, "abcdefghi", strlen( (const char *)str0)) == 0 );

	str0.Append('l');
	//PS? t_assert ( str0.Capacity() == 22 );
	t_assert ( str0.Length() == 10 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 10 );
	t_assert ( memcmp((const char *)str0, "abcdefghil", strlen( (const char *)str0)) == 0 );

	str0.Append('m');
	//PS? t_assert ( str0.Capacity() == 22 );
	t_assert ( str0.Length() == 11 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 11 );
	t_assert ( memcmp((const char *)str0, "abcdefghilm", strlen( (const char *)str0)) == 0 );

	str0.Append('n');
	//PS? t_assert ( str0.Capacity() == 22 );
	t_assert ( str0.Length() == 12 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 12 );
	t_assert ( memcmp((const char *)str0, "abcdefghilmn", strlen( (const char *)str0)) == 0 );

	// Wenn im Content der String ein '\0' gibt, gibt String.Length() einen inkonsistenten Wert zurueck.
	// 'String.Length' und 'strlen(Content)' stimmen nicht ueberein.  ????
	String str1;
	str1.Append( (char) 0);
	//PS? t_assert ( str1.Capacity() == 4 );
	t_assert ( str1.Length() == 1 );
	t_assert ( str1.Capacity() >= str1.Length() );
	t_assert ( (long)strlen( (const char *)str1 ) != str1.Length() );

	str1.Append('A');
	//PS? t_assert ( str1.Capacity() == 4 );
	t_assert ( str1.Length() == 2 );
	t_assert ( str1.Capacity() >= str1.Length() );
	t_assert ( (long)strlen( (const char *)str1 ) != str1.Length() );

	// Append mit Zahlen
	String str2;
	str2.Append((char)97); // 97 = 'a'
	//PS? t_assert ( str2.Capacity() == 4 );
	t_assert ( str2.Length() == 1 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( strlen( (const char *)str2 ) == 1 );
	t_assert ( memcmp((const char *)str2, "a", strlen( (const char *)str2)) == 0 );

	str2.Append((char)98); // 98 = 'b'
	//PS? t_assert ( str2.Capacity() == 4 );
	t_assert ( str2.Length() == 2 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( strlen( (const char *)str2 ) == 2 );
	t_assert ( memcmp((const char *)str2, "ab", strlen( (const char *)str2)) == 0 );

	str2.Append((char)99); // 99 = 'c'
	//PS? t_assert ( str2.Capacity() == 4 );
	t_assert ( str2.Length() == 3 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( strlen( (const char *)str2 ) == 3 );
	t_assert ( memcmp((const char *)str2, "abc", strlen( (const char *)str2)) == 0 );

	// Append mit unerlaubten Zahlen
	String str3;
	str3.Append( (char)(-5) );
	str3.Append( (char)(-10) );
	str3.Append( (char)(9999999L) );
	//PS? t_assert ( str3.Capacity() == 4 );
	t_assert ( str3.Length() == 3 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( strlen( (const char *)str3 ) == 3 );
}

void StringTest::appendsNullPtr ()
/* what: tries to append a 0-ptr to a String. The behavior seems somewhat inconsistent
*/
{
	String test("0123456789");
	long oldLength = test.Length();
	long oldCapacity = test.Capacity();
	test.Append((const char *)0, 10);
	assertEqual(oldLength, test.Length());
	// seems pretty crazy to copy from zero or leave it unititialized
	t_assert(test.Length() < test.Capacity());
	assertEqual(oldCapacity, test.Capacity());
	// not a necessary assertion, just to make sure we know what we are doing

	test.Append((const char *)0, 5);
	assertEqual(oldLength, test.Length());
	// seems pretty crazy to copy from zero or leave it unititialized
	t_assert(test.Length() < test.Capacity());
	assertEqual(oldCapacity, test.Capacity());
	// capacity does not increase

	String test1("abcdefghijklmnopqrstuvwxyz");		// 26 characters
	test1.Trim(10);									// trimmed to 10
	String test2("abcdefghij");						// only 10 characters

	assertEqual(test1, test2);
	test1.Append((const char *)0, 15);				// does not expand capacity
	test2.Append((const char *)0, 15);				// expands capacity and copies

	assertEqual(test1, test2);						// compares only up to first 0
	assertEqual(test1.Length(), test2.Length());
	assertEqual(10, test1.Length());				// must stay 10

	t_assert(memcmp((const char *)test1, (const char *)test2, test1.Length()) == 0);
	// not really consistent ...
}

void StringTest::appendsCharChain ()
{
	char   charChain[6] = { 'a', 'b', 'c', 'd', 'e', '\000' };
	// PT: if Append(char * is used, the buffer *must* be 0 terminated!

	// Capacity = 2*(Lenght+1)
	// charChain 100% gelesen
	String str0;
	str0.Append( charChain, 5 );
	//PS? t_assert ( str0.Capacity() == 12 );
	t_assert ( str0.Length() == 5 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( strlen( (const char *)str0 ) == 5 );
	t_assert ( memcmp((const char *)str0, charChain, str0.Length() ) == 0 );

	// String.Length() != strlen((const char *)String) !!!
	// reading all of charChain plus something extra
	// (purify might generate a UMR, ABR or something the like
	String str1;
	str1.Append( (void *) charChain, 10 );
	//PS? t_assert ( str1.Capacity() == 22 );
	t_assert ( str1.Length() == 10 );
	t_assert ( str1.Capacity() >= str1.Length() );
	t_assert ( memcmp((const char *)str1, charChain, strlen(charChain) ) == 0 );
//	t_assert ( memcmp((const char *)str1, charChain, max(str1.Length(), strlen(charChain)) ) == 0 );
	// PT: this is a highly dubious assertion, compare some random memory!
	// (purify generates a UMR)

	// 0 char von charChain gelesen
	String str2;
	str2.Append( charChain, 0 );
	//PS? t_assert ( str2.Capacity() == 0 );
	t_assert ( str2.Length() == 0 );
	t_assert ( str2.Capacity() >= str2.Length() );

	// Lesen innerhalb von charChain
	String str3;
	str3.Append( charChain, 3 );
	//PS? t_assert ( str3.Capacity() == 8 );
	t_assert ( str3.Length() == 3 );
	t_assert ( str3.Capacity() >= str3.Length() );
	t_assert ( memcmp((const char *)str3, "abcde", str3.Length() ) == 0 );

	// Lesen innerhalb von charChain
	String str4;
	str4.Append( &charChain[3], 2 );
	//PS? t_assert ( str4.Capacity() == 6 );
	t_assert ( str4.Length() == 2 );
	t_assert ( str4.Capacity() >= str4.Length() );
	t_assert ( memcmp((const char *)str4, "de", str4.Length() ) == 0 );

	//Lesen innerhalb von charChain
	String str5;
	str5.Append( &charChain[4], 1 );
	//PS? t_assert ( str5.Capacity() == 4 );
	t_assert ( str5.Length() == 1 );
	t_assert ( str5.Capacity() >= str5.Length() );
	t_assert ( memcmp((const char *)str5, "e", str5.Length() ) == 0 );

	//  deliberately reading out of charChain
	String str6;
	str6.Append( (void *)&charChain[4], 2 );
	//PS? t_assertEqual ( 6, str6.Capacity() );
	assertEqual ( 2, str6.Length() );
	t_assert ( str6.Capacity() >= str6.Length() );

	// Lesen mit komischer Laenge:  ????
	String str7;
	str7.Append( charChain, -4 );

	// 2 Mal Append
	String str8;
	str8.Append( charChain, 5 );
	str8.Append( charChain, 5 );
	//PS? t_assert ( str8.Capacity() == 12 );  // 2*(5+1)
	t_assert ( str8.Length() == 10 );
	t_assert ( str8.Capacity() >= str8.Length() );
	t_assert ( memcmp((const char *)str8, "abcdeabcde", str8.Length() ) == 0 );

	// Append mit anderen Stringen
	String str9, str10;
	str9.Append( "123", 3 );
	str10.Append( "456", 3 );
	str9.Append( (const char *)str10, strlen((const char *)str10) );
	//PS? t_assert ( str9.Capacity() == 8 );
	t_assert ( str9.Length() == 6 );
	t_assert ( str9.Capacity() >= str9.Length() );
	str9.Append( (const char *)str10, strlen((const char *)str10) );
	//PS? t_assert ( str9.Capacity() == 20 );
	t_assert ( str9.Length() == 9 );
	t_assert ( str9.Capacity() >= str9.Length() );

	// Append mit sich selbst
	String str11;
	str11.Append( "123", 3 );
	str11.Append( (const char *)str11, strlen((const char *)str11) );
	//PS? t_assert ( str11.Capacity() == 8 );
	t_assert ( str11.Length() == 6 );
	t_assert ( str11.Capacity() >= str11.Length() );
	t_assert ( memcmp( (const char *)str11, "123123", str11.Length() ) == 0 );

	// Append mit einem Teil von sich selbst
	String str12;
	str12.Append( "123", 3 );
	str12.Append( ((const char *)(str12) + 1), 2 );
	//PS? t_assert ( str12.Capacity() == 8 );
	t_assert ( str12.Length() == 5 );
	t_assert ( str12.Capacity() >= str12.Length() );
	t_assert ( memcmp( (const char *)str12, "12323", str12.Length() ) == 0 );

	// Im Content gibt es einen '\0'.  Geht's ?
	char buf[6] = { 'a', 'b', (char)0, 'c', 'd', '\000' };
	String str13;
	str13.Append( (void *)buf, 5 );
	// (void *) cast because otherwise Append does not read past 0
	//PS? assertEqual ( 12, str13.Capacity() );
	assertEqual ( 5, str13.Length() );
	t_assert ( str13.Capacity() >= str13.Length() );
	t_assert ( str13.Capacity() >= (long)strlen( (const char *)str13 ) );
	str13.Append( (void *)buf, 5 );
	// (void *) cast because otherwise Append does not read past 0
	//PS? assertEqual ( 12, str13.Capacity() );
	assertEqual ( 10, str13.Length() );
	t_assert ( str13.Capacity() >= str13.Length() );
	t_assert ( str13.Capacity() >= (long)strlen( (const char *)str13 ) );
	assertEqual ( strlen( buf ), strlen( (const char *)str13 ) );
	assertEqual ( 2, strlen( (const char *)str13 ) );
	t_assert ( str13.Length() >= (long)strlen( (const char *)str13 ) );
}

void StringTest::appendsString ()
{
	// Append von 2 leeren Stringen
	String str0, str1;
	str0.Append( str1 );
	t_assert ( str0.Capacity() >= 0 );
	t_assert ( str0.Length() == 0 );

	// Append von sich selbst
	String str2;
	str2.Append("123", 3);  // length = 3 --> capacity = 2*(3+1)
	str2.Append( str2 );
	//PS? t_assert ( str2.Capacity() == 8 );
	t_assert ( str2.Length() == 6 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( memcmp( (const char *)str2, "123123", str2.Length() ) == 0 );
	str2.Append( str2 );
	//PS? t_assert ( str2.Capacity() == 26 );
	t_assert ( str2.Length() == 12 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( memcmp( (const char *)str2, "123123123123", str2.Length() ) == 0 );
	str2.Append( str2 );
	//PS? t_assert ( str2.Capacity() == 26 );
	t_assert ( str2.Length() == 24 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( memcmp( (const char *)str2, "123123123123123123123123", str2.Length() ) == 0 );
	str2.Append( str2 );
	//PS? t_assert ( str2.Capacity() == 98 );
	t_assert ( str2.Length() == 48 );
	t_assert ( str2.Capacity() >= str2.Length() );
	t_assert ( memcmp( (const char *)str2, "123123123123123123123123123123123123123123123123", str2.Length() ) == 0 );

	// Append von 2 Stringen
	String str3, str4;
	str3.Append( "123", 3 );
	str4.Append( "456", 3 );
	str3.Append( str4 );
	//PS? t_assert ( str3.Capacity() == 8 );
	t_assert ( str3.Length() == 6 );
	t_assert ( str3.Capacity() >= str3.Length() );
	t_assert ( memcmp( (const char *)str3, "123456", str3.Length() ) == 0 );
	str4.Append( str3 );
	//PS? t_assert ( str4.Capacity() == 20 );
	t_assert ( str4.Length() == 9 );
	t_assert ( str4.Capacity() >= str4.Length() );
	t_assert ( memcmp( (const char *)str4, "456123456", str4.Length() ) == 0 );

	// Append mit einer leeren String
	String str5, str6;
	str5.Append( "123", 3);
	str5.Append( str6 );
	//PS? t_assert ( str5.Capacity() == 8 );
	t_assert ( str5.Length() == 3 );
	t_assert ( str5.Capacity() >= str5.Length() );
	t_assert ( memcmp( (const char *)str5, "123", str5.Length() ) == 0 );

	// Append mit einer leeren String
	String str7, str8;
	str8.Append( "123", 3 );
	str7.Append( str8 );
	//PS? t_assert ( str7.Capacity() == 8 );
	t_assert ( str7.Length() == 3 );
	t_assert ( str7.Capacity() >= str7.Length() );
	t_assert ( memcmp( (const char *)str7, "123", str7.Length() ) == 0 );

	// Funktionniert Append auch wenn innerhalb des Contents ein '\0' ist?
	// Alles funktionniert NUR muss man erinnern, dass 'Strlen' ungleich 'Length()' sein kann.  ????
	String str9, str10;
	char buf[5] = {'a', 'b', (char)0, 'c', 'd'};
	str9.Append( (void *) buf, 5 );
	// (void *) cast because otherwise Append stops at 0
	str10.Append( "123", 3 );
	str9.Append( str10 );
	//PS? t_assert ( str9.Capacity() ==  12 );
	t_assert ( str9.Length() ==  8 );
	t_assert ( str9.Capacity() >= str9.Length() );
	t_assert ( str9.Length() >= (long)strlen( (const char *)str9 ) );
	t_assert ( strlen( (const char *)str9 ) == strlen(buf) );
	char bufHelp[10] = {0};
	memcpy( bufHelp, buf, sizeof(buf) );
	memcpy( &bufHelp[sizeof(buf)], "123", strlen("123") );
	t_assert ( memcmp( (const char *)str9, bufHelp, str9.Length() ) == 0 );

	// Funktionniert Append auch wenn innerhalb des Contents ein '\0' ist?
	// char buf[5] = {'a', 'b', (char)0, 'c', 'd'};
	String str11, str12;
	str11.Append( (void *)buf, 5 );
	// (void *) cast because otherwise Append does not read past 0
	str12.Append( "123", 3 );
	str12.Append( str11 );
	//PS? t_assert ( str12.Capacity() ==  18 );
	t_assert ( str12.Capacity() >= str12.Length() );
	t_assert ( strlen( (const char *)str12 ) == strlen("123") + strlen(buf) );
	memset( bufHelp, 0, sizeof(bufHelp) );
	memcpy( bufHelp, "123", strlen("123") );
	memcpy( &bufHelp[strlen("123")], buf, sizeof(buf) );
	t_assert ( memcmp( (const char *)str12, bufHelp, str12.Length() ) == 0 );
}

void StringTest::appendsLong ()
{
	// Append with several long numbers
	String str0;
	str0.Append( 0L );
	//PS? t_assert ( str0.Capacity() == 4);
	t_assert ( str0.Length() == 1 );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( memcmp( (const char *)str0, "0", str0.Length() ) == 0 );

	str0.Append( 1L );
	//PS? t_assert ( str0.Capacity() == 4);
	t_assert ( str0.Length() == 2 );
	t_assert ( str0.Capacity() >= str0.Length() );
	assertEqual("01", str0);
	//PS? t_assert ( memcmp( (const char *)str0, "01", str0.Length() )==0 );

	str0.Append( 2L );
	//PS? t_assert ( str0.Capacity() == 4);
	t_assert ( str0.Length() == 3 );
	t_assert ( str0.Capacity() >= str0.Length() );
	//PS? t_assert ( memcmp( (const char *)str0, "012", str0.Length() )==0 );

	str0.Append( 3L );
	//PS? t_assert ( str0.Capacity() == 10);
	t_assert ( str0.Length() == 4 );
	t_assert ( str0.Capacity() >= str0.Length() );
	//PS? t_assert ( memcmp( (const char *)str0, "0123", str0.Length() )==0 );

	str0.Append( -1L );
	//PS? t_assert ( str0.Capacity() == 10);
	t_assert ( str0.Length() == 6 );
	t_assert ( str0.Capacity() >= str0.Length() );
	//PS? t_assert ( memcmp( (const char *)str0, "0123-1", str0.Length() )==0 );

	str0.Append( -2L );
	//PS? t_assert ( str0.Capacity() == 10);
	t_assert ( str0.Length() == 8 );
	t_assert ( str0.Capacity() >= str0.Length() );
	//PS? t_assert ( memcmp( (const char *)str0, "0123-1-2", str0.Length() )==0 );

	str0.Append( -3L );
	//PS? t_assert ( str0.Capacity() == 22);
	//PS? t_assert ( str0.Length() == 10 );
	t_assert ( str0.Capacity() >= str0.Length() );
	//PS? t_assert ( memcmp( (const char *)str0, "0123-1-2-3", str0.Length() )==0 );

	str0.Append( 2147483647L );
	//PS? t_assert ( str0.Capacity() == 22);
	t_assert ( str0.Length() == 20 );
	t_assert ( str0.Capacity() >= str0.Length() );
	//PS? t_assert ( memcmp( (const char *)str0, "0123-1-2-32147483647", str0.Length() )==0 );

	str0.Append( -2147483647L );
	//PS? t_assert ( str0.Capacity() == 64);
	//PS? t_assert ( str0.Length() == 31 );
	t_assert ( str0.Capacity() >= str0.Length() );
	//PS? t_assert ( memcmp( (const char *)str0, "0123-1-2-32147483647-2147483647", str0.Length() )==0 );
}

void StringTest::appendsDouble ()
{
	// Rundungen sind sehr ungenau ???? //
	//----------------------------------//

	// === capacity 4 === Length 3 === Content 0 0
	String str0;
	str0.Append( (double)0 );
	str0.Append(' ');
	str0.Append( (double)(-0) );
	t_assert ( str0.Capacity() >= str0.Length() );

	// === capacity 4 === Length 3 === Content 0 -0
	String str1;
	str1.Append(0.0);
	str1.Append(' ');
	str1.Append(-0.0);
	t_assert ( str1.Capacity() >= str1.Length() );

	// === capacity 42 === Length 24 === Content 2.22045e-16 -2.22045e-16
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str2;
	str2.Append(DBL_EPSILON);
	str2.Append(' ');
	str2.Append(-DBL_EPSILON);
	t_assert ( str2.Capacity() >= str2.Length() );

	// === capacity 34 === Length 26 === Content 1.79769e+308 -1.79769e+308
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str3;
	str3.Append(DBL_MAX);
	str3.Append(' ');
	str3.Append(-DBL_MAX);
	t_assert ( str3.Capacity() >= str3.Length() );

	// === capacity 34 === Length 26 === Content 2.22507e-308 -2.22507e-308
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str4;
	str4.Append(DBL_MIN);
	str4.Append(' ');
	str4.Append(-DBL_MIN);
	t_assert ( str4.Capacity() >= str4.Length() );

	// === capacity 42 === Length 24 === Content 1.19209e-07 -1.19209e-07
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str5;
	str5.Append(FLT_EPSILON);
	str5.Append(' ');
	str5.Append(-FLT_EPSILON);
	t_assert ( str5.Capacity() >= str5.Length() );

	// === capacity 42 === Length 24 === Content 3.40282e+38 -3.40282e+38
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str6;
	str6.Append(FLT_MAX);
	str6.Append(' ');
	str6.Append(-FLT_MAX);
	t_assert ( str6.Capacity() >= str6.Length() );

	// === capacity 42 === Length 24 === Content 1.17549e-38 -1.17549e-38
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str7;
	str7.Append(FLT_MIN);
	str7.Append(' ');
	str7.Append(-FLT_MIN);
	t_assert ( str7.Capacity() >= str7.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e-08 -1.23457e-08
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str8;
	str8.Append(0.123456789e-7);
	str8.Append(' ');
	str8.Append(-0.123456789e-7);
	t_assert ( str8.Capacity() >= str8.Length() );

	// === capacity 42 === Length 24 === Content 7.12346e+07 -7.12346e+07
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str9;
	str9.Append(7.123456789E+7);
	str9.Append(' ');
	str9.Append(-7.123456789E+7);
	t_assert ( str9.Capacity() >= str9.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+15 -1.23457e+15
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str10;
	str10.Append(123456789.123456789E+7);
	str10.Append(' ');
	str10.Append(-123456789.123456789E+7);
	t_assert ( str10.Capacity() >= str10.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+15 -1.23457e+15
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str11;
	str11.Append(123456789.123456789E+7);
	str11.Append(' ');
	str11.Append(-123456789.123456789E+7);
	t_assert ( str11.Capacity() >= str11.Length() );

	// Die Nachkommastellen gehen nicht verloren sondern gerundet XXX.XXX
	// === capacity 18 === Length 16 === Content 123.012 -123.012
	String str12;
	str12.Append(123.0123456789);
	str12.Append(' ');
	str12.Append(-123.0123456789);
	t_assert ( str12.Capacity() >= str12.Length() );

	// Die Nachkommastellen gehen nicht verloren sondern gerundet XXXX.XX
	// === capacity 28 === Length 16 === Content 1234.01 -1234.01
	String str13;
	str13.Append(1234.0123456789);
	str13.Append(' ');
	str13.Append(-1234.0123456789);
	t_assert ( str13.Capacity() >= str13.Length() );

	// Die Nachkommastellen gehen verloren
	// === capacity 26 === Length 12 === Content 12345 -12345
	String str14;
	str14.Append(12345.0123456789);
	str14.Append(' ');
	str14.Append(-12345.0123456789);
	t_assert ( str14.Capacity() >= str14.Length() );

	// Die Nachkommastellen gehen verloren
	// === capacity 30 === Length 14 === Content 123456 -123456
	String str15;
	str15.Append(123456.0123456789);
	str15.Append(' ');
	str15.Append(-123456.0123456789);
	t_assert ( str15.Capacity() >= str15.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+06 -1.23457e+06
	String str16;
	str16.Append(1234567.0123456789);
	str16.Append(' ');
	str16.Append(-1234567.0123456789);
	t_assert ( str16.Capacity() >= str16.Length() );

	// === capacity 42 === Length 24 === Content 1.23457e+09 -1.23457e+09
	// Das Format ist X.XXXXXe"+/-"XXX:   grosse Rundungsfehler
	String str17;
	str17.Append(1234567890.0123456789);
	str17.Append(' ');
	str17.Append(-1234567890.0123456789);
	t_assert ( str17.Capacity() >= str17.Length() );
}

void StringTest::appendsAsHex ()
{
	// Test Methode AppendAsHex(char)
	{
		int i;
		String str0;
		for ( i = 0; i < 256; i++ ) {
			str0.AppendAsHex( (unsigned char)i);
			str0.Append(' ');
		}

		const char exp[] = "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F 10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F 20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F 30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F 40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F 60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F 70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F 80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F 90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF ";
		t_assert ( str0.Capacity() >= str0.Length() );
		t_assert ( str0.Length() == (256 * 3));  // '3E ':  3 Stellen per Zahl
		t_assert ( memcmp((const char *)str0, exp, (256 * 3)) == 0);
	}

	// Test Methode AppendAsHex(const char*, long, char)
	{
		{
			String str0, str1, str2, str3;

			const unsigned char buf[3] = { 0x00, 0xA1, 0xFF };

			str0.AppendAsHex(buf, 0);
			t_assert ( str0 == String("") );
			t_assert ( str0.Length()  == 0 );

			str1.AppendAsHex(buf, 1);
			t_assert ( str1 == String("00") );
			t_assert ( str1.Length()  == 2 );

			str2.AppendAsHex(buf, 3);
			t_assert ( str2 == String("00A1FF") );
			t_assert ( str2.Length()  == 6 );

			str3.AppendAsHex(buf, -1);
			t_assert ( str3 == String("") );
			t_assert ( str3.Length()  == 0 );
		}
		{
			String str0, str1, str2, str3;

			const unsigned char buf[3] = { 0x00, 0xA1, 0xFF };

			str0.AppendAsHex(buf, 0, ' ');
			t_assert ( str0 == String("") );
			t_assert ( str0.Length()  == 0 );

			str1.AppendAsHex(buf, 1, ' ');
			t_assert ( str1 == String("00") );
			t_assert ( str1.Length()  == 2 );

			str2.AppendAsHex(buf, 3, ' ');
			t_assert ( str2 == String("00 A1 FF") );
			t_assert ( str2.Length()  == 8 );

			str3.AppendAsHex(buf, -1, ' ');
			t_assert ( str3 == String("") );
			t_assert ( str3.Length()  == 0 );
		}
	}
}

void StringTest::appendsTwoHexAsChar ()
{
	// Test Methode AppendTwoHexAsChar(const char *)
	{
		char	a[2];
		int		i, j;
		String	str0;

		for ( i = 0; i < 16; i++ ) {
			if ( i > 9 ) {
				switch (i) {
					case 10:
						a[0] = 'a';
						break;
					case 11:
						a[0] = 'b';
						break;
					case 12:
						a[0] = 'c';
						break;
					case 13:
						a[0] = 'd';
						break;
					case 14:
						a[0] = 'e';
						break;
					case 15:
						a[0] = 'f';
						break;
				}
			} else {
				a[0] = (char)i + '0';
			}

			for ( j = 0; j < 16; j++ ) {
				if ( j > 9 ) {
					switch (j) {
						case 10:
							a[1] = 'a';
							break;
						case 11:
							a[1] = 'b';
							break;
						case 12:
							a[1] = 'c';
							break;
						case 13:
							a[1] = 'd';
							break;
						case 14:
							a[1] = 'e';
							break;
						case 15:
							a[1] = 'f';
							break;
					}
				} else {
					a[1] = (char)j + '0';
				}

				str0.AppendTwoHexAsChar(a);
				str0.Append(' ');
			}
		}

		String str1;
		int ch;
		for ( ch = (char)0; ch < 256; ch++ ) {
			str1.Append( (char)ch );
			str1.Append(' ');
		}

		//PS? t_assert( str0.Capacity() == str1.Capacity() );
		t_assert( str0.Length() == str1.Length() );
		t_assert ( str0.Capacity() >= str0.Length() );
		t_assert ( str1.Capacity() >= str1.Length() );
		t_assert( memcmp( (const char *)str0, (const char *)str1, str0.Length() ) == 0 );

		String str2;
		for ( i = 0; i < 16; i++ ) {
			if ( i > 9 ) {
				switch (i) {
					case 10:
						a[0] = 'A';
						break;
					case 11:
						a[0] = 'B';
						break;
					case 12:
						a[0] = 'C';
						break;
					case 13:
						a[0] = 'D';
						break;
					case 14:
						a[0] = 'E';
						break;
					case 15:
						a[0] = 'F';
						break;
				}
			} else {
				a[0] = (char)i + '0';
			}

			for ( j = 0; j < 16; j++ ) {
				if ( j > 9 ) {
					switch (j) {
						case 10:
							a[1] = 'A';
							break;
						case 11:
							a[1] = 'B';
							break;
						case 12:
							a[1] = 'C';
							break;
						case 13:
							a[1] = 'D';
							break;
						case 14:
							a[1] = 'E';
							break;
						case 15:
							a[1] = 'F';
							break;
					}
				} else {
					a[1] = (char)j + '0';
				}

				str2.AppendTwoHexAsChar(a);
				str2.Append(' ');
			}
		}

		String str3;
		for ( ch = (char)0; ch < 256; ch++ ) {
			str3.Append( (char)ch );
			str3.Append(' ');
		}

		//PS? t_assert( str2.Capacity() == str3.Capacity() );
		t_assert( str2.Length() == str3.Length() );
		t_assert ( str2.Capacity() >= str2.Length() );
		t_assert ( str3.Capacity() >= str3.Length() );
		t_assert( memcmp( (const char *)str2, (const char *)str3, str2.Length() ) == 0 );

		//PS? t_assert( str2.Capacity() == str0.Capacity() );
		t_assert( str2.Length() == str0.Length() );
		t_assert( memcmp( (const char *)str2, (const char *)str0, str2.Length() ) == 0 );

		// unerlaubte Eingabeparameter
		String str4;
		for ( i = 0; i < 256; i++ ) {
			for ( j = 0; j < 256; j++ ) {
				if ( ((char)i < '0') && ((char)i > '9') &&
					 ((char)i < 'a') && ((char)i > 'f') &&
					 ((char)i < 'A') && ((char)i > 'F') &&
					 ((char)j < '0') && ((char)j > '9') &&
					 ((char)j < 'a') && ((char)j > 'f') &&
					 ((char)j < 'A') && ((char)j > 'F')   ) {
					a[0] = (char)i;
					a[1] = (char)j;
					str4.AppendTwoHexAsChar( a );
					str4.Append(' ');
				}
			}
		}

		String str5;
		//PS? t_assert( str4.Capacity() == str5.Capacity() );
		t_assert( str4.Length() == str5.Length() );
		t_assert ( str4.Capacity() >= str4.Length() );
		t_assert ( str5.Capacity() >= str5.Length() );
		t_assert( memcmp( (const char *)str4, (const char *)str5, str4.Length() ) == 0 );
	}

	// Test Methode AppendTwoHexAsChar(const char *, long, bool)
	{
		{
			String str0, str1, str2, str3;

			const char buf[7] = "005AFF";

			str0.AppendTwoHexAsChar(buf, 0);
			t_assert ( str0 == String("") );
			t_assert ( str0.Length() == 0 );

			const char exp1[1] = { 0x00 };
			str1.AppendTwoHexAsChar(buf, 1);
			t_assert ( memcmp(exp1, (const char *)str1, 1) == 0 );
			t_assert ( str1.Length() == 1 );

			const char exp2[3] = { 0x00, 0x5A, 0xff};
			str2.AppendTwoHexAsChar(buf, 3);
			t_assert ( memcmp(exp2, (const char *)str2, 3) == 0 );
			t_assert ( str2.Length() == 3 );

			str3.AppendTwoHexAsChar(buf, -1);
			t_assert ( str3 == String("") );
			t_assert ( str3.Length() == 0 );
		}
		{
			String str0, str1, str2, str3;

			const char buf[9] = "00 5A FF";

			str0.AppendTwoHexAsChar(buf, 0, true);
			t_assert ( str0 == String("") );
			t_assert ( str0.Length() == 0 );

			const char exp1[1] = { 0x00 };
			str1.AppendTwoHexAsChar(buf, 1, true);
			t_assert ( memcmp(exp1, (const char *)str1, 1) == 0 );
			t_assert ( str1.Length() == 1 );

			const char exp2[3] = { 0x00, 0x5A, 0xff};
			str2.AppendTwoHexAsChar(buf, 3, true);
			t_assert ( memcmp(exp2, (const char *)str2, 3) == 0 );
			t_assert ( str2.Length() == 3 );

			str3.AppendTwoHexAsChar(buf, -1, true);
			t_assert ( str3 == String("") );
			t_assert ( str3.Length() == 0 );
		}
	}
}

void StringTest::appendsFile ()
{
	// Init a string with a stream and a length
	String input = "01234";

	{
		IStringStream is(input);

		String str0;
		str0.Append( is, 5 );
		t_assert (str0.Capacity() >= str0.Length() );
		assertEqual ( 5, str0.Length() );
		assertEqual ( 5, strlen( (const char *)str0 ) );
		assertEqual ( "01234", str0 );
	}
	{
		IStringStream is(input);

		String  str1;
		str1.Append( is, 10 );
		t_assert (str1.Capacity() >= str1.Length()  );
		assertEqual ( 5, str1.Length() );
		assertEqual ( 5, strlen( (const char *)str1) );
		assertEqual ( "01234", str1 );
	}

	{
		IStringStream is(input);

		String  str2;
		str2.Append( is, -5 );
		t_assert ( str2.Capacity() >= str2.Length() );
	}

	istream *is3 = System::OpenStream("len5", "tst");
	t_assert( is3 != NULL );
	if ( is3 ) {
		String  str3;
		str3.Append( *is3, 0 );
		t_assert (str3 == "");
		t_assert (str3.Length() == 0);
		//t_assert (str3.Capacity() == 1);
		t_assert ( str3.Capacity() >= str3.Length() );
		delete is3;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}

	istream *is4 = System::OpenStream("len5", "tst");
	t_assert( is4 != NULL );
	if (is4) {
		String  str4;
		str4.Append( *is4, 'a' );  // 'a' = 97
		t_assert (str4 == "01234");
		t_assert (str4.Length() == 5);
		t_assert (str4.Capacity() >= 'a' + 1);
		t_assert ( str4.Capacity() >= str4.Length() );
		delete is4;
	} else {
		assertEqual("'read file len5.tst'", "'could not read len5.tst'");
	}
}

//==============================================================================================
//        M E T H O D E N
//==============================================================================================
void StringTest::methods ()
{
	// IsEqual
	isEquals();

	//	Compare    STRCMP
	compares();

	// CompareN    STRNCMP
	compareNs();

	// CopyTo
	copyTos();

	// At
	ats();

	// PutAt
	putAts();

	// ReplaceAt
	replaceAts();

	// StrChr
	strChrs();

	// StrRChr
	strRChrs ();

	// Contain
	contains();

	// SubString
	subStrings();

	// TrimFront
	trimFronts();

	// Trim
	trims();

	reserve();

	// ToLower
	toLowers();

	// ToUpper
	toUppers();

	// Dump
	dumps();

	// CaselessCompare
	caselessCompares();

	intPrintOns();
//	intReadFroms();

	asLong();

	asDouble();

	startsWith();

	asLongLong();
}

void StringTest::startsWith()
{
	t_assert(String("").StartsWith(""));
	t_assert(!String("").StartsWith(0));
	t_assert(!String("").StartsWith("a"));

	t_assert(String("a").StartsWith(""));
	t_assert(String("a").StartsWith("a"));
	t_assert(!String("a").StartsWith("ab"));
	t_assert(!String("a").StartsWith("ba"));

	t_assert(String("/foo/bar").StartsWith(""));
	t_assert(String("/foo/bar").StartsWith("/"));
	t_assert(String("/foo/bar").StartsWith("/foo"));
	t_assert(String("/foo/bar").StartsWith("/foo/bar"));
	t_assert(!String("/foo/bar").StartsWith("/foo/bar/"));
	t_assert(!String("/foo/bar").StartsWith("/fox"));
}

void StringTest::asLong()
{
	String str;

	// Test a null string -> default value
	t_assert ( str.AsLong(-1) == -1 );

	// Test a positive number
	str = "123456789";
	t_assert ( str.AsLong(-1) == 123456789 );

	// Test a negative number
	str = "-123456789";
	t_assert ( str.AsLong(-1) == -123456789 );

	// Test a positive number with ending text
	str = "123456789Hello";
	t_assert ( str.AsLong(-1) == 123456789 );

	// Test a negative number with ending text
	str = "-123456789Hello";
	t_assert ( str.AsLong(-1) == -123456789 );

	// Test a positive number with leading text
	str = "Hello123456789";
	t_assert ( str.AsLong(-1) == -1 );

	// Test a negative number with leading text -> default value
	str = "Hello-123456789";
	t_assert ( str.AsLong(-1) == -1 );
}

void StringTest::asLongLong()
{
	String str;

	// Test a null string -> default value
	t_assert ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(-1) );

	// Test a positive number smaller than 9223372036854775807 but higher than a long
	str = "12345678912345678";
	t_assert ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(12345678912345678) );

	// Test a negative number higher than -9223372036854775808
	str = "-12345678912345678";
	t_assert ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(-12345678912345678) );

	// Test a positive number with ending text
	str = "12345678912345678Hello";
	t_assert ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(12345678912345678) );

	// Test a negative number with ending text
	str = "-12345678912345678Hello";
	t_assert ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(-12345678912345678) );

	// Test a positive number with leading text
	str = "Hello12345678912345678";
	t_assert ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(-1) );

	// Test a negative number with leading text -> default value
	str = "Hello-12345678912345678";
	t_assert ( str.AsLongLong(INT64_LITERAL(-1)) == INT64_LITERAL(-1) );
}

void StringTest::asDouble()
{
	String str;

	// Test a null string -> default value
	t_assert ( str.AsDouble(-1.5) == -1.5 );

	// Test a positive number
	str = "123456789.123";
	t_assert ( str.AsDouble(-1.5) == 123456789.123 );

	// Test a negative number
	str = "-123456789.123";
	t_assert ( str.AsDouble(-1.5) == -123456789.123 );

	// Test a positive number with ending text
	str = "123456789.123Hello";
	t_assert ( str.AsDouble(-1.5) == 123456789.123 );

	// Test a negative number with ending text
	str = "-123456789.123Hello";
	t_assert ( str.AsDouble(-1.5) == -123456789.123 );

	// Test a positive number with leading text
	str = "Hello123456789.123";
	t_assert ( str.AsDouble(-1.5) == -1.5 );

	// Test a negative number with leading text -> default value
	str = "Hello-123456789.123";
	t_assert ( str.AsDouble(-1.5) == -1.5 );
}

//==============================================================================================
//        I S E Q U A L             Beginn
//===========================================================================
void StringTest::isEqual0 ()
{
	String str0;
	String str1;
	str0.Append("abc");
	str0.Append('d');
	str0.Append(-123456789L);
	str0.Append(1.123456789E+23);
	str1.Append("abc");
	str1.Append('d');
	str1.Append(-123456789L);
	str1.Append(1.123456789E+23);
	t_assert ( str0.IsEqual( (const char *)str1 ) == 1 );
	t_assert ( str1.IsEqual( (const char *)str0 ) == 1 );
	//PS? t_assert ( str0.Capacity() == str1.Capacity() );
	t_assert ( str0.Length() == str1.Length() );
	t_assert ( str0.Capacity() >= str0.Length() );
	t_assert ( str1.Capacity() >= str1.Length() );
	t_assert ( memcmp( (const char *)str0, (const char *)str1, str0.Length() ) == 0 );

	String str2 = str0;
	t_assert ( str0.IsEqual( (const char *)str2 ) == 1 );
	t_assert ( str2.IsEqual( (const char *)str0 ) == 1 );
	// t_assert ( str0.Capacity() == str2.Capacity() );
	t_assert ( str0.Length() == str2.Length() );
}

void StringTest::isEqual1 ()
{
	char buf[5] = { 'a', 'b', (char)0, 'c', 'd' };
	String str;
	str.Append( (void *) buf, 5 );
	// void * cast because otherwise Append stops at 0
	t_assert ( str.IsEqual( buf ) == 1 );
	t_assert ( str.Length() == 5 );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( memcmp( (const char *)str, buf, 5 ) == 0 );
	// IsEqual geht bis zum \0 oder bis str.Length() ? Bis \0 (ist das gewuenscht ????)
	buf[3] = 'z';		// buffer changed, c is now z
	String str1((void *)buf, 5);
	// be careful with IsEqual
	// semantics as expected with String argument
	assertEqual ( 0, str.IsEqual( str1 ) );
	// char * semantics (0 terminated) with const char * argument!
	assertEqual ( 1, str.IsEqual( buf ) );
}

void StringTest::isEqual2 ()
{
	char buf[5] = { 'a', 'b', (char)0, 'c', 'd' };
	String str0, str1 = "qqq";
	str0.Append( buf, 5 );

	t_assert( str1.IsEqual( buf ) == 0 );
	t_assert( str1.IsEqual( (const char *)str0  ) == 0);
	t_assert( str0.IsEqual( (const char *)str1  ) == 0);
}

void StringTest::isEquals ()
{
	isEqual0();
	isEqual1();
	isEqual2();
}
//==============================================================================================
//        I S E Q U A L             Ende
//==============================================================================================

//==============================================================================================
//        C O M P A R E             Beginn
//==============================================================================================
void StringTest::compare0 ()
{
	String str0, str1, str2;
	str0.Append("0123456789", 10);
	str1.Append("0123456789", 10);
	str2.Append("01234567", 8);
	t_assert( str0.Compare( (const char *)str0 ) == 0 );
	t_assert( str0.Compare( (const char *)str1 ) == 0 );
	t_assert( str1.Compare( (const char *)str0 ) == 0 );
	t_assert( str1.Compare( (const char *)str2 ) >  0 );
	t_assert( str2.Compare( (const char *)str1 ) <  0 );
}

void StringTest::compareWithNullBytes ()
{
	String str0((void *)"0123\00056789", 10);
	String str1((void *)"0123\00045678", 10);
	String str2((void *)"0123\000", 5);
	t_assert( str0.Compare( (const char *)str0 ) == 0 );
	t_assert( str0.Compare( str0 ) == 0 );
	t_assert( str0.Compare( (const char *)str1 ) ==  0 );
	// caution: comparison with a const char * stops at the first 0 character
	t_assert( str0.Compare( str1 ) >  0 );
	// comparison with a real string is safer
	t_assert( str1.Compare( (const char *)str0 ) == 0 );
	t_assert( str1.Compare( str0 ) <  0 );
	t_assert( str1.Compare( (const char *)str2 ) == 0 );
	t_assert( str1.Compare( str2 ) >  0 );
	t_assert( str2.Compare( (const char *)str1 ) == 0 );
	t_assert( str2.Compare( str1 ) <  0 );
}

void StringTest::compares ()
{
	compare0();
	compareWithNullBytes();
}
//==============================================================================================
//        C O M P A R E             Ende
//==============================================================================================

//==============================================================================================
//        C O M P A R E N            Beginn
//==============================================================================================
void StringTest::compareN0 ()
{
	String str0, str1, str2;
	str0.Append("0123456789", 10);
	str1.Append("0123456789", 10);
	str2.Append("01234567", 8);

	t_assert( str0.CompareN( (const char *)str0, str0.Length(), 0 ) == 0 );
	t_assert( str0.CompareN( (const char *)str1, str1.Length(), 0 ) == 0 );
	t_assert( str1.CompareN( (const char *)str0, str1.Length(), 0 ) == 0 );
	t_assert( str1.CompareN( (const char *)str2, str1.Length(), 0 ) >  0 );
	t_assert( str2.CompareN( (const char *)str1, str1.Length(), 0 ) <  0 );
	t_assert( str2.CompareN( (const char *)str1, str2.Length(), 0 ) ==  0 );
	t_assert( str1.CompareN( (const char *)str2, str2.Length(), 0 ) ==  0 );
}

void StringTest::compareN1 ()
{
	// Der Test geht bis zum \0
	String	str0, str1, str2, str3 = "XXXX456789", str4 = "456789";
	long	length;

	str0.Append("0123456789", 10);
	str1.Append("0123456789", 10);
	str2.Append("01234567", 8);

	for ( length = 1; length < 50; length++ ) {
		// strncmp( &str0[4], &str1[4], ...)
		t_assert( str0.CompareN( (const char *)str1 + 4, length, 4 ) == 0 );
		// strncmp( &str1[4], &str0[4], ...)
		t_assert( str1.CompareN( (const char *)str0 + 4, length, 4 ) == 0 );
		if ( (length + 4) <= str2.Length() ) {
			//  Da der Test innerhalb str1 und str2 ist, ist strncmp == 0
			// strncmp( &str2[4], &str1[4], ...)
			t_assert( str2.CompareN( (const char *)str1 + 4, length, 4 ) ==  0 );
			// strncmp( &str1[4], &str2[4], ...)
			t_assert( str1.CompareN( (const char *)str2 + 4, length, 4 ) ==  0 );
		} else {
			//  Da der Test auch ausserhalb str1 und str2 ist, ist strncmp != 0 (str1 ist laenger als str2)
			// strncmp( &str2[4], &str1[4], ...)
			t_assert( str2.CompareN( (const char *)str1 + 4, length, 4 ) <  0 );
			// strncmp( &str1[4], &str2[4], ...)
			t_assert( str1.CompareN( (const char *)str2 + 4, length, 4 ) >  0 );
		}

		// strncmp( &str1[4], &str3[4], ...)
		t_assert( str1.CompareN( (const char *)str3 + 4, length, 4 ) == 0 );
		// strncmp( &str3[4], &str1[4], ...)
		t_assert( str3.CompareN( (const char *)str1 + 4, length, 4 ) == 0 );

		// strncmp( &str1[4], &str4[0], ...)
		t_assert ( str1.CompareN( (const char *)str4, length, 4 ) == 0 );
		// strncmp( &str4[0], &str1[4], ... );
		t_assert ( str4.CompareN( (const char *)str1 + 4, length, 0 ) == 0 );
	}

	// strncmp( &str1[0], &str1[1], 2 );
	t_assert ( str1.CompareN( (const char *)str1 + 1, 2, 0 ) < 0 );

	// strncmp( &str1[0], &str1[1], 0 );
	t_assert ( str1.CompareN( (const char *)str1 + 1, 0, 0 ) == 0 );
}

void StringTest::compareNs ()
{
	compareN0();
	compareN1();
}
//==============================================================================================
//        C O M P A R E N            Ende
//==============================================================================================

//==============================================================================================
//        C O P Y T O              Beginn
//==============================================================================================
void StringTest::copyTo0 ()
{
	String str;
	char buf[50];
	str.Append( "0123456789" );
	long i, i_max, result;

	i_max = str.Length() + 5;
	for ( i = 0; i < i_max; i++ ) {
		memset( buf, 0, sizeof(buf) );
		result = str.CopyTo( buf, i, 0 );
		if ( i < str.Length() ) {
			t_assert ( memcmp( buf, (const char *)str, i ) == 0 );
			t_assert ( (long)strlen(buf) == i );
		} else {
			t_assert ( memcmp( buf, (const char *)str, str.Length() ) == 0 );
			t_assert ( (long)strlen(buf) == str.Length() );
		}
	}
}

void StringTest::copyTo1 ()
{
	// Test strlen( (const char *)str ) < str.Length():  There is a \0 within the content of the string
	char buf0[5] = { 'a', 'b', (char)0, 'c', 'd' }, buf1[50] = {0};
	String str;
	str.Append( (void *) buf0, 5 );
	// the void* cast is necessary as otherwise Append does not read past 0
	str.CopyTo( buf1, 5, 0 );
	t_assert( memcmp( buf0, buf1, 5) == 0 );
	t_assert( str.Length() == 5 );
	t_assert( str.Capacity() >= str.Length() );
}

void StringTest::copyTo2 ()
{
	// Test:  nr of char to copy is negative:  CopyTo bewirkt nichts!  OK
	char 	buf[50] = {0}, bufHlp[50] = {0};
	String	str = "0123456789";

	str.CopyTo( buf, -7, 0 );
	t_assert ( memcmp( buf, bufHlp, sizeof(buf) ) == 0 );
}

void StringTest::copyTo3 ()
{
	// Test:  copy into the buffer but not from position 0
	char buf[50] = {'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'l', 'm', 'n', 'o', 0};
	int len = strlen( buf );
	String str = "0123456789";
	str.CopyTo( buf, 5, 3 );  // len = 5    pos = 3
	t_assert( memcmp( buf, "34567fghilmno", len ) == 0 );
}

void StringTest::copyTo4 ()
{
	// Test:  copy into the buffer from a negative position
	String str = "0123456789";
	char buf[50] = {0};
	str.CopyTo( buf, 5, -2 );
}

void StringTest::copyTos ()
{
	copyTo0();
	copyTo1();
	copyTo2();
	copyTo3();
	copyTo4();
}
//==============================================================================================
//        C O P Y T O              Ende
//==============================================================================================

//==============================================================================================
//         T O L O W E R           Beginn
//==============================================================================================
void StringTest::toLower0()
{
	String	str, strHlp;
	long	i;

	for ( i = 1; i < 256; i++ ) {
		str.Append( (char)i );

		if ( ( i >= (long)'A' ) && ( i <= (long) 'Z' )  ) {
			strHlp.Append( (char)(i + (long)'a' - (long)'A' ) );
		} else {
			strHlp.Append( (char)i );
		}
	}

	str.ToLower();
	t_assert ( str == strHlp );
	t_assert ( str.Length() == strHlp.Length() );
	t_assert ( str.Capacity() == strHlp.Capacity() );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( strHlp.Capacity() >= strHlp.Length() );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );

	str.ToLower();
	t_assert ( str == strHlp );
	t_assert ( str.Length() == strHlp.Length() );
	t_assert ( str.Capacity() == strHlp.Capacity() );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( strHlp.Capacity() >= strHlp.Length() );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );

}

void StringTest::toLowers()
{
	toLower0();
}
//==============================================================================================
//         T O L O W E R          Ende
//==============================================================================================

//==============================================================================================
//         T O U P P E R          Beginn
//==============================================================================================
void StringTest::toUpper0()
{
	String	str, strHlp;
	long	i;

	for ( i = 1; i < 256; i++ ) {
		str.Append( (char)i );

		if ( ( i >= (long)'a' ) && ( i <= (long) 'z' )  ) {
			strHlp.Append( (char)(i - (long)'a' + (long)'A' ) );
		} else {
			strHlp.Append( (char)i );
		}
	}

	str.ToUpper();
	t_assert ( str == strHlp );
	t_assert ( str.Length() == strHlp.Length() );
	t_assert ( str.Capacity() == strHlp.Capacity() );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( strHlp.Capacity() >= strHlp.Length() );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );

	str.ToUpper();
	t_assert ( str == strHlp );
	t_assert ( str.Length() == strHlp.Length() );
	t_assert ( str.Capacity() == strHlp.Capacity() );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( strHlp.Capacity() >= strHlp.Length() );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::toUppers()
{
	toUpper0();
}
//==============================================================================================
//         T O P P E R         Ende
//==============================================================================================

//==============================================================================================
//         D U M P            Beginn
//==============================================================================================
void StringTest::dump0()
{
	String str = "Test for methos DUMP ...";
	str.Dump();
}

void StringTest::dumps()
{
	dump0();
}

void StringTest::IntReplaceTest(String orig, const String &pattern, const String &nStr, bool exp, const String expStr)
{
	t_assert(exp == orig.Replace(pattern, nStr));
	assertEqual(expStr, orig);
}

void StringTest::ReplaceTest()
{
	IntReplaceTest("/a/b/c", "a", "d", true, "/d/b/c");
	IntReplaceTest("/a/b/c", "/a/b/c", "d", true, "d");
	IntReplaceTest("/a/b/c", "/a/b/d", "d", false, "/a/b/c");
	IntReplaceTest("/a/b/c", "", "d", true, "d/a/b/c");
	IntReplaceTest("", "", "d", false, "");
	IntReplaceTest("", "b", "d", false, "");
}

void StringTest::DumpAsHexTest()
{
	String teststring;
	{
		teststring = "0123456789ABCDEF";
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "30 31 32 33 34 35 36 37 38 39 41 42 43 44 45 46  0123456789ABCDEF" << flush;
		teststring.DumpAsHex(os);
		assertEqual(expected, answer);
	}
	{
		teststring = "0123456789ABCDEF";
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "30 31 32 33 34 35 36 37  01234567" << endl;
		os2 << "38 39 41 42 43 44 45 46  89ABCDEF" << flush;
		teststring.DumpAsHex(os, 8L);
		assertEqual(expected, answer);
	}
	{
		teststring = "0123456789ABCDEF";
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "30 31 32 33  0123" << endl;
		os2 << "34 35 36 37  4567" << endl;
		os2 << "38 39 41 42  89AB" << endl;
		os2 << "43 44 45 46  CDEF" << flush;
		teststring.DumpAsHex(os, 4L);
		assertEqual(expected, answer);
	}
	{
		teststring = "0123456789ABCDEF";
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "30 31  01" << endl;
		os2 << "32 33  23" << endl;
		os2 << "34 35  45" << endl;
		os2 << "36 37  67" << endl;
		os2 << "38 39  89" << endl;
		os2 << "41 42  AB" << endl;
		os2 << "43 44  CD" << endl;
		os2 << "45 46  EF" << flush;
		teststring.DumpAsHex(os, 2L);
		assertEqual(expected, answer);
	}
	{
		teststring = "0123456789ABCDEF";
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "30  0" << endl;
		os2 << "31  1" << endl;
		os2 << "32  2" << endl;
		os2 << "33  3" << endl;
		os2 << "34  4" << endl;
		os2 << "35  5" << endl;
		os2 << "36  6" << endl;
		os2 << "37  7" << endl;
		os2 << "38  8" << endl;
		os2 << "39  9" << endl;
		os2 << "41  A" << endl;
		os2 << "42  B" << endl;
		os2 << "43  C" << endl;
		os2 << "44  D" << endl;
		os2 << "45  E" << endl;
		os2 << "46  F" << flush;
		teststring.DumpAsHex(os, 1L);
		assertEqual(expected, answer);
	}
	{
		teststring = "";
		for (long l = 0; l < 16; l++) {
			teststring.PutAt(l, char(l));
		}
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ................" << flush;
		teststring.DumpAsHex(os);
		assertEqual(expected, answer);
	}
	{
		teststring = "ABCDEFGHIJKLMNOPQRSTUVQWXYZ";
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F 50  ABCDEFGHIJKLMNOP" << endl;
		os2 << "51 52 53 54 55 56 51 57 58 59 5A                 QRSTUVQWXYZ     " << flush;
		teststring.DumpAsHex(os);
		assertEqual(expected, answer);
	}
	{
		teststring = "";
		for (long l = 0; l < 256; l++) {
			teststring.PutAt(l, char(l));
		}
		String expected, answer;
		OStringStream os(&answer), os2(&expected);
		os2 << "00 01 02 03 04 05 06 07 08 09 0A 0B 0C 0D 0E 0F  ................" << endl;
		os2 << "10 11 12 13 14 15 16 17 18 19 1A 1B 1C 1D 1E 1F  ................" << endl;
		os2 << "20 21 22 23 24 25 26 27 28 29 2A 2B 2C 2D 2E 2F   !" << '"' << "#$%&'()*+,-./" << endl;
		os2 << "30 31 32 33 34 35 36 37 38 39 3A 3B 3C 3D 3E 3F  0123456789:;<=>?" << endl;
		os2 << "40 41 42 43 44 45 46 47 48 49 4A 4B 4C 4D 4E 4F  @ABCDEFGHIJKLMNO" << endl;
		os2 << "50 51 52 53 54 55 56 57 58 59 5A 5B 5C 5D 5E 5F  PQRSTUVWXYZ[\\]^_" << endl;
		os2 << "60 61 62 63 64 65 66 67 68 69 6A 6B 6C 6D 6E 6F  `abcdefghijklmno" << endl;
		os2 << "70 71 72 73 74 75 76 77 78 79 7A 7B 7C 7D 7E 7F  pqrstuvwxyz{|}~." << endl;
		os2 << "80 81 82 83 84 85 86 87 88 89 8A 8B 8C 8D 8E 8F  ................" << endl;
		os2 << "90 91 92 93 94 95 96 97 98 99 9A 9B 9C 9D 9E 9F  ................" << endl;
		os2 << "A0 A1 A2 A3 A4 A5 A6 A7 A8 A9 AA AB AC AD AE AF  ................" << endl;
		os2 << "B0 B1 B2 B3 B4 B5 B6 B7 B8 B9 BA BB BC BD BE BF  ................" << endl;
		os2 << "C0 C1 C2 C3 C4 C5 C6 C7 C8 C9 CA CB CC CD CE CF  ................" << endl;
		os2 << "D0 D1 D2 D3 D4 D5 D6 D7 D8 D9 DA DB DC DD DE DF  ................" << endl;
		os2 << "E0 E1 E2 E3 E4 E5 E6 E7 E8 E9 EA EB EC ED EE EF  ................" << endl;
		os2 << "F0 F1 F2 F3 F4 F5 F6 F7 F8 F9 FA FB FC FD FE FF  ................" << flush;
		teststring.DumpAsHex(os);
		assertEqual(expected, answer);
	}
}

//==============================================================================================
//         D U M P           Ende
//==============================================================================================

//==============================================================================================
//         C A S E L E S S C O M P A R E          Beginn
//==============================================================================================
void StringTest::caselessCompare0()
{
	// Fall OHNE case
	String str = "012345";
	t_assert( str.CaselessCompare( "012345", "012345") == 0 );
	t_assert( str.CaselessCompare( (const char *)str, "012345") == 0 );
	t_assert( str.CaselessCompare( "012345", (const char *)str ) == 0 );
	t_assert( str.CaselessCompare( (const char *)str, (const char *)str ) == 0 );

	t_assert( str.CaselessCompare( "012345", "01234567") < 0 );
	t_assert( str.CaselessCompare( (const char *)str, "01234567") < 0 );
	t_assert( str.CaselessCompare( "01234567", (const char *)str ) > 0 );

	t_assert( str.CaselessCompare( "012345", "0123") > 0 );
	t_assert( str.CaselessCompare( (const char *)str, "0123") > 0 );
	t_assert( str.CaselessCompare( "0123", (const char *)str ) < 0 );
}

void StringTest::caselessCompare1()
{
	// Fall OHNE case
	String str = "abcdef";
	t_assert( str.CaselessCompare( "abcdef", "abcdef") == 0 );
	t_assert( str.CaselessCompare( (const char *)str, "abcdef") == 0 );
	t_assert( str.CaselessCompare( "abcdef", (const char *)str ) == 0 );
	t_assert( str.CaselessCompare( (const char *)str, (const char *)str ) == 0 );

	t_assert( str.CaselessCompare( "abcdef", "abcdefgh") < 0 );
	t_assert( str.CaselessCompare( (const char *)str, "abcdefgh") < 0 );
	t_assert( str.CaselessCompare( "abcdefgh", (const char *)str ) > 0 );

	t_assert( str.CaselessCompare( "abcdef", "abcd") > 0 );
	t_assert( str.CaselessCompare( (const char *)str, "abcd") > 0 );
	t_assert( str.CaselessCompare( "abcd", (const char *)str ) < 0 );
}

void StringTest::caselessCompare2()
{
	// Fall MIT case
	String str = "aBcdEf", strReverse = "AbCDeF";
	t_assert( str.CaselessCompare( "AbcdeF", "abCDef") == 0 );
	t_assert( str.CaselessCompare( (const char *)str, "ABCdef") == 0 );
	t_assert( str.CaselessCompare( "abcDEf", (const char *)str ) == 0 );
	t_assert( str.CaselessCompare( (const char *)str, (const char *)strReverse ) == 0 );
	t_assert( str.CaselessCompare( (const char *)str, (const char *)str ) == 0 );

	t_assert( str.CaselessCompare( "abcdef", "ABDDEFGH") < 0 );
	t_assert( str.CaselessCompare( "ABCDEF", "abcdefgh") < 0 );
	t_assert( str.CaselessCompare( (const char *)str, "abcdEfgh") < 0 );
	t_assert( str.CaselessCompare( "abcdefgH", (const char *)str ) > 0 );

	t_assert( str.CaselessCompare( "abcdef", "abcd") > 0 );
	t_assert( str.CaselessCompare( "ABCDEF", "abcd") > 0 );
	t_assert( str.CaselessCompare( "abcdef", "ABCD") > 0 );
	t_assert( str.CaselessCompare( "ABCDEF", "ABCD") > 0 );
	t_assert( str.CaselessCompare( (const char *)str, "abcA") > 0 );
	t_assert( str.CaselessCompare( "aBCd", (const char *)str ) < 0 );
}

void StringTest::caselessCompare3()
{
	// Fall MIT case
	String	str, strHlp, str1;
	long	i;

	for ( i = 0; i < 256; i++ ) {
		str.Append( (char)i );

		if ( ( i >= (long)'a' ) && ( i <= (long) 'z' )  ) {
			strHlp.Append( (char)(i - (long)'a' + (long)'A' ) );
		} else {
			strHlp.Append( (char)i );
		}
	}

	t_assert( str.CaselessCompare( (const char *)str, (const char *)strHlp ) == 0 );

	t_assert( strHlp.CaselessCompare( (const char *)str, (const char *)strHlp ) == 0 );

	t_assert( str1.CaselessCompare( (const char *)str, (const char *)strHlp ) == 0 );
}

void StringTest::caselessCompareWithNullPtr()
{
	String test("test");

	t_assert(String::CaselessCompare(test, "Test") == 0);
	t_assert(String::CaselessCompare(test, 0) == 1);
}

void StringTest::caselessCompares()
{
	caselessCompare0();
	caselessCompare1();
	caselessCompare2();
	caselessCompare3();
	caselessCompareWithNullPtr();
}
//==============================================================================================
//         C A S E L E S S C O M P A R E          Ende
//==============================================================================================

//==============================================================================================
//         I N T P R I N T O N               Beginn
//==============================================================================================

void StringTest::intPrintOn0()
{
	int		i;
	String	str, strHlp;

	for ( i = 0; i <= 256; i++ ) {
		if ( isprint( (unsigned char) i ) ) {
			str.Append( (char)i );
		}
	}

	// With proper Read/Write-Methods
	ostream *os = System::OpenOStream("tmp/fileprnt1", "tst");
	if ( os ) {
		str.IntPrintOn( *os, '\"');
		delete os;
		istream *is = System::OpenStream("tmp/fileprnt1", "tst");
		if ( is ) {
			String	str1;
			str1.IntReadFrom( *is, '\"');
			t_assert( str.Length() == str1.Length() );
			t_assert( memcmp( (const char *)str, (const char *)str1, str.Length() ) == 0 );
			delete is;

			// With normal Read/Write-Methods
			ostream *os1 = System::OpenOStream("tmp/fileprnt", "tst");
			if ( os1 ) {
				*os1 << str;
				delete os1;
				istream *is1 = System::OpenStream("tmp/fileprnt", "tst");
				if ( is1 ) {
					*is1 >> str1;
					delete is1;
					t_assert( str.Length() == str1.Length() + 1 );
					// the leading space is skiped
					t_assert( memcmp( (const char *)str, (const char *)str1, str.Length() ) < 0 );  // str1 does not have the leading SPACE ????
					// << and >> are not symmetric if spaces are involved!!
				} else {
					assertEqual("'read file tmp/fileprnt.tst'", "'could not read tmp/fileprnt.tst'");
				}
			} else {
				assertEqual("'write to file tmp/fileprnt.tst'", "'could not write tmp/fileprnt.tst'");
			}
		} else {
			assertEqual("'read file tmp/fileprnt1.tst'", "'could not read tmp/fileprnt1.tst'");
		}
	} else {
		assertEqual("'write to file tmp/fileprnt1.tst'", "'could not write tmp/fileprnt1.tst'");
	}
	// read from a stream with spaces
	String text = " This test ";
	String forStream;
	{
		OStringStream os2(&forStream);
		os2 << text;
	}

	IStringStream is2(forStream);

	is2 >> str;
	t_assert(str == "This");
	t_assert(is2.good() != 0);
	is2 >> str;
	t_assert(str == "test");
	t_assert(is2.good() != 0);
	t_assert(!is2.eof());		// space at the end is not eof!
	is2 >> str;
	t_assert(str == "");
	t_assert(is2.good() == 0);
	t_assert(is2.eof() != 0);

}

void StringTest::intPrintOn1()
{
	int		i;
	String	str, strHlp;

	for ( i = 0; i <= 256; i++ ) {
		if ( !isprint( (unsigned char) i ) ) {
			str.Append( (char)i );
		}
	}

	ostream *os = System::OpenOStream("tmp/filenoprnt", "tst");
	if ( os ) {
		str.IntPrintOn( *os, '\"');
		delete os;
		istream *is = System::OpenStream("tmp/filenoprnt", "tst");
		if ( is ) {
			String str1;
			str1.IntReadFrom( *is, '\"');
			delete is;
			t_assert( str.Length() == str1.Length() );
			t_assert( memcmp( (const char *)str, (const char *)str1, str.Length() ) == 0 );
		} else {
			assertEqual("'read file tmp/filenoprnt.tst'", "'could not read tmp/filenoprnt.tst'");
		}
	} else {
		assertEqual("'write to file tmp/filenoprnt.tst'", "'could not write tmp/filenoprnt.tst'");
	}
}

void StringTest::intPrintOns()
{
	intPrintOn0();
	intPrintOn1();
}
//==============================================================================================
//         I N T P R I N T O N               Ende
//==============================================================================================

//==============================================================================================
//         A T              Beginn
//==============================================================================================
void StringTest::at0 ()
{
	String		str;
	long		i;

	for ( i = 0; i < 256; i++ ) {
		str.Append( (char)i );
	}
	for ( i = 0; i < 256; i++ ) {
		t_assert( str.At(i) == (char)i );
	}

	// At out of boundaries:  result is 0  (OK!)
	t_assert( str.At(-2) == 0 );
	t_assert( str.At(300) == 0 );
	t_assert( str.At( str.Length() ) == (char)str.Length() );
	t_assert( str.At( str.Capacity() ) == 0 );
	t_assert( str.At( str.Capacity() + 1 ) == 0 );
}

void StringTest::ats ()
{
	at0 ();
}
//==============================================================================================
//         A T              Ende
//==============================================================================================

//==============================================================================================
//        P U T A T         Beginn
//==============================================================================================
void StringTest::putAt0 ()
{
	String		str;
	long 		i;

	for ( i = 0; i < 256; i++ ) {
		str.Append( ' ' );
	}

	for ( i = 0; i < 256; i++ ) {
		t_assert( str.At(i) == ' ' );
	}

	for ( i = 0; i < 256; i++ ) {
		t_assert ( str.Length() == 256 );

		str.PutAt(i, (char)i);
	}
}

void StringTest::putAts ()
{
	putAt0();
}

//==============================================================================================
//        P U T A T                  Ende
//==============================================================================================

//==============================================================================================
//        R E P L A C E A T          Beginn
//==============================================================================================
void StringTest::replaceAt0 ()
{
	String	str, strHlp;
	long	i;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos 0
	long strCapTmp, strLenTmp;
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( 0, buf, i );
		t_assert ( strCapTmp == str.Capacity() );
		t_assert ( strLenTmp == str.Length() );
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str before replacing
		memcpy( &bufHlp[0], buf, i );												// replacing
		t_assert ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt1 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos > 0
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( 10, buf, i );
		t_assert ( strCapTmp == str.Capacity() );
		t_assert ( strLenTmp == str.Length() );
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 before replacing
		memcpy( &bufHlp[10], buf, i );												// replacing
		t_assert ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt2 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos < 0 : no replacement.
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( -5, buf, i );
		t_assert ( strCapTmp == str.Capacity() );
		t_assert ( strLenTmp == str.Length() );
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
		t_assert ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAtBeyondLength ()
{
	String	str("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz");
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9' }, bufHlp[150];
	memset(bufHlp, '\0', 150);

	long length = str.Length();
	memcpy( bufHlp, (const char *)str, length );
	memcpy( bufHlp + length + 1, buf, 50);			// concat

	// Replace at pos > length :  Append
	// Replace at Length()+1
	str.ReplaceAt( str.Length() + 1, buf, 50 );
	// the length of the char* is not checked, no purify error here!

	t_assert ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	assertEqual(bufHlp, (const char *) str);
}

void StringTest::replaceAt5 ()
{
	String	str, strHlp;
	long	i;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 };

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace at pos = length-5 ---> Overflow:  Content, Length
	//	and Capacity grow and there is no problem (OK!)
	// Do not let the string grow
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str = strHlp;
		str.ReplaceAt( str.Length() - 5, buf, i );
	}

	// let the string grow
	str = strHlp;
	for ( i = 0; i <= (long)strlen( buf ); i++ ) {
		str.ReplaceAt( str.Length() - 5, buf, i );
	}
}

void StringTest::replaceAt6 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace 0 char:  no replacement
	for ( i = 0; i < str.Length() ; i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( i, buf, 0 );
		t_assert ( strCapTmp == str.Capacity() );
		t_assert ( strLenTmp == str.Length() );
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
		t_assert ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt7 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	buf[50] = { '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', 0 }, bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// Replace -3 char: no replacement
	for ( i = 0; i < str.Length() ; i++ ) {
		str = strHlp;
		strCapTmp = str.Capacity();
		strLenTmp = str.Length();
		str.ReplaceAt( i, buf, -3 );
		t_assert ( strCapTmp == str.Capacity() );
		t_assert ( strLenTmp == str.Length() );
		memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
		t_assert ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison
	}
}

void StringTest::replaceAt8 ()
{
	// code eliminated since it read past the valid contents of a const char *
	// and did not really assert the actual result
	// (which is basically impossible anyway)
}

void StringTest::replaceAt9 ()
{
	String	str, strHlp;
	long	i, strCapTmp, strLenTmp;
	char 	bufHlp[100];

	for ( i = (long)'A'; i <= (long)'Z'; i++ ) {
		strHlp.Append( (char)i );
	}
	for ( i = (long)'a'; i <= (long)'z'; i++ ) {
		strHlp.Append( (char)i );
	}

	// ReplaceAt on itsself
	str = strHlp;
	strCapTmp = str.Capacity();
	strLenTmp = str.Length();
	str.ReplaceAt( 0, (const char *)str, str.Length() );
	t_assert ( strCapTmp == str.Capacity() );
	t_assert ( strLenTmp == str.Length() );
	memcpy( bufHlp, (const char *)strHlp, strHlp.Length() );					// bufHlp = str19 (no replacement)
	t_assert ( memcmp( bufHlp, (const char *)str, str.Length() ) == 0 );		// comparison

	// ReplaceAt itsself does not work ?????
	str = strHlp;
	strCapTmp = str.Capacity();
	strLenTmp = str.Length();
	str.ReplaceAt( 1, (const char *)str, str.Length() );
	t_assert ( strCapTmp <= str.Capacity() );
	t_assert ( strLenTmp <  str.Length() );
	memcpy( &bufHlp[1], (const char *)strHlp, strHlp.Length() );					// bufHlp = the expected result
}

void StringTest::replaceAts ()
{
	replaceAt0 ();
	replaceAt1 ();
	replaceAt2 ();
	replaceAtBeyondLength ();
	replaceAt5 ();
	replaceAt6 ();
	replaceAt7 ();
	replaceAt8 ();
	replaceAt9 ();
}
//==============================================================================================
//         R E P L A C E A T         Ende
//==============================================================================================

//==============================================================================================
//         S T R C H R          Beginn
//==============================================================================================
void StringTest::strChr0 ()
{
	String	str = "0123456789";
	long	i, j;

	for ( i = 0 ; i < 10 ; i++ ) {
		for ( j = 0; j < 10; j++ ) {
			if ( i >= j ) { // (char)(i+'0') must be found at index i
				t_assert( str.StrChr( (char)(i + '0'), j ) == i );
			} else { // (char)i cannot be found
				t_assert( str.StrChr( (char)(i + '0'), j ) == -1 );
			}
		}
	}

	for ( i = 0 ; i < 256 ; i++ ) {
		if ( i == 0 ) {
			t_assert( str.StrChr( (char)i, 0 ) == -1 );
			// PT: this seems to have changed ?! Should be correct now
		} else if ( ((char)i >= '0') && ((char)i <= '9') ) {
			t_assert( str.StrChr( (char)i, 0 ) == (long)(i - '0') );
		} else {
			t_assert( str.StrChr( (char)i, 0 ) == -1 );
		}
	}
}

void StringTest::strChr1 ()
{
	String	str = "0123456789";
	long	i;

	// Exceptions
	for ( i = 0; i < 10 ; i++ ) {
		t_assert( str.StrChr( (char)(i + '0'), -5 ) == i );							// FOUND !!!
		t_assert( str.StrChr( (char)(i + '0'), str.Length() + 1 ) == -1 );
		t_assert( str.StrChr( (char)(i + '0'), str.Length() + 2 ) == -1 );
		t_assert( str.StrChr( (char)(i + '0'), str.Capacity() ) == -1 );
		t_assert( str.StrChr( (char)(i + '0'), str.Capacity() + 1 ) == -1 );
		t_assert( str.StrChr( (char)(i + '0'), str.Capacity() + 2 ) == -1 );
	}
}

void StringTest::strChrs ()
{
	strChr0();
	strChr1();
}
//==============================================================================================
//         S T R R C H R          Beginn
//==============================================================================================

//==============================================================================================
//         S T R R C H R          Beginn
//==============================================================================================
void StringTest::strRChr0 ()
{
	String	str = "0123456789";
	long	i;

	for ( i = 0 ; i < 10 ; i++ ) {
		t_assert( str.StrRChr( (char)(i + '0') ) == i );
	}

	for ( i = 0 ; i < 256 ; i++ ) {
		if ( ((char)i >= '0') && ((char)i <= '9') ) {
			assertEqual( (long)(i - '0'), str.StrRChr( (char)i ) );
		} else {
			assertEqual( -1, str.StrRChr( (char)i ) );
		}
	}
}

void StringTest::strRChrs ()
{
	strRChr0();
}
//==============================================================================================
//         S T R R C H R          Beginn
//==============================================================================================

//==============================================================================================
//         C O N T A I N           Beginn
//==============================================================================================
void StringTest::contain0 ()
{
	//	bool   Contains(const char *pattern) const;
	//----------------------------------------------
	String	str = "qwertzuiopasdfghjklyxcvbnm";
	char	buf[50];
	long	i, j, length = str.Length();

	for ( i = 0; i < length; i++ ) {
		for ( j = 1; j <= length - i; j++ ) {
			memcpy( buf, (const char *)str + i, j );
			buf[j] = 0;
			t_assert( str.Contains( buf ) > -1 );
		}
	}

	t_assert( str.Contains( "" ) > -1 );
	t_assert( str.Contains( "0" ) == -1 );
	t_assert( str.Contains( "01" ) == -1 );
	t_assert( str.Contains( "012" ) == -1 );
	t_assert( str.Contains( "0123" ) == -1 );
	t_assert( str.Contains( "01234" ) == -1 );
	t_assert( str.Contains( "012345" ) == -1 );
	t_assert( str.Contains( "0123456" ) == -1 );
	t_assert( str.Contains( "01234567" ) == -1 );
}

void StringTest::contains ()
{
	contain0 ();
}
//==============================================================================================
//         C O N T A I N           Ende
//==============================================================================================

//==============================================================================================
//         S U B S T R I N G           Beginn
//==============================================================================================

//	String SubString(const char *pattern) const;	finds first occurence of pattern in String
//----------------------------------------------------------------------------------------------
void StringTest::subString0 ()
{
	String	str = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm", strHlp;
	char	buf[50];
	long	i, j, length = str.Length() / 2;

	// SubString is done:  is the result the expected one?  Yes!!!
	for ( i = 0; i < length; i++ ) {
		for ( j = 1; j <= length - i; j++ ) {
			memcpy( buf, (const char *)str + i, j );
			buf[j] = 0;
			t_assert( memcmp( (const char *)str.SubString( buf ),
							  (const char *)str + i,							  // the pattern starts at pos 'i'
							  str.Length() - i
							) == 0
					);

			strHlp = str.SubString( buf );
			t_assert( memcmp( (const char *)strHlp, (const char *)str + i, str.Length() - i) == 0 );
			t_assert( strHlp.Length() <= strHlp.Capacity() );
			t_assert( strHlp.Length() == str.Length() - i );
		}
	}

	// After many str.SubString() is str unchanged? Yes!!!
	String	strBak = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm";
	t_assert( str.Length() == strBak.Length() );
	t_assert( str.Capacity() == strBak.Capacity() );
	t_assert( memcmp( (const char *)str, (const char *)strBak, str.Length() ) == 0 );
}

void StringTest::subString1 ()
{
	String	str = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm";

	// SubString cannot be done: result is an empty string? Yes!!! Aber nicht 100%
	// PT: Something must have changed here ...
	t_assert ( str.SubString( "" ).Length() == str.Length() );  // whole string
	t_assert ( str.SubString( "" ).Capacity() >= str.SubString( "" ).Length() );
	t_assert ( str.SubString( "" ) == str );
	t_assert ( str.SubString( "" ).Length() <= str.SubString( "" ).Capacity() );

	t_assert ( str.SubString( "0" ).Length() == 0 );
	t_assert ( str.SubString( "0" ).Capacity() >= 0 );
	t_assert ( str.SubString( "0" ).Length() <= str.SubString( "0" ).Capacity() );
}

void StringTest::subString2 ()
{
	String	str = "qwertzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm";
	// SubString with itsself
	str = str.SubString( "rtzu" );
	t_assert( memcmp( (const char *)str, "rtzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm", str.Length() ) == 0 );
	t_assert( str.Length() <= str.Capacity() );
	t_assert( str.Length() ==  (long)strlen("rtzuiopasdfghjklyxcvbnmqwertzuiopasdfghjklyxcvbnm") );
}

void StringTest::subString3 ()
{
	//	String SubString(long start, long length= -1) const;
	//----------------------------------------------------------
	String	str = "01234567890123456789", strHlp;
	long	i, j, length = str.Length();

	for ( i = 0; i < length; i++ ) {		// i=start
		for ( j = 0; j < length - i; j++ ) {	// j=nr of char to copy
			t_assert( memcmp( (const char *)str.SubString( i, j ), (const char *)str + i, j ) == 0 );
			t_assert( str.SubString( i, j ).Length() <= str.SubString( i, j ).Capacity() );
			t_assert( str.SubString( i, j ).Length() == j);

			strHlp = str.SubString( i, j );
			t_assert( memcmp( (const char *)strHlp, (const char *)str + i, j ) == 0 );
			t_assert( strHlp.Length() <= strHlp.Capacity() );
			t_assert( strHlp.Length() == j );
		}
	}
}

void StringTest::subString4 ()
{
	//	String SubString(long start, long length= -1) const;
	//----------------------------------------------------------
	String str = "01234567890123456789";
	// SubString starts at negative index: adjust start to 0 and return length chars
	t_assert ( str.SubString( -5, 3 ).Length() == 3 );
	t_assert ( str.SubString( -5, 3 ).Capacity() >= str.SubString( -5, 3 ).Length() );
	t_assert ( str.SubString( -5, 3 ) == "012" );

	// Negative Länge:  bis zum Ende
	// Es ist OK (KEIN FEHLER)
	String subStr1 = str.SubString( 5, -3 );
	// von Stelle Nummer 5 bis zum Ende (d.h. ab der 6. Position!)
	t_assert ( subStr1.Length() == (long)strlen("567890123456789") );
	t_assert ( memcmp( (const char *)subStr1, "567890123456789", subStr1.Length()) == 0 );
	t_assert ( subStr1.Capacity() >= subStr1.Length() );

	// SubString out of boundary
	// PT: must stop at the end.
	t_assert ( str.SubString( 5, 100 ).Length() == str.Length() - 5 );
	t_assert ( str.SubString( 5, 100 ).Capacity() >= str.SubString( 5, 100 ).Length() );
	t_assert ( str.SubString( 5, 100 ) == "567890123456789" );

	// After many str.SubString() is str unchanged?
	String	strHlp = "01234567890123456789";
	t_assert ( str.Length() == strHlp.Length() );
	t_assert ( str.Capacity() == strHlp.Capacity() );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::subString5 ()
{
	String str = "01234567890123456789";
	// SubString with itsself
	str = str.SubString( 2, 5 );
	t_assert( memcmp( (const char *)str, "23456", str.Length() ) == 0 );
	t_assert( str.Length() <= str.Capacity() );
	t_assert( str.Length() == 5 );
}

void StringTest::subStringWithNull ()
{
	String source((void *)"abcdefg\000hijklmn", 14);

	assertEqual("bcdefg", source.SubString(1, 6));
	String str1 = source.SubString(2, 7);
	assertEqual("cdefg\000", str1);
	t_assert(memcmp("cdefg\000", str1, 6) == 0);

	String str2 = source.SubString(3, 8);
	assertEqual("defg\000h", str2);
	t_assert(memcmp("defg\000h", str2, 6) == 0);
} // subStringWithNull

void StringTest::subStrings ()
{
	subString0();
	subString1();
	subString2();
	subString3();
	subString4();
	subString5();
	subStringWithNull();
}
//==============================================================================================
//         S U B S T R I N G           Ende
//==============================================================================================

//==============================================================================================
//         T R I M F R O N T           Beginn
//==============================================================================================
void StringTest::trimFront0 ()
{
	String	str, strHlp;
	long	i;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// Normal cases
	for ( i = 0; i < str.Length() ; i++ ) {
		str = strHlp;
		str.TrimFront(i);
		t_assert ( str.Length() == (long)strlen("0123456789") - i );
		t_assert ( str.Length() <= str.Capacity() );
	}
}

void StringTest::trimFront1 ()
{
	String str;
	str.Append( "0123456789", strlen("0123456789") );
	// newStart < 0: nothing changes
	str.TrimFront( -5 );
	t_assert ( str.Length() == (long)strlen("0123456789") );
	t_assert ( str.Length() <= str.Capacity() );
	t_assert ( memcmp( (const char *)str, "0123456789", str.Length() ) == 0 );
}

void StringTest::trimFront2 ()
{
	String str;
	str.Append( "0123456789", strlen("0123456789") );
	// newStart > Length()
	str.TrimFront( str.Length() + 1 );
	t_assert ( str.Length() == 0);
	t_assert ( str.Capacity() >=  str.Length() );
}

void StringTest::trimFrontXXX ()
{
	String tst("X");

	assertEqual(1, tst.Length());
	tst.TrimFront(1);
	assertEqual("", tst);
	assertEqual(0L, tst.Length());

}

void StringTest::trimFrontEmpty ()
{
	String tst;
	assertEqual(0, tst.Length());
	tst.TrimFront(1);
	assertEqual("", tst);
	assertEqual(0L, tst.Length());
}

void StringTest::trimFrontOneByte ()
{
	String tst("longstringmorethanone");
	long length = tst.Length();
	t_assert(tst.Length() > 1);
	tst.TrimFront(1);
	assertEqual(length - 1, tst.Length());
	assertEqual("ongstringmorethanone", tst);
}

void StringTest::trimFronts ()
{
	trimFront0();
	trimFront1();
	trimFront2();
	trimFrontOneByte();
	trimFrontXXX();
}
//==============================================================================================
//         T R I M F R O N T           Ende
//==============================================================================================

//==============================================================================================
//         T R I M                    Beginn
//==============================================================================================
void StringTest::trim0 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newlen < 0:  leeres String
	str.Trim( -1 );
	t_assert ( str.Length() == 0 );
	t_assert ( str.Capacity() >=  str.Length() );
}

void StringTest::trim1 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newlen > Length()
	str = strHlp;
	str.Trim( str.Length() + 1 );		// Trim does *not* increase the length (anymore)
	t_assert ( str.Length()   == strHlp.Length() );
	t_assert ( str.Capacity() == strHlp.Capacity() );
	t_assert ( str == strHlp );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, strHlp.Length() ) == 0 );
}

void StringTest::trim2 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newLen > Capacity()
	str = strHlp;
	long oldLength = str.Length();				// old length
	long oldCapacity = str.Capacity();		// old capacity
	str.Trim( str.Capacity() + 5 );
	// semantic change: must not change capacity or length if capacity>length
	assertEqual( oldLength, str.Length() );
	assertEqual( oldCapacity, str.Capacity() );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( str == strHlp );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, strHlp.Length() ) == 0 );
}

void StringTest::trim3 ()
{
	String str, strHlp;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newLen == 0
	str = strHlp;
	str.Trim( 0 );
	t_assert ( str.Length()   == 0 );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( str != strHlp );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::trim4 ()
{
	String str, strHlp;
	long	length;
	strHlp.Append( "0123456789", strlen("0123456789") );
	str = strHlp;
	// newLen > 0  and  < Length()
	str = strHlp;
	length = str.Length();
	str.Trim( length - 5 );
	t_assert ( str.Length()   == length - 5 );
	t_assert ( str.Capacity() >= str.Length() );
	t_assert ( str != strHlp );
	t_assert ( memcmp( (const char *)str, (const char *)strHlp, str.Length() ) == 0 );
}

void StringTest::trims ()
{
	trim0();
	trim1();
	trim2();
	trim3();

}
//==============================================================================================
//         T R I M                    Ende
//==============================================================================================

//==============================================================================================
//         T R I M R E S E R V E      Beginn
//==============================================================================================

void StringTest::reserve ()
{
	String test;

	assertEqual( test.Capacity(), 0);
	test.Reserve(9);					// you will get twice that much!
	t_assert( test.Capacity() > 9);

	long oldCapacity = test.Capacity();
	test.Append("Something");
	assertEqual( test.Length(), 9);
	t_assert( test.Capacity() > test.Length() ); 	// must be more
	assertEqual(oldCapacity, test.Capacity());
	long rest = test.Capacity() - test.Length();
	test.Reserve(rest - 1);
	// if you ask for little enough you won't get more
	assertEqual(oldCapacity, test.Capacity());
	t_assert(test.Capacity() > test.Length() + rest - 1);
	test.Reserve(rest);
	// but ask for one more and get a lot more ...
	t_assert(test.Capacity() > oldCapacity);
	t_assert(test.Capacity() > test.Length() + rest);
}

//==============================================================================================
//         R E S E R V E      Ende
//==============================================================================================
void StringTest::TestCapacity ()
{
	long i;
	String test;
	for ( i = 1; i < (3 * 4048); i++ ) {
		test.Append( 'a' );
		t_assert( test.Length() == i );
		t_assert( test.Capacity() > test.Length() );
		t_assert( ( (test.Capacity() < 8192) || ( test.Capacity() <= test.Length() + 1024 )) );
	}
}

void StringTest::GetLine()
{
	String test;
	{
		String base("First line\nSecond line\n");
		IStringStream is(&base);

		getline(is, test);
		assertEqual("First line", test);
		getline(is, test);
		assertEqual("Second line", test);
	}
	{
		String base("First line\r\nSecond line\r\n");
		IStringStream is(&base);

		getline(is, test);
		assertEqual("First line\r", test);
		getline(is, test);
		assertEqual("Second line\r", test);
	}
} // GetLine

void StringTest::OptimizedConstructorOrAssignment()
{
	const char *s = "Hello world";

	String ss(s);
	assertEqual(strlen(s) + 1, ss.Capacity());
	ss.Append(s);
	t_assert(ss.Capacity() > long(2 * strlen(s)));
	t_assert(ss.Capacity() > ss.Length());

	String t;
	assertEqual(0, t.Capacity());
	t = s;
	assertEqual(strlen(s) + 1, t.Capacity());
}

void StringTest::EmptyAllocatorTest()
{
	String str(0L);
	t_assert(str.GetAllocator() == Storage::Current());
	str = "foo";
	assertEqual("foo", str);

	String str1((Allocator *)0);
	t_assert(str1.GetAllocator() == Storage::Current());
	t_assert(str1.GetAllocator() != 0);
	str1 = "bah";
	assertEqual("bah", str1);
}

void StringTest::appendsWithDelimiter()
{
	{
		String testinput =
			"-----------------------------210003122518197\n"
			"Content-Disposition: inline; name=\"kimdojo1\"; filename=\"C:\\karate\\pics\\hank.gif\"\n"
			"\n" /* this has been the problem of String::Append(istream&,long,char) */
			"A492993f39393939f393939393f393939r393\n"
			"-----------------------------210003122518197--\n";
		IStringStream is(testinput);
		String result;
		while ( !is.eof() ) {
			result.Append(is, 4096, '\n');
			t_assert(is.eof() || is.good());
			if ('\n' == is.peek()) {
				result.Append((char)is.get());    // consume EOL
			}
		}
		assertEqual(testinput, result);
	}
	{
		String result;
		String testinput("0123456789");
		IStringStream iss(testinput);
		while ( iss.good() ) {
			result.Append(iss, 3, '\n');
		}
		assertEqual(testinput, result);
	}
	{
		String result;
		String testinput5K;
		for (long i = 0; i < 5 * 1024; i++) {
			testinput5K.Append((char)('0' + (i % 10)));
		}
		IStringStream iss(testinput5K);
		while ( iss.good() ) {
			String tmp;
			tmp.Append(iss, 4096, '\n');
			result.Append(tmp);
		}
		assertEqual(testinput5K.Length(), 5 * 1024);
		assertEqual(testinput5K, result);
	}
}

void StringTest::TestFirstCharOf()
{
	String test("0123456789");

	long lIdx;
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		String containsOneOff;
		containsOneOff.Append(lIdx);
		assertEqual(lIdx, test.FirstCharOf(containsOneOff));
	}
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		String containsOneOff;
		char c(lIdx + 70); // 70 is ascii F
		containsOneOff.Append(c);
		assertEqual(-1, test.FirstCharOf(containsOneOff));
	}
	for ( lIdx = 0; lIdx < test.Length() - 1L; lIdx++ ) {
		String containsOneOff;
		containsOneOff.Append(lIdx);
		containsOneOff.Append(lIdx + 1L);
		assertEqual(lIdx, test.FirstCharOf(containsOneOff));
	}

	// no chars given
	assertEqual(-1L, test.FirstCharOf(""));

	// String on which method is executed is empty
	String empty;
	assertEqual(-1L, empty.FirstCharOf("0123456789"));
}

void StringTest::TestContainsCharAbove()
{
	String test;
	unsigned int ui;
	for ( ui = 0; ui < 256; ui++ ) {
		test.Append((char) ui);
	}
	for (ui = 0; ui < 255; ui++) {
		assertEqual(ui + 1, test.ContainsCharAbove(ui));
	}
	assertEqual(-1, test.ContainsCharAbove(255));
	assertEqual(0, test.ContainsCharAbove(256));

	String empty;
	assertEqual(-1, empty.ContainsCharAbove(255));
	assertEqual(0, empty.ContainsCharAbove(256));
}

void StringTest::TestLastCharOf()
{
	String test("0123456789");

	long lIdx;
	String containsOneOff;
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		containsOneOff.Append(lIdx);
		assertEqual(lIdx + 1, test.LastCharOf(containsOneOff));
	}
	containsOneOff.Trim(0);
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		// like above test, and adding char not in set
		for (long llIdx = 0; llIdx < lIdx; llIdx++) {
			containsOneOff.Append(llIdx);
		}
		char c(lIdx + 70);
		containsOneOff.Append(c);
		assertEqual((lIdx == 0 ? -1 : lIdx), test.LastCharOf(containsOneOff));
		containsOneOff.Trim(0);
	}
	for ( lIdx = 0; lIdx < test.Length(); lIdx++ ) {
		// reverse order
		for (long llIdx = lIdx; llIdx >= 0; llIdx--) {
			containsOneOff.Append(llIdx);
		}
		char c(lIdx + 70);
		containsOneOff.Append(c);
		assertEqual((lIdx + 1), test.LastCharOf(containsOneOff));
		containsOneOff.Trim(0);
	}

	// no chars given
	assertEqual(-1L, test.LastCharOf(""));

	// String on which method is executed is empty
	String empty;
	assertEqual(-1L, empty.LastCharOf("0123456789"));
}
