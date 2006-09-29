/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "URLUtils.h"
#include "Dbg.h"

//--- interface include -----------------------------------------------------
#include "URLUtilsTest.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "Resolver.h"

//--- c-library modules used ---------------------------------------------------

// utility
static void Dump(ostream &os, const Anything &data, const String &str);

URLUtilsTest::URLUtilsTest (TString tname) : TestCaseType(tname) {};
URLUtilsTest::~URLUtilsTest() {};

void URLUtilsTest::CheckUrlEncodingTest()
{
	StartTrace(URLUtilsTest.CheckUrlEncodingTest);

	// --- test with default
	String arguments("abcd");
	bool ret = URLUtils::CheckUrlEncoding(arguments);
	assertEqual(true, ret);

	arguments = "abcd{";
	ret = URLUtils::CheckUrlEncoding(arguments);
	assertEqual(false, ret);

	arguments = "{abcd";
	ret = URLUtils::CheckUrlEncoding(arguments);
	assertEqual(false, ret);

	arguments = "";
	ret = URLUtils::CheckUrlEncoding(arguments);
	assertEqual(true, ret);

	// ~ is not allowed
	arguments = "$-_.+/?%~";
	ret = URLUtils::CheckUrlEncoding(arguments);
	assertEqual(false, ret);

	// empty override string
	arguments = "$-_.+/?%~";
	String override;
	ret = URLUtils::CheckUrlEncoding(arguments, override);
	assertEqual(false, ret);

	// --- test with override
	arguments = "$-_.+/?%{}";
	ret = URLUtils::CheckUrlEncoding(arguments, "{}");
	assertEqual(true, ret);

	// --- test with override
	// "# is not contained in override set
	arguments = "$-_.+/?%~#";
	ret = URLUtils::CheckUrlEncoding(arguments, "~");
	assertEqual(false, ret);

	arguments = "bubu^bubu";
	ret = URLUtils::CheckUrlEncoding(arguments, "{}");
	assertEqual(false, ret);

	// ! is additionally defined
	arguments = "$-_.+/?%!";
	ret = URLUtils::CheckUrlEncoding(arguments, "{}!");
	assertEqual(true, ret);

	arguments = "bubu!bubu";
	ret = URLUtils::CheckUrlEncoding(arguments, "!");
	assertEqual(true, ret);
}

void URLUtilsTest::CheckUrlPathContainsUnsafeCharsTest()
{
	StartTrace(URLUtilsTest.CheckUrlPathContainsUnsafeCharsTest);

	// --- testing with default set, check ascii extended
	String arguments("\\");
	bool ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments);
	assertEqual(true, ret);

	arguments = "abcd{";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments);
	assertEqual(true, ret);

	arguments = "{abcd";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments);
	assertEqual(true, ret);

	arguments = "";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments);
	assertEqual(false, ret);

	arguments = "abcd efg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments);
	assertEqual(false, ret);

	arguments = "abcdöefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments);
	assertEqual(true, ret);

	// --- testing with different set

	arguments = "abcd<\\efg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "<\\");
	assertEqual(true, ret);

	arguments = "abcd{}efg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "<\\");
	assertEqual(false, ret);

	arguments = "ab^cd\\!efg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "\\!^");
	assertEqual(true, ret);

	arguments = "ab#cdefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "\\!^");
	assertEqual(false, ret);

	// --- testing with do not check ascii extended. Note default set must be given
	// because it is a default parameter

	arguments = "abcdöefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "ö", "", 0);
	assertEqual(true, ret);

	// same test, but check for extended ascii, which will return false
	arguments = "abcdöefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "ö", "", 1);
	assertEqual(true, ret);

	// no ö present
	arguments = "abcdäüefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "ö", "", 0);
	assertEqual(false, ret);

	// no ö present
	arguments = "abcdäüefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "ö", "", 1);
	assertEqual(true, ret);

	// Exclude some extended ascii chars from check
	// no ö present
	arguments = "abcdäüefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "ö", "äü", 1);
	assertEqual(false, ret);

	// Because ö is declared an unsafe char the ascii override has no impact
	arguments = "abcdöefg";
	ret = URLUtils::CheckUrlPathContainsUnsafeChars(arguments, "ö", "ö", 1);
	assertEqual(true, ret);

}

void URLUtilsTest::CheckUrlArgEncodingTest()
{
	StartTrace(URLUtilsTest.CheckUrlArgEncodingTest);

	// --- testing different appearances of the &# token
	String arguments("abcd=+&");
	bool ret = URLUtils::CheckUrlArgEncoding(arguments);
	assertEqual(true, ret);

	arguments = "abcd{";
	ret = URLUtils::CheckUrlArgEncoding(arguments);
	assertEqual(false, ret);

	arguments = "{abcd";
	ret = URLUtils::CheckUrlArgEncoding(arguments);
	assertEqual(false, ret);

	arguments = "";
	ret = URLUtils::CheckUrlArgEncoding(arguments);
	assertEqual(true, ret);

	// Testing with different set (without $, but with >)
	arguments = "abcd$efgh";
	ret = URLUtils::CheckUrlArgEncoding(arguments, "-_.+/%=&>");
	assertEqual(false, ret);

	// Testing with different set (without $, but with >)
	arguments = "abcd>efgh";
	ret = URLUtils::CheckUrlArgEncoding(arguments, "-_.+/%=&>");
	assertEqual(true, ret);

}

void URLUtilsTest::HTMLDecodeTest()
{
	StartTrace(URLUtilsTest.HTMLDecodeTest);

	// --- testing different appearances of the &# token
	String arguments("&#x002E;");
	String answer(URLUtils::HTMLDecode(arguments));
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x01FF;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0256;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x02E;";
	answer		=	URLUtils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x1FF;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#256;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x2E;";
	answer		=	URLUtils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#46;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x9;";
	answer		=	URLUtils::HTMLDecode(arguments);
	assertEqual("\t", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#9;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("\t", answer);
	Trace("Answer: " << answer);

	// combinations
	arguments	=	"&#x002E;&#x002E;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x01FF;&#x01FF;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046;&#0046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#0256;&#0256;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x02E;&#x02E;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x1FF;&#x1FF;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#046;&#046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#256;&#256;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x2E;&#x2E;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#46;&#46;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x9;&#x9;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("\t\t", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#9;&#9;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("\t\t", answer);
	Trace("Answer: " << answer);

	// --- testing two &# tokens with no ; separator
	arguments	=	"&#x002E&#x002E;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#x002E.", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046&#0046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#0046.", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046&#0046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#0046.", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x01FF&#x01FF;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#x01FF", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#0256&#0256;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#0256", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x1FF&#x1FF;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#x1FF", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#256&#256;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#256", answer);
	Trace("Answer: " << answer);

	// --- testing two &# tokens with typos

	// this is invalid, ignore
	arguments	=	"&#xg02E;&#x002E;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// this is invalid, ignore
	arguments	=	"&#g046;&#0046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// This is above max length of expression (&#xffff;) consider as text
	arguments	=	"&#xg002E;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&#xg002E;", answer);
	Trace("Answer: " << answer);

	// This is within length of expression (&#xffff;) consider as error
	arguments	=	"&#g0046;";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	// --- testing normal text flow
	arguments	=	"&#x002E;xxx";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual(".xxx", answer);
	Trace("Answer: " << answer);

	arguments	=	"xxx&#x002E;xxx";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("xxx.xxx", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046;xxx";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual(".xxx", answer);
	Trace("Answer: " << answer);

	arguments	=	"&xxx&#0046;xxx&";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&xxx.xxx&", answer);
	Trace("Answer: " << answer);

	// First expression is above max length of expression (&#xffff;) consider as text
	arguments	=	"&xxx&#xg002E;xxx&";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&xxx&#xg002E;xxx&", answer);
	Trace("Answer: " << answer);

	// First expression is within length of expression (&#xffff;) consider as error
	arguments	=	"&xxx&#g0046;xxx&";
	answer = URLUtils::HTMLDecode(arguments);
	assertEqual("&xxxxxx&", answer);
	Trace("Answer: " << answer);
}

void URLUtilsTest::RemoveUnwantedCharsTest()
{
	StartTrace(URLUtilsTest.RemoveUnwantedCharsTest);

	String arguments("0123<4567>89");
	String badOnes("<>");
	assertEqual("0123456789", URLUtils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"0123456789";
	badOnes	 	=	"<>";
	assertEqual("0123456789", URLUtils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"";
	badOnes 	=	 "<>";
	assertEqual("", URLUtils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"";
	badOnes 	=	 "";
	assertEqual("", URLUtils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"0123<4567>89";
	badOnes		=	"<>";
	assertEqual("0123456789", URLUtils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"0>";
	badOnes		=	"<>";
	assertEqual("0", URLUtils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	">";
	badOnes		=	"<>";
	assertEqual("", URLUtils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"<0";
	badOnes		=	"<>";
	assertEqual("0", URLUtils::RemoveUnwantedChars(arguments, badOnes));
}

void URLUtilsTest::ExhaustiveHTMLDecodeTest()
{
	StartTrace(URLUtilsTest.ExhaustiveUrlDecodeTest);

	String Request;
	String Answer;
	Request = "&#x0026;#x002E;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;#x002e;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#0038;#0046;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;&#x0023;x002E;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#0038;&#0035;0046;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;&#x0023;&#x0078;002E;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#0038;&#0035;&#0048;046;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	// mixed combinations
	Request = "&#x26;&#X0023;&#120;002E;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#038;&#x23;&#048;046;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	// escapings within escaping
	Request = "&#x0026;&#x23;&#x078;&#x0032;&#054;;#x26;&#X0023;&#120;002E;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;&#x23;&#x078;&#x0032;&#054;&#x003B;#x26;&#X0023;&#120;002E;";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "<>&#x0026;&#x23;&#x078;&#x0032;&#054;&#x003B;#x26;&#X0023;&#120;002E;<>";
	Answer = URLUtils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "<>.<>", Answer );

}

void URLUtilsTest::ExhaustiveUrlDecodeTest()
{
	StartTrace(URLUtilsTest.ExhaustiveUrlDecodeTest);

	URLUtils::URLCheckStatus eUrlCheckStatus;
	String Request;
	String Answer;
	Request = "%252E";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%252e";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%25%32E";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%%%32E";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%25%32%45";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%25%32%25%34%35";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%25%32%35%25%32%25%32%3545";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%25u002E";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%25%75002E";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%25%25%25%37%35002E";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%%%25%25%32%35%25%25%37%35002E";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%%%25%25%32%35%25%25%37%35002%u0045";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "%%%25%25%32%35%25%25%37%35002%%%u0075004%u0035";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "<>%%%25%25%32%35%25%25%37%35002%u0045+++////ok";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "<>.   ////ok", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	// Preserve the +
	Request = "<>%%%25%25%32%35%25%25%37%35002%u0045+++////ok";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus, false );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "<>.+++////ok", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eOk);

	Request = "a%u772Eb";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "ab", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eSuspiciousChar);

	// %u772E in % escapes
	Request = "a%25%75%37%37%32%45b";
	Answer = URLUtils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "ab", Answer );
	t_assert(eUrlCheckStatus ==  URLUtils::eSuspiciousChar);
}

void URLUtilsTest::urlDecodeTest()
{
	StartTrace(URLUtilsTest.urlDecodeTest);
	// Check decoding escaped chars

	String Request = "E";
	String Answer = URLUtils::urlDecode( Request );
	assertCharPtrEqual( "E", Answer );

	Request = "Ein String ohne Spezialzeichen";
	Answer = URLUtils::urlDecode( Request );
	assertCharPtrEqual( "Ein String ohne Spezialzeichen", Answer );

	// +
	Request = "+";
	Answer = URLUtils::urlDecode( Request );
	assertCharPtrEqual( " ", Answer );

	Request = "+Ein String+";
	Answer = URLUtils::urlDecode( Request );
	assertCharPtrEqual( " Ein String ", Answer );

	Request = "Ein +String";
	Answer = URLUtils::urlDecode( Request );
	assertCharPtrEqual( "Ein  String", Answer );

	Request = "++++++";
	Answer = URLUtils::urlDecode( Request );
	assertCharPtrEqual( "      ", Answer );

	{
		// However: escaped plusses (%2B) must get through
		Request = "I+am+a+%2B (plus),+not a space!";
		Answer = URLUtils::urlDecode( Request );
		assertEqual( "I am a + (plus), not a space!", Answer );

		// Normal case
		Request = "%30";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "0", Answer );

		// Only Escape char
		Request = "%";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%", Answer );
		Request = "%";

		Request = "%3";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%3", Answer );

		Request = "%%30";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%30", Answer );

		Request = "% -%\"-%%-%&-%?-%/-%x-%x-";
		Answer = URLUtils::urlDecode( Request );
		String expected(" -");
		expected.Append('"').Append("-%-&-?-/-%x-%x-"); // needed to remove sniffparser complaints
		assertCharPtrEqual( expected, Answer );

		Request = "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "0123456789ABCDEFabcdef", Answer );

		Request = "";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "", Answer );

		Answer = URLUtils::urlDecode( 0L );
		assertCharPtrEqual( "", Answer );

		Request = "%2541";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%41", Answer );

		Request = "%2";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%2", Answer );

		Request = "%2e";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%2E";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%%2E%";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%2E%", Answer );
	}

	{
		// However: escaped plusses (%u002B) must get through
		Request = "I+am+a+%u002B (plus),+not a space!";
		Answer = URLUtils::urlDecode( Request );
		assertEqual( "I am a + (plus), not a space!", Answer );

		// Normal case
		Request = "%u0030";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "0", Answer );

		// Only Escape char
		Request = "%u";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%u", Answer );
		Request = "%";

		Request = "%u3";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%u3", Answer );

		Request = "%%u0030";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%u0030", Answer );

		Request = "%u -%u\"-%%u-%u&-%u?-%u/-%ux-%ux-";
		Answer = URLUtils::urlDecode( Request );
		String expected(" -");
		expected.Append('"').Append("-%u-&-?-/-%ux-%ux-"); // needed to remove sniffparser complaints
		assertCharPtrEqual( expected, Answer );

		Request = "%u0030%u0031%u0032%u0033%u0034%u0035%u0036%u0037%u0038%u0039%u0041%u0042%u0043%u0044%u0045%u0046%u0061%u0062%u0063%u0064%u0065%u0066";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "0123456789ABCDEFabcdef", Answer );

		Request = "";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "", Answer );

		Answer = URLUtils::urlDecode( 0L );
		assertCharPtrEqual( "", Answer );

		Request = "%u002541";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%41", Answer );

		Request = "%u002";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%u002", Answer );

		Request = "%u002e";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%u002E";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%%u002E%";
		Answer = URLUtils::urlDecode( Request );
		assertCharPtrEqual( "%u002E%", Answer );
	}
}

void URLUtilsTest::HTMLEscapeTest()
{
	StartTrace(URLUtilsTest.HTMLEscapeTest);

	assertEqual("abcde", URLUtils::HTMLEscape("abcde"));
	assertEqual("&#60;aTag&#47;&#62;", URLUtils::HTMLEscape("<aTag/>"));
	assertEqual("&#228;&#232;&#233;", URLUtils::HTMLEscape("äèé"));
}

void URLUtilsTest::urlEncodeTest()
{

	String Request = "Dies ist\n \"%&?/\\#{}einTestString";
	String Answer;

	Answer = URLUtils::urlEncode( Request );
	assertCharPtrEqual( "Dies%20ist%0A%20%22%25&%3F%2F%5C%23%7B%7DeinTestString", Answer );

	String exclusionSet;
	exclusionSet = "?\\";
	Answer = URLUtils::urlEncode( Request, exclusionSet );
	assertCharPtrEqual( "Dies%20ist%0A%20%22%25&?%2F\\%23%7B%7DeinTestString", Answer );
}

void URLUtilsTest::PairTest()
{

	Anything empty;
	Anything out;

	// Usual cases
	URLUtils::Pair( "Haaksdkhjf:ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "Haaksdkhjf" ].AsCharPtr() );

	out = empty;
	URLUtils::Pair( "Haaksdkh:ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkh", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "Haaksdkh" ].AsCharPtr() );

	// Slotname or content only one char
	out = empty;
	URLUtils::Pair( "H:ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "H", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "H" ].AsCharPtr() );

	out = empty;
	URLUtils::Pair( "Haaksdkhjf:a", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	assertCharPtrEqual( "a", out[ "Haaksdkhjf" ].AsCharPtr() );

	// No SlotName
	out = empty;
	URLUtils::Pair( ":ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	t_assert( 0 == out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ 0L ].AsCharPtr() );

	// Only SlotName
	out = empty;
	URLUtils::Pair( "Haaksdkhjf:", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	assertCharPtrEqual( "", out[ "Haaksdkhjf" ].AsCharPtr() );

	// No Delimiter
	out = empty;
	URLUtils::Pair( "Haaksdkhjf", ':', out );
	t_assert( out.GetSize() == 1 );
	t_assert( 0 == out.SlotName( 0 ) );
	assertCharPtrEqual( "Haaksdkhjf", out[ 0L ].AsCharPtr() );

	// Empty input
	out = empty;
	URLUtils::Pair( "", ':', out );
	assertEqual( 0, out.GetSize() );
	// no results may be returned
	t_assert( 0 == out.SlotName( 0 ) );
	assertCharPtrEqual( 0, out[ 0L ].AsCharPtr(0) );

	// Only delimiter
	out = empty;
	URLUtils::Pair( ":", ':', out );
	assertEqual( 0, out.GetSize() );
	// no results may be returned

	// Two entries with the same SlotName
	out = empty;
	URLUtils::Pair( "Haaksdkhjf:ashdfkjahgdkjfhad", ':', out );
	URLUtils::Pair( "Haaksdkhjf:blurp", ':', out );

	t_assert( out.GetSize() == 1 );
	t_assert( out[ "Haaksdkhjf" ].GetSize() == 2 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	t_assert( 0 == out[ "Haaksdkhjf" ].SlotName( 0 ) );
	t_assert( 0 == out[ "Haaksdkhjf" ].SlotName( 1 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "Haaksdkhjf" ][0L].AsCharPtr() );
	assertCharPtrEqual( "blurp", out[ "Haaksdkhjf" ][1].AsCharPtr() );

	// Input is Null-Pointer
	out = empty;
	URLUtils::Pair( 0, ':', out );
	t_assert( out.GetSize() == 0 );

	out = empty;
	URLUtils::Pair( "Haaksdkh:ashdfkjahgdk:jfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkh", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdk:jfhad", out[ "Haaksdkh" ].AsCharPtr() );
}

void URLUtilsTest::SplitTest()
{

	// A usual case
	Anything out;
	String testString = "ABCDE:EFGHI$KLMNO:PQRSTU$VWXYZ:abcde$fghiklmnop:qrstuvw";
	URLUtils::Split(testString, '$', out, ':');
	t_assert( out.GetSize() == 4 );
	assertCharPtrEqual( "ABCDE", 		out.SlotName( 0 ) );
	assertCharPtrEqual( "KLMNO", 		out.SlotName( 1 ) );
	assertCharPtrEqual( "VWXYZ", 		out.SlotName( 2 ) );
	assertCharPtrEqual( "fghiklmnop", 	out.SlotName( 3 ) );
	assertCharPtrEqual( "EFGHI", 	out["ABCDE"].AsCharPtr() );
	assertCharPtrEqual( "PQRSTU", 	out["KLMNO"].AsCharPtr() );
	assertCharPtrEqual( "abcde", 	out["VWXYZ"].AsCharPtr() );
	assertCharPtrEqual( "qrstuvw", 	out["fghiklmnop"].AsCharPtr() );

	// Only one pair
	Anything empty;
	out = empty;
	testString = "ABCDE:EFGHI";
	URLUtils::Split(testString, '$', out, ':');
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "ABCDE", 		out.SlotName( 0 ) );
	assertCharPtrEqual( "EFGHI", 	out["ABCDE"].AsCharPtr() );

	// An empty pair
	out = empty;
	testString = "ABCDE:EFGHI$$fghiklmnop:qrstuvw";
	URLUtils::Split(testString, '$', out, ':');
	assertEqual( 2, out.GetSize() );
	// empty pair is not returned
	assertCharPtrEqual( "ABCDE", 		out.SlotName( 0 ) );
	assertCharPtrEqual( "fghiklmnop", 	out.SlotName( 1 ) );
	assertCharPtrEqual( "EFGHI", 	out["ABCDE"].AsCharPtr() );
	assertCharPtrEqual( "qrstuvw", 	out["fghiklmnop"].AsCharPtr() );

	// Some special cases
	out = empty;
	testString = ":$KLMNO:PQRSTU$:$$fghiklmnop:qrstuvw$x$";
	URLUtils::Split(testString, '$', out, ':');
	assertEqual( 3, out.GetSize() );
	// only 2 decent pairs and one strange thing in above!
	assertCharPtrEqual( "KLMNO", 		out.SlotName( 0 ) );
	assertCharPtrEqual( "fghiklmnop", 	out.SlotName( 1 ) );
	t_assert( out.SlotName( 2 ) == 0 );

	assertCharPtrEqual( "PQRSTU", 	out["KLMNO"].AsCharPtr() );
	assertCharPtrEqual( "qrstuvw", 	out["fghiklmnop"].AsCharPtr() );
	assertCharPtrEqual( "x", 	out[ 2 ].AsCharPtr(0) );

	out = empty;
	testString = "form-data; name=Datei; filename=G:\\DEVELOP\\coast\\wdbase\\Application.h";
	Anything result;
	result[0L] = "form-data";
	result["NAME"] = "Datei";
	result["FILENAME"] = "G:\\DEVELOP\\coast\\wdbase\\Application.h";

	URLUtils::Split(testString, ';', out, '=', URLUtils::eUpshift);
	assertAnyEqual(out, result);

	out = empty;
	testString = "form-data; name=\"Datei\"; filename=\"G:\\DEVELOP\\coast\\wdbase\\Application.h\"";
	result = Anything();
	result[0L] = "form-data";
	result["NAME"] = "Datei";
	result["FILENAME"] = "G:\\DEVELOP\\coast\\wdbase\\Application.h";

	URLUtils::Split(testString, ';', out, '=', URLUtils::eUpshift);
	assertAnyEqual(result, out);

}

void URLUtilsTest::DecodeAllTest()
{
	// Anything is eCharPtr

	Anything Request = "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66";
	URLUtils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request.AsCharPtr() );

	// Anything is eArray with one entry

	Anything Empty;
	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	URLUtils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );

	// Anything is eArray with two entries

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	Request.Append( "Ein String mit %46 Spezialzeichen" );
	URLUtils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1].AsCharPtr() );

	// Anything is eArray with a eCharPtr and a eArray

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	Anything SecondEntry;
	SecondEntry.Append( "Ein String mit %46 Spezialzeichen" );
	Request.Append( SecondEntry );
	URLUtils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1][0L].AsCharPtr() );

	// Anything is eArray with a eCharPtr and a eArray and a eCharPtr

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	SecondEntry = Empty;
	SecondEntry.Append( "Ein String mit %46 Spezialzeichen" );
	Request.Append( SecondEntry );
	Request.Append( "Ein dritter String %30%31%32%33%34%35%36%37%38%39" );
	URLUtils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1][0L].AsCharPtr() );
	assertCharPtrEqual( "Ein dritter String 0123456789", Request[ 2].AsCharPtr() );

	// 2 Subanythings

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	SecondEntry = Empty;
	SecondEntry.Append( "Ein String mit %46 Spezialzeichen" );
	Anything ThirdEntry;
	ThirdEntry.Append( "Ein dritter String %30%31%32%33%34%35%36%37%38%39" );
	SecondEntry.Append( ThirdEntry );
	Request.Append( SecondEntry );
	URLUtils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1][0L].AsCharPtr() );
	assertCharPtrEqual( "Ein dritter String 0123456789", Request[ 1][ 1][0L].AsCharPtr() );

	// 2 Subanythings

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	SecondEntry = Empty;
	SecondEntry.Append( "Ein String mit %46 Spezialzeichen" );
	ThirdEntry = Empty;
//	ThirdEntry.Append( "Ein dritter String %30%31%32%33%34%35%36%37%38%39" );
	SecondEntry.Append( ThirdEntry );
	Request.Append( SecondEntry );
	URLUtils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1][0L].AsCharPtr() );
	t_assert( 0 == Request[ 1][ 1][0L].AsCharPtr() );
}

void Dump(ostream &os, const Anything &data, const String &str)
{
#if 0
	os << endl << "----------------------------------" << endl << "Input: " << str << endl;
	data.PrintOn(os);
	os << endl;
	os << "----------------------------------" << endl;
#endif
}

void URLUtilsTest::HandleURITest()
{
	// standard query case
	String uriQuery("cgi?param1=value1&param2=value2");
	Anything query;

	URLUtils::HandleURI(query, uriQuery);

	t_assert(query.Contains("cgi"));

	t_assert(query["param1"] == "value1");
	t_assert(query["param2"] == "value2");
	t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	query = Anything();

	// degenerated queries
	uriQuery = "cgi?";

	URLUtils::HandleURI(query, uriQuery);

	t_assert(query.Contains("cgi"));
	t_assert(query.GetSize() == 1);
	Dump(cerr, query, uriQuery);

	query = Anything();

	// degenerated queries
	uriQuery = "cgi?=&=";

	URLUtils::HandleURI(query, uriQuery);

	t_assert(query.Contains("cgi"));
	t_assert(query.GetSize() == 1);
	Dump(cerr, query, uriQuery);

	query = Anything();

	// standard path expression case
	String uriPath("cgi/param1=value1/param2=value2");

	URLUtils::HandleURI(query, uriPath);

	t_assert(query.Contains("cgi"));

	t_assert(query["param1"] == "value1");
	t_assert(query["param2"] == "value2");
	t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	query = Anything();

	// mixed case path expression case and query !!! beware of double decoding
	String uriPath1("/param1=%2541/cgi?param2=%2545");

	URLUtils::HandleURI(query, uriPath1);

	t_assert(query.Contains("cgi"));

	assertEqual("%41", query["param1"].AsCharPtr(""));
	assertEqual("%45", query["param2"].AsCharPtr(""));
	t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriPath1);
}

void URLUtilsTest::HandleURI2Test()
{
	// standard query case
	String uriQuery("https://www.hsr.ch/part1/part2/part3");
	const String testHost("www.hsr.ch");
	const String testHostIp = Resolver::DNS2IPAddress(testHost); // assume resolver is working
	Anything query;

	URLUtils::HandleURI2(query, uriQuery);
	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertEqual("/part1/part2/part3", query["Path"].AsCharPtr() );
	assertEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertEqual( "443", query["Port"].AsCharPtr());
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();

	// relative path
	uriQuery = "../part2.1/part3.1";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTPS");
	assertEqual("/part1/part2/../part2.1/part3.1", query["Path"].AsCharPtr() );
	assertEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	t_assert(query["Port"] == "443");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();

	// relative path
	uriQuery = "part3.2/part4.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTPS");
	assertEqual( "/part1/part2/../part2.1/part3.2/part4.2", query["Path"].AsCharPtr());
	assertEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	t_assert(query["Port"] == "443");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();

	// absolute path
	uriQuery = "/part1.1/part2.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTPS");
	t_assert(query["Path"] == "/part1.1/part2.2");
	assertEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	t_assert(query["Port"] == "443");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// relative path again, should strip down only to root "/" in existing path(?) or ignore this relative path
	// because .. unstacking is deeper than existing Path name allows...
	uriQuery = "../../../../part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTPS");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before??
	assertEqual( "/part1.1/../../../../part2.2/part3.2", query["Path"].AsCharPtr() ); // stripped down as far as possible and then added as if absolute in fact
	assertEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	t_assert(query["Port"] == "443");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// Protocol change->new protocol, new default Port
	uriQuery = "HTTP://www.hsr.ch/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	t_assert(query["Port"] == "80");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery = "HTTP://www.muc.de";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	assertEqual("/", query["Path"].AsCharPtr() ); // stripped down as far as possible and then added as if absolute in fact
	t_assert(query["Server"] == "193.149.48.10");
	t_assert(query["ServerName"] == "WWW.MUC.DE");
	t_assert(query["Port"] == "80");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery = "HTTP://www.muc.de/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	t_assert(query["Server"] == "193.149.48.10");
	t_assert(query["ServerName"] == "WWW.MUC.DE");
	t_assert(query["Port"] == "80");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery = "HTTPS://www.muc.de/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTPS");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	t_assert(query["Server"] == "193.149.48.10");
	t_assert(query["ServerName"] == "WWW.MUC.DE");
	t_assert(query["Port"] == "443");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery = "HTTP://www.muc.de/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	t_assert(query["Server"] == "193.149.48.10");
	t_assert(query["ServerName"] == "WWW.MUC.DE");
	t_assert(query["Port"] == "80");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery = "HTTPS://www.muc.de:1919/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTPS");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	t_assert(query["Server"] == "193.149.48.10");
	t_assert(query["ServerName"] == "WWW.MUC.DE");
	t_assert(query["Port"] == "1919");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery = "HTTPS://www.muc.de:2020/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTPS");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	t_assert(query["Server"] == "193.149.48.10");
	t_assert(query["ServerName"] == "WWW.MUC.DE");
	t_assert(query["Port"] == "2020");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery = "HTTP://www.muc.de:1919/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	t_assert(query["Server"] == "193.149.48.10");
	t_assert(query["ServerName"] == "WWW.MUC.DE");
	t_assert(query["Port"] == "1919");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// IP address directly given as domain...
	uriQuery = "HTTP://";
	uriQuery << testHostIp << "/part2.2/part3.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual(testHostIp, query["Server"].AsCharPtr());
	assertEqual(testHostIp, query["ServerName"].AsCharPtr() );
	t_assert(query["Port"] == "80");
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "part3.3";
	//String baseHREF= "http://www.muc.de:1929/base/part2.2";

	URLUtils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	//t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual( "/part2.2/part3.3", query["Path"].AsCharPtr() );
	assertEqual( testHostIp, query["Server"].AsCharPtr() );
	assertEqual( testHostIp, query["ServerName"].AsCharPtr() );
	assertEqual( "80", query["Port"].AsCharPtr() );
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "part3.2";
	String baseHREF = "http://www.muc.de:1929/base/part2.2";

	URLUtils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	//t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual( "/base/part3.2", query["Path"].AsCharPtr() );
	assertEqual( "193.149.48.10", query["Server"].AsCharPtr() );
	assertEqual( "WWW.MUC.DE", query["ServerName"].AsCharPtr() );
	assertEqual( "1929", query["Port"].AsCharPtr() );
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\'part3.2\'";
	// TEST with base href string in quotes...
	baseHREF = "\'http://www.muc.de:1939/base/part2.2\'";

	URLUtils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	//t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual( "/base/part3.2", query["Path"].AsCharPtr() );
	assertEqual( "193.149.48.10", query["Server"].AsCharPtr() );
	assertEqual( "WWW.MUC.DE", query["ServerName"].AsCharPtr() );
	assertEqual( "1939", query["Port"].AsCharPtr() );
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\"part3.2\"";
	// TEST with base href string in quotes...
	baseHREF = "\"http://www.muc.de:1949/base/part2.2\"";

	URLUtils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	//t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual( "/base/part3.2", query["Path"].AsCharPtr() );
	assertEqual( "193.149.48.10", query["Server"].AsCharPtr() );
	assertEqual( "WWW.MUC.DE", query["ServerName"].AsCharPtr() );
	assertEqual( "1949", query["Port"].AsCharPtr() );
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\"part3.2\"";
	// TEST with "/" at end of BASE HREF string
	baseHREF = "\"http://www.muc.de:1949/base/part2.2/\"";

	URLUtils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	//t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual( "/base/part2.2/part3.2", query["Path"].AsCharPtr() );
	assertEqual( "193.149.48.10", query["Server"].AsCharPtr() );
	assertEqual( "WWW.MUC.DE", query["ServerName"].AsCharPtr() );
	assertEqual( "1949", query["Port"].AsCharPtr() );
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// Mike: baseHREF must always be a full URI, if not what do browsers do? ignore it??
	// Thats what we do here
	// baseHREF w/o http:// at start is ignored...
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\"part3.4\"";
	// TEST with "/" at end of BASE HREF string
	baseHREF = "\"/base/part5.6/\"";

	URLUtils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	t_assert(query["Protocol"] == "HTTP");
	//t_assert(query["Path"] == "/part1/part2.1/part3.1"); // ignored, as before
	//t_assert(query["Path"] == "/part2.2/part3.2"); // stripped down as far as possible and then added as if absolute in fact
	assertEqual( "/base/part2.2/part3.4", query["Path"].AsCharPtr() );
	assertEqual( "193.149.48.10", query["Server"].AsCharPtr() );
	assertEqual( "WWW.MUC.DE", query["ServerName"].AsCharPtr() );
	assertEqual( "1949", query["Port"].AsCharPtr() );
	//t_assert(query.GetSize() == 3);

	Dump(cerr, query, uriQuery);

}

void URLUtilsTest::TrimBlanksTest ()
{
	String test;

	URLUtils::TrimBlanks(test);
	assertEqual("", test);

	URLUtils::TrimBlanks(test, false);
	assertEqual("", test);

	test = "NoBlanks";
	URLUtils::TrimBlanks(test);
	assertEqual("NoBlanks", test);

	test = "NoBlanks";
	URLUtils::TrimBlanks(test, false);
	assertEqual("NoBlanks", test);

	test = "  BlanksAtStartOnly";
	URLUtils::TrimBlanks(test);
	assertEqual("BlanksAtStartOnly", test);

	test = "  BlanksAtStartOnly";
	URLUtils::TrimBlanks(test, false);
	assertEqual("  BlanksAtStartOnly", test);

	test = "BlanksAtEndOnly  ";
	URLUtils::TrimBlanks(test);
	assertEqual("BlanksAtEndOnly  ", test);

	test = "BlanksAtEndOnly  ";
	URLUtils::TrimBlanks(test, false);
	assertEqual("BlanksAtEndOnly", test);

	test = "  BlanksAtStartAndEnd  ";
	URLUtils::TrimBlanks(test);
	assertEqual("BlanksAtStartAndEnd  ", test);

	test = "  BlanksAtStartAndEnd  ";
	URLUtils::TrimBlanks(test, false);
	assertEqual("  BlanksAtStartAndEnd", test);
}

Test *URLUtilsTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, URLUtilsTest, urlDecodeTest);
	ADD_CASE(testSuite, URLUtilsTest, ExhaustiveUrlDecodeTest);
	ADD_CASE(testSuite, URLUtilsTest, ExhaustiveHTMLDecodeTest);
	ADD_CASE(testSuite, URLUtilsTest, HTMLDecodeTest);
	ADD_CASE(testSuite, URLUtilsTest, urlEncodeTest);
	ADD_CASE(testSuite, URLUtilsTest, PairTest);
	ADD_CASE(testSuite, URLUtilsTest, SplitTest);
	ADD_CASE(testSuite, URLUtilsTest, DecodeAllTest);
	ADD_CASE(testSuite, URLUtilsTest, HandleURITest);
	ADD_CASE(testSuite, URLUtilsTest, HandleURI2Test);
	ADD_CASE(testSuite, URLUtilsTest, TrimBlanksTest);
	ADD_CASE(testSuite, URLUtilsTest, HTMLEscapeTest);
	ADD_CASE(testSuite, URLUtilsTest, RemoveUnwantedCharsTest);
	ADD_CASE(testSuite, URLUtilsTest, CheckUrlEncodingTest);
	ADD_CASE(testSuite, URLUtilsTest, CheckUrlArgEncodingTest);
	ADD_CASE(testSuite, URLUtilsTest, CheckUrlPathContainsUnsafeCharsTest);

	return testSuite;

}
