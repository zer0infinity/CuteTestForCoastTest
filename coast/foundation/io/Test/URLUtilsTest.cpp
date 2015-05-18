/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "URLUtilsTest.h"
#include "TestSuite.h"
#include "URLUtils.h"
#include <iostream>

namespace {
	void Dump(std::ostream &os, const Anything &data, const String &str) {
		if (TriggerEnabled(URLUtilsTest.Dump)) {//lint !e506
			os << std::endl << "----------------------------------" << std::endl << "Input: " << str << std::endl;
			data.PrintOn(os);
			os << std::endl;
			os << "----------------------------------" << std::endl;
		}
	}
}

URLUtilsTest::URLUtilsTest (TString tname) : TestCaseType(tname) {};

void URLUtilsTest::CheckUrlEncodingTest()
{
	StartTrace(URLUtilsTest.CheckUrlEncodingTest);

	// --- test with default
	String arguments("abcd");
	bool ret = coast::urlutils::CheckUrlEncoding(arguments);
	assertEqual(true, ret);

	arguments = "abcd{";
	ret = coast::urlutils::CheckUrlEncoding(arguments);
	assertEqual(false, ret);

	arguments = "{abcd";
	ret = coast::urlutils::CheckUrlEncoding(arguments);
	assertEqual(false, ret);

	arguments = "";
	ret = coast::urlutils::CheckUrlEncoding(arguments);
	assertEqual(true, ret);

	// ~ is not allowed
	arguments = "$-_.+/?%~";
	ret = coast::urlutils::CheckUrlEncoding(arguments);
	assertEqual(false, ret);

	// empty override string
	arguments = "$-_.+/?%~";
	String override;
	ret = coast::urlutils::CheckUrlEncoding(arguments, override);
	assertEqual(false, ret);

	// --- test with override
	arguments = "$-_.+/?%{}";
	ret = coast::urlutils::CheckUrlEncoding(arguments, "{}");
	assertEqual(true, ret);

	// --- test with override
	// "# is not contained in override set
	arguments = "$-_.+/?%~#";
	ret = coast::urlutils::CheckUrlEncoding(arguments, "~");
	assertEqual(false, ret);

	arguments = "bubu^bubu";
	ret = coast::urlutils::CheckUrlEncoding(arguments, "{}");
	assertEqual(false, ret);

	// ! is additionally defined
	arguments = "$-_.+/?%!";
	ret = coast::urlutils::CheckUrlEncoding(arguments, "{}!");
	assertEqual(true, ret);

	arguments = "bubu!bubu";
	ret = coast::urlutils::CheckUrlEncoding(arguments, "!");
	assertEqual(true, ret);
}

void URLUtilsTest::CheckUrlPathContainsUnsafeCharsTest()
{
	StartTrace(URLUtilsTest.CheckUrlPathContainsUnsafeCharsTest);

	// --- testing with default set, check ascii extended
	String arguments("\\");
	assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments));

	arguments = "abcd{";
	assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments));

	arguments = "{abcd";
	assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments));

	arguments = "";
	assertEqual(false, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments));

	arguments = "abcd efg";
	assertEqual(false, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments));

	char x00 = 0, xF6 = (char)246, xFC = (char)252, xE4 = (char)228;
	{
		char srcChars[] = {'a','b','c','d', xF6,'e','f','g',x00};
		assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(srcChars));
	}
	arguments = "abcd<\\efg";
	assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments, "<\\"));

	arguments = "abcd{}efg";
	assertEqual(false, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments, "<\\"));

	arguments = "ab^cd\\!efg";
	assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments, "\\!^"));

	arguments = "ab#cdefg";
	assertEqual(false, coast::urlutils::CheckUrlPathContainsUnsafeChars(arguments, "\\!^"));

	bool careAboutExtendedAscii = true, ignoreExtendedAscii = false;
	{
		char srcChars[] = {'a','b','c','d', xF6,'e','f','g',x00};
		char unsafeChars[] = {xF6, x00};
		char asciiChars[] = {x00};
		assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(srcChars, unsafeChars, asciiChars, ignoreExtendedAscii));
	}
	{
		char srcChars[] = {'a','b','c','d', xF6,'e','f','g',x00};
		char unsafeChars[] = {xF6, x00};
		char asciiChars[] = {x00};
		assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(srcChars, unsafeChars, asciiChars, careAboutExtendedAscii));
	}
	{
		char srcChars[] = {'a','b','c','d', xE4,'e','f','g',x00};
		char unsafeChars[] = {xF6, x00};
		char asciiChars[] = {x00};
		assertEqual(false, coast::urlutils::CheckUrlPathContainsUnsafeChars(srcChars, unsafeChars, asciiChars, ignoreExtendedAscii));
	}
	{
		char srcChars[] = {'a','b','c','d', xE4,'e','f','g',x00};
		char unsafeChars[] = {xF6, x00};
		char asciiChars[] = {x00};
		assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(srcChars, unsafeChars, asciiChars, careAboutExtendedAscii));
	}
	{
		char srcChars[] = {'a','b','c','d', xE4,'e','f','g',x00};
		char unsafeChars[] = {xF6, x00};
		char asciiChars[] = {xE4,xFC,x00};
		assertEqual(false, coast::urlutils::CheckUrlPathContainsUnsafeChars(srcChars, unsafeChars, asciiChars, careAboutExtendedAscii));
	}
	{
		char srcChars[] = {'a','b','c','d', xF6,'e','f','g',x00};
		char unsafeChars[] = {xF6, x00};
		char asciiChars[] = {xF6,x00};
		assertEqual(true, coast::urlutils::CheckUrlPathContainsUnsafeChars(srcChars, unsafeChars, asciiChars, careAboutExtendedAscii));
	}
}

void URLUtilsTest::CheckUrlArgEncodingTest()
{
	StartTrace(URLUtilsTest.CheckUrlArgEncodingTest);

	// --- testing different appearances of the &# token
	String arguments("abcd=+&");
	bool ret = coast::urlutils::CheckUrlArgEncoding(arguments);
	assertEqual(true, ret);

	arguments = "abcd{";
	ret = coast::urlutils::CheckUrlArgEncoding(arguments);
	assertEqual(false, ret);

	arguments = "{abcd";
	ret = coast::urlutils::CheckUrlArgEncoding(arguments);
	assertEqual(false, ret);

	arguments = "";
	ret = coast::urlutils::CheckUrlArgEncoding(arguments);
	assertEqual(true, ret);

	// Testing with different set (without $, but with >)
	arguments = "abcd$efgh";
	ret = coast::urlutils::CheckUrlArgEncoding(arguments, "-_.+/%=&>");
	assertEqual(false, ret);

	// Testing with different set (without $, but with >)
	arguments = "abcd>efgh";
	ret = coast::urlutils::CheckUrlArgEncoding(arguments, "-_.+/%=&>");
	assertEqual(true, ret);

}

void URLUtilsTest::HTMLDecodeTest()
{
	StartTrace(URLUtilsTest.HTMLDecodeTest);

	// --- testing different appearances of the &# token
	String arguments("&#x002E;");
	String answer(coast::urlutils::HTMLDecode(arguments));
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x01FF;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0256;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x02E;";
	answer		=	coast::urlutils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x1FF;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#256;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x2E;";
	answer		=	coast::urlutils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#46;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x9;";
	answer		=	coast::urlutils::HTMLDecode(arguments);
	assertEqual("\t", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#9;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("\t", answer);
	Trace("Answer: " << answer);

	// combinations
	arguments	=	"&#x002E;&#x002E;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x01FF;&#x01FF;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046;&#0046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#0256;&#0256;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x02E;&#x02E;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x1FF;&#x1FF;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#046;&#046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#256;&#256;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x2E;&#x2E;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#46;&#46;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("..", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#x9;&#x9;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("\t\t", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#9;&#9;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("\t\t", answer);
	Trace("Answer: " << answer);

	// --- testing two &# tokens with no ; separator
	arguments	=	"&#x002E&#x002E;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#x002E.", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046&#0046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#0046.", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046&#0046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#0046.", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x01FF&#x01FF;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#x01FF", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#0256&#0256;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#0256", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#x1FF&#x1FF;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#x1FF", answer);
	Trace("Answer: " << answer);

	// above hex FF
	arguments	=	"&#256&#256;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#256", answer);
	Trace("Answer: " << answer);

	// --- testing two &# tokens with typos

	// this is invalid, ignore
	arguments	=	"&#xg02E;&#x002E;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// this is invalid, ignore
	arguments	=	"&#g046;&#0046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual(".", answer);
	Trace("Answer: " << answer);

	// This is above max length of expression (&#xffff;) consider as text
	arguments	=	"&#xg002E;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&#xg002E;", answer);
	Trace("Answer: " << answer);

	// This is within length of expression (&#xffff;) consider as error
	arguments	=	"&#g0046;";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("", answer);
	Trace("Answer: " << answer);

	// --- testing normal text flow
	arguments	=	"&#x002E;xxx";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual(".xxx", answer);
	Trace("Answer: " << answer);

	arguments	=	"xxx&#x002E;xxx";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("xxx.xxx", answer);
	Trace("Answer: " << answer);

	arguments	=	"&#0046;xxx";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual(".xxx", answer);
	Trace("Answer: " << answer);

	arguments	=	"&xxx&#0046;xxx&";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&xxx.xxx&", answer);
	Trace("Answer: " << answer);

	// First expression is above max length of expression (&#xffff;) consider as text
	arguments	=	"&xxx&#xg002E;xxx&";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&xxx&#xg002E;xxx&", answer);
	Trace("Answer: " << answer);

	// First expression is within length of expression (&#xffff;) consider as error
	arguments	=	"&xxx&#g0046;xxx&";
	answer = coast::urlutils::HTMLDecode(arguments);
	assertEqual("&xxxxxx&", answer);
	Trace("Answer: " << answer);
}

void URLUtilsTest::RemoveUnwantedCharsTest()
{
	StartTrace(URLUtilsTest.RemoveUnwantedCharsTest);

	String arguments("0123<4567>89");
	String badOnes("<>");
	assertEqual("0123456789", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"0123456789";
	badOnes	 	=	"<>";
	assertEqual("0123456789", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"";
	badOnes 	=	 "<>";
	assertEqual("", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"";
	badOnes 	=	 "";
	assertEqual("", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"0123<4567>89";
	badOnes		=	"<>";
	assertEqual("0123456789", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"0>";
	badOnes		=	"<>";
	assertEqual("0", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	">";
	badOnes		=	"<>";
	assertEqual("", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));

	arguments	=	"<0";
	badOnes		=	"<>";
	assertEqual("0", coast::urlutils::RemoveUnwantedChars(arguments, badOnes));
}

void URLUtilsTest::ExhaustiveHTMLDecodeTest()
{
	StartTrace(URLUtilsTest.ExhaustiveUrlDecodeTest);

	String Request;
	String Answer;
	Request = "&#x0026;#x002E;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;#x002e;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#0038;#0046;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;&#x0023;x002E;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#0038;&#0035;0046;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;&#x0023;&#x0078;002E;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#0038;&#0035;&#0048;046;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	// mixed combinations
	Request = "&#x26;&#X0023;&#120;002E;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#038;&#x23;&#048;046;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	// escapings within escaping
	Request = "&#x0026;&#x23;&#x078;&#x0032;&#054;;#x26;&#X0023;&#120;002E;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "&#x0026;&#x23;&#x078;&#x0032;&#054;&#x003B;#x26;&#X0023;&#120;002E;";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );

	Request = "<>&#x0026;&#x23;&#x078;&#x0032;&#054;&#x003B;#x26;&#X0023;&#120;002E;<>";
	Answer = coast::urlutils::ExhaustiveHTMLDecode( Request );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "<>.<>", Answer );

}

void URLUtilsTest::ExhaustiveUrlDecodeTest()
{
	StartTrace(URLUtilsTest.ExhaustiveUrlDecodeTest);

	coast::urlutils::URLCheckStatus eUrlCheckStatus;
	String Request;
	String Answer;
	Request = "%252E";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%252e";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%25%32E";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%%%32E";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%25%32%45";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%25%32%25%34%35";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%25%32%35%25%32%25%32%3545";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%25u002E";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%25%75002E";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%25%25%25%37%35002E";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%%%25%25%32%35%25%25%37%35002E";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%%%25%25%32%35%25%25%37%35002%u0045";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "%%%25%25%32%35%25%25%37%35002%%%u0075004%u0035";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( ".", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "<>%%%25%25%32%35%25%25%37%35002%u0045+++////ok";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "<>.   ////ok", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	// Preserve the +
	Request = "<>%%%25%25%32%35%25%25%37%35002%u0045+++////ok";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus, false );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "<>.+++////ok", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eOk);

	Request = "a%u772Eb";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "ab", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eSuspiciousChar);

	// %u772E in % escapes
	Request = "a%25%75%37%37%32%45b";
	Answer = coast::urlutils::ExhaustiveUrlDecode( Request, eUrlCheckStatus );
	Trace("Answer: " << Answer);
	assertCharPtrEqual( "ab", Answer );
	t_assert(eUrlCheckStatus ==  coast::urlutils::eSuspiciousChar);
}

void URLUtilsTest::urlDecodeTest()
{
	StartTrace(URLUtilsTest.urlDecodeTest);
	// Check decoding escaped chars

	String Request = "E";
	String Answer = coast::urlutils::urlDecode( Request );
	assertCharPtrEqual( "E", Answer );

	Request = "Ein String ohne Spezialzeichen";
	Answer = coast::urlutils::urlDecode( Request );
	assertCharPtrEqual( "Ein String ohne Spezialzeichen", Answer );

	// +
	Request = "+";
	Answer = coast::urlutils::urlDecode( Request );
	assertCharPtrEqual( " ", Answer );

	Request = "+Ein String+";
	Answer = coast::urlutils::urlDecode( Request );
	assertCharPtrEqual( " Ein String ", Answer );

	Request = "Ein +String";
	Answer = coast::urlutils::urlDecode( Request );
	assertCharPtrEqual( "Ein  String", Answer );

	Request = "++++++";
	Answer = coast::urlutils::urlDecode( Request );
	assertCharPtrEqual( "      ", Answer );

	{
		// However: escaped plusses (%2B) must get through
		Request = "I+am+a+%2B (plus),+not a space!";
		Answer = coast::urlutils::urlDecode( Request );
		assertEqual( "I am a + (plus), not a space!", Answer );

		// Normal case
		Request = "%30";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "0", Answer );

		// Only Escape char
		Request = "%";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%", Answer );
		Request = "%";

		Request = "%3";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%3", Answer );

		Request = "%%30";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%30", Answer );

		Request = "% -%\"-%%-%&-%?-%/-%x-%x-";
		Answer = coast::urlutils::urlDecode( Request );
		String expected(" -");
		expected.Append('"').Append("-%-&-?-/-%x-%x-"); // needed to remove sniffparser complaints
		assertCharPtrEqual( expected, Answer );

		Request = "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "0123456789ABCDEFabcdef", Answer );

		Request = "";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "", Answer );

		Request = "%2541";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%41", Answer );

		Request = "%2";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%2", Answer );

		Request = "%2e";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%2E";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%%2E%";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%2E%", Answer );
	}

	{
		// However: escaped plusses (%u002B) must get through
		Request = "I+am+a+%u002B (plus),+not a space!";
		Answer = coast::urlutils::urlDecode( Request );
		assertEqual( "I am a + (plus), not a space!", Answer );

		// Normal case
		Request = "%u0030";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "0", Answer );

		// Only Escape char
		Request = "%u";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%u", Answer );
		Request = "%";

		Request = "%u3";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%u3", Answer );

		Request = "%%u0030";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%u0030", Answer );

		Request = "%u -%u\"-%%u-%u&-%u?-%u/-%ux-%ux-";
		Answer = coast::urlutils::urlDecode( Request );
		String expected(" -");
		expected.Append('"').Append("-%u-&-?-/-%ux-%ux-"); // needed to remove sniffparser complaints
		assertCharPtrEqual( expected, Answer );

		Request = "%u0030%u0031%u0032%u0033%u0034%u0035%u0036%u0037%u0038%u0039%u0041%u0042%u0043%u0044%u0045%u0046%u0061%u0062%u0063%u0064%u0065%u0066";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "0123456789ABCDEFabcdef", Answer );

		Request = "";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "", Answer );

		Request = "%u002541";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%41", Answer );

		Request = "%u002";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%u002", Answer );

		Request = "%u002e";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%u002E";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( ".", Answer );

		Request = "%%u002E%";
		Answer = coast::urlutils::urlDecode( Request );
		assertCharPtrEqual( "%u002E%", Answer );
	}
}

void URLUtilsTest::HTMLEscapeTest()
{
	StartTrace(URLUtilsTest.HTMLEscapeTest);

	assertEqual("abcde", coast::urlutils::HTMLEscape("abcde"));
	assertEqual("&#60;aTag&#47;&#62;", coast::urlutils::HTMLEscape("<aTag/>"));
	assertEqual("&#228;&#232;&#233;", coast::urlutils::HTMLEscape("\xe4\xe8\xe9"));
}

void URLUtilsTest::urlEncodeTest()
{

	String Request = "Dies ist\n \"%&?/\\#{}einTestString";
	String Answer;

	Answer = coast::urlutils::urlEncode( Request );
	assertCharPtrEqual( "Dies%20ist%0A%20%22%25&%3F%2F%5C%23%7B%7DeinTestString", Answer );

	String exclusionSet;
	exclusionSet = "?\\";
	Answer = coast::urlutils::urlEncode( Request, exclusionSet );
	assertCharPtrEqual( "Dies%20ist%0A%20%22%25&?%2F\\%23%7B%7DeinTestString", Answer );
}

void URLUtilsTest::PairTest()
{

	Anything empty;
	Anything out;

	// Usual cases
	coast::urlutils::Pair( "Haaksdkhjf:ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "Haaksdkhjf" ].AsCharPtr() );

	out = empty;
	coast::urlutils::Pair( "Haaksdkh:ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkh", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "Haaksdkh" ].AsCharPtr() );

	// Slotname or content only one char
	out = empty;
	coast::urlutils::Pair( "H:ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "H", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "H" ].AsCharPtr() );

	out = empty;
	coast::urlutils::Pair( "Haaksdkhjf:a", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	assertCharPtrEqual( "a", out[ "Haaksdkhjf" ].AsCharPtr() );

	// No SlotName
	out = empty;
	coast::urlutils::Pair( ":ashdfkjahgdkjfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	t_assert( 0 == out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ 0L ].AsCharPtr() );

	// Only SlotName
	out = empty;
	coast::urlutils::Pair( "Haaksdkhjf:", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	assertCharPtrEqual( "", out[ "Haaksdkhjf" ].AsCharPtr() );

	// No Delimiter
	out = empty;
	coast::urlutils::Pair( "Haaksdkhjf", ':', out );
	t_assert( out.GetSize() == 1 );
	t_assert( 0 == out.SlotName( 0 ) );
	assertCharPtrEqual( "Haaksdkhjf", out[ 0L ].AsCharPtr() );

	// Empty input
	out = empty;
	coast::urlutils::Pair( "", ':', out );
	assertEqual( 0, out.GetSize() );
	// no results may be returned
	t_assert( 0 == out.SlotName( 0 ) );
	assertCharPtrEqual( 0, out[ 0L ].AsCharPtr(0) );

	// Only delimiter
	out = empty;
	coast::urlutils::Pair( ":", ':', out );
	assertEqual( 0, out.GetSize() );
	// no results may be returned

	// Two entries with the same SlotName
	out = empty;
	coast::urlutils::Pair( "Haaksdkhjf:ashdfkjahgdkjfhad", ':', out );
	coast::urlutils::Pair( "Haaksdkhjf:blurp", ':', out );

	t_assert( out.GetSize() == 1 );
	t_assert( out[ "Haaksdkhjf" ].GetSize() == 2 );
	assertCharPtrEqual( "Haaksdkhjf", out.SlotName( 0 ) );
	t_assert( 0 == out[ "Haaksdkhjf" ].SlotName( 0 ) );
	t_assert( 0 == out[ "Haaksdkhjf" ].SlotName( 1 ) );
	assertCharPtrEqual( "ashdfkjahgdkjfhad", out[ "Haaksdkhjf" ][0L].AsCharPtr() );
	assertCharPtrEqual( "blurp", out[ "Haaksdkhjf" ][1].AsCharPtr() );

	// Input is Null-Pointer
	out = empty;
	coast::urlutils::Pair( 0, ':', out );
	t_assert( out.GetSize() == 0 );

	out = empty;
	coast::urlutils::Pair( "Haaksdkh:ashdfkjahgdk:jfhad", ':', out );
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "Haaksdkh", out.SlotName( 0 ) );
	assertCharPtrEqual( "ashdfkjahgdk:jfhad", out[ "Haaksdkh" ].AsCharPtr() );
}

void URLUtilsTest::SplitTest()
{

	// A usual case
	Anything out;
	String testString = "ABCDE:EFGHI$KLMNO:PQRSTU$VWXYZ:abcde$fghiklmnop:qrstuvw";
	coast::urlutils::Split(testString, '$', out, ':');
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
	coast::urlutils::Split(testString, '$', out, ':');
	t_assert( out.GetSize() == 1 );
	assertCharPtrEqual( "ABCDE", 		out.SlotName( 0 ) );
	assertCharPtrEqual( "EFGHI", 	out["ABCDE"].AsCharPtr() );

	// An empty pair
	out = empty;
	testString = "ABCDE:EFGHI$$fghiklmnop:qrstuvw";
	coast::urlutils::Split(testString, '$', out, ':');
	assertEqual( 2, out.GetSize() );
	// empty pair is not returned
	assertCharPtrEqual( "ABCDE", 		out.SlotName( 0 ) );
	assertCharPtrEqual( "fghiklmnop", 	out.SlotName( 1 ) );
	assertCharPtrEqual( "EFGHI", 	out["ABCDE"].AsCharPtr() );
	assertCharPtrEqual( "qrstuvw", 	out["fghiklmnop"].AsCharPtr() );

	// Some special cases
	out = empty;
	testString = ":$KLMNO:PQRSTU$:$$fghiklmnop:qrstuvw$x$";
	coast::urlutils::Split(testString, '$', out, ':');
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

	coast::urlutils::Split(testString, ';', out, '=', coast::urlutils::eUpshift);
	assertAnyEqual(out, result);

	out = empty;
	testString = "form-data; name=\"Datei\"; filename=\"G:\\DEVELOP\\coast\\wdbase\\Application.h\"";
	result = Anything();
	result[0L] = "form-data";
	result["NAME"] = "Datei";
	result["FILENAME"] = "G:\\DEVELOP\\coast\\wdbase\\Application.h";

	coast::urlutils::Split(testString, ';', out, '=', coast::urlutils::eUpshift);
	assertAnyEqual(result, out);

}

void URLUtilsTest::DecodeAllTest()
{
	// Anything is eCharPtr

	Anything Request = "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66";
	coast::urlutils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request.AsCharPtr() );

	// Anything is eArray with one entry

	Anything Empty;
	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	coast::urlutils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );

	// Anything is eArray with two entries

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	Request.Append( "Ein String mit %46 Spezialzeichen" );
	coast::urlutils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1].AsCharPtr() );

	// Anything is eArray with a eCharPtr and a eArray

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	Anything SecondEntry;
	SecondEntry.Append( "Ein String mit %46 Spezialzeichen" );
	Request.Append( SecondEntry );
	coast::urlutils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1][0L].AsCharPtr() );

	// Anything is eArray with a eCharPtr and a eArray and a eCharPtr

	Request = Empty;
	Request.Append( "%30%31%32%33%34%35%36%37%38%39%41%42%43%44%45%46%61%62%63%64%65%66" );
	SecondEntry = Empty;
	SecondEntry.Append( "Ein String mit %46 Spezialzeichen" );
	Request.Append( SecondEntry );
	Request.Append( "Ein dritter String %30%31%32%33%34%35%36%37%38%39" );
	coast::urlutils::DecodeAll( Request );
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
	coast::urlutils::DecodeAll( Request );
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
	coast::urlutils::DecodeAll( Request );
	assertCharPtrEqual( "0123456789ABCDEFabcdef", Request[0L].AsCharPtr() );
	assertCharPtrEqual( "Ein String mit F Spezialzeichen", Request[ 1][0L].AsCharPtr() );
	t_assert( 0 == Request[ 1][ 1][0L].AsCharPtr() );
}

void URLUtilsTest::HandleURITest()
{
	// standard query case
	String uriQuery("cgi?param1=value1&param2=value2");
	Anything query;

	coast::urlutils::HandleURI(query, uriQuery);

	t_assert(query.Contains("cgi"));

	t_assert(query["param1"] == "value1");
	t_assert(query["param2"] == "value2");
	t_assert(query.GetSize() == 3);

	Dump(std::cerr, query, uriQuery);

	query = Anything();

	// degenerated queries
	uriQuery = "cgi?";

	coast::urlutils::HandleURI(query, uriQuery);

	t_assert(query.Contains("cgi"));
	t_assert(query.GetSize() == 1);
	Dump(std::cerr, query, uriQuery);

	query = Anything();

	// degenerated queries
	uriQuery = "cgi?=&=";

	coast::urlutils::HandleURI(query, uriQuery);

	t_assert(query.Contains("cgi"));
	t_assert(query.GetSize() == 1);
	Dump(std::cerr, query, uriQuery);

	query = Anything();

	// standard path expression case
	String uriPath("cgi/param1=value1/param2=value2");

	coast::urlutils::HandleURI(query, uriPath);

	t_assert(query.Contains("cgi"));

	t_assert(query["param1"] == "value1");
	t_assert(query["param2"] == "value2");
	t_assert(query.GetSize() == 3);

	Dump(std::cerr, query, uriQuery);

	query = Anything();

	// mixed case path expression case and query !!! beware of double decoding
	String uriPath1("/param1=%2541/cgi?param2=%2545");

	coast::urlutils::HandleURI(query, uriPath1);

	t_assert(query.Contains("cgi"));

	assertEqual("%41", query["param1"].AsCharPtr(""));
	assertEqual("%45", query["param2"].AsCharPtr(""));
	t_assert(query.GetSize() == 3);

	Dump(std::cerr, query, uriPath1);
}

void URLUtilsTest::HandleURI2Test()
{
	ROAnything roaTestConfig=GetTestCaseConfig();
	// standard query case
	const String testHost = roaTestConfig["name"].AsString();
	const String testHostIp = roaTestConfig["ip"].AsString();
 	Anything query;
	String uriQuery(64L);

	uriQuery << "https://" << testHost << "/part1/part2/part3";

	coast::urlutils::HandleURI2(query, uriQuery);
	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "443", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual("/part1/part2/part3", query["Path"].AsCharPtr() );

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();

	// relative path
	uriQuery = "../part2.1/part3.1";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "443", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual("/part1/part2/../part2.1/part3.1", query["Path"].AsCharPtr() );

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();

	// relative path
	uriQuery = "part3.2/part4.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "443", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part1/part2/../part2.1/part3.2/part4.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();

	// absolute path
	uriQuery = "/part1.1/part2.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "443", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part1.1/part2.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// relative path again, should strip down only to root "/" in existing path(?) or ignore this relative path
	// because .. unstacking is deeper than existing Path name allows...
	uriQuery = "../../../../part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "443", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part1.1/../../../../part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// Protocol change->new protocol, new default Port
	uriQuery.Trim(0);
	uriQuery << "HTTP://" << testHost << "/part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "80", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery.Trim(0);
	uriQuery << "HTTP://" << testHost;

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "80", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery.Trim(0);
	uriQuery << "HTTP://" << testHost << "/part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "80", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery.Trim(0);
	uriQuery << "HTTPS://" << testHost << "/part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "443", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery.Trim(0);
	uriQuery << "HTTPS://" << testHost << ":1919/part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "1919", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery.Trim(0);
	uriQuery << "HTTPS://" << testHost << ":2020/part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTPS", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "2020", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// domain change
	uriQuery.Trim(0);
	uriQuery << "HTTP://" << testHost << ":1919/part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "1919", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds on previous // NO query.Free();
	// IP address directly given as domain...
	uriQuery.Trim(0);
	uriQuery << "HTTP://" << testHostIp << "/part2.2/part3.2";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "80", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHostIp, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "part3.3";

	coast::urlutils::HandleURI2(query, uriQuery);

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertCharPtrEqual( "80", query["Port"].AsCharPtr());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHostIp, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/part2.2/part3.3", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "part3.2";
	String baseHREF(32L);
	baseHREF << "HTTP://" << testHostIp << ":1929/base/part2.2";

	coast::urlutils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertEqual( 1929, query["Port"].AsLong());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHostIp, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/base/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\'part3.2\'";
	baseHREF.Trim(0);
	baseHREF << "\'http://" << testHost << ":1939/base/part2.2\'";

	coast::urlutils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertEqual( 1939, query["Port"].AsLong());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/base/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);

	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\"part3.2\"";
	baseHREF.Trim(0);
	baseHREF << "\"http://" << testHost << ":1949/base/part2.2\"";

	coast::urlutils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertEqual( 1949, query["Port"].AsLong());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/base/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\"part3.2\"";
	baseHREF.Trim(0);
	baseHREF << "\"http://" << testHost << ":1949/base/part2.2/\"";

	coast::urlutils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertEqual( 1949, query["Port"].AsLong());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/base/part2.2/part3.2", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
	//---------------------------------------------------------------------
	// Mike: baseHREF must always be a full URI, if not what do browsers do? ignore it??
	// Thats what we do here
	// baseHREF w/o http:// at start is ignored...
	// builds? on previous // NO query.Free();
	// domain change
	uriQuery = "\"part3.4\"";
	// TEST with "/" at end of BASE HREF string
	baseHREF = "\"/base/part5.6/\"";

	coast::urlutils::HandleURI2(query, uriQuery, baseHREF );

	t_assert(query.IsDefined("Protocol"));
	t_assert(query.IsDefined("Path"));
	t_assert(query.IsDefined("Server"));
	t_assert(query.IsDefined("ServerName"));
	t_assert(query.IsDefined("Port"));

	assertCharPtrEqual("HTTP", query["Protocol"].AsCharPtr());
	assertEqual( 1949, query["Port"].AsLong());
	assertCharPtrEqual(testHostIp, query["Server"].AsCharPtr());
	t_assert(!String::CaselessCompare(testHost, query["ServerName"].AsCharPtr()));
	assertCharPtrEqual( "/base/part2.2/part3.4", query["Path"].AsCharPtr());

	Dump(std::cerr, query, uriQuery);
}

void URLUtilsTest::TrimBlanksTest ()
{
	String test;

	coast::urlutils::TrimBlanks(test);
	assertEqual("", test);

	coast::urlutils::TrimBlanks(test, false);
	assertEqual("", test);

	test = "NoBlanks";
	coast::urlutils::TrimBlanks(test);
	assertEqual("NoBlanks", test);

	test = "NoBlanks";
	coast::urlutils::TrimBlanks(test, false);
	assertEqual("NoBlanks", test);

	test = "  BlanksAtStartOnly";
	coast::urlutils::TrimBlanks(test);
	assertEqual("BlanksAtStartOnly", test);

	test = "  BlanksAtStartOnly";
	coast::urlutils::TrimBlanks(test, false);
	assertEqual("  BlanksAtStartOnly", test);

	test = "BlanksAtEndOnly  ";
	coast::urlutils::TrimBlanks(test);
	assertEqual("BlanksAtEndOnly  ", test);

	test = "BlanksAtEndOnly  ";
	coast::urlutils::TrimBlanks(test, false);
	assertEqual("BlanksAtEndOnly", test);

	test = "  BlanksAtStartAndEnd  ";
	coast::urlutils::TrimBlanks(test);
	assertEqual("BlanksAtStartAndEnd  ", test);

	test = "  BlanksAtStartAndEnd  ";
	coast::urlutils::TrimBlanks(test, false);
	assertEqual("  BlanksAtStartAndEnd", test);
}

Test *URLUtilsTest::suite() {
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
