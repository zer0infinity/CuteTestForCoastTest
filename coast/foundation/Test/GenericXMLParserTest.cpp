/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "GenericXMLParserTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "GenericXMLParser.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "StringStream.h"
#include "System.h"

//---- GenericXMLParserTest ----------------------------------------------------------------
GenericXMLParserTest::GenericXMLParserTest(TString tstrName) : TestCase(tstrName)
{
	StartTrace(GenericXMLParserTest.Ctor);
}

GenericXMLParserTest::~GenericXMLParserTest()
{
	StartTrace(GenericXMLParserTest.Dtor);
}

// setup for this TestCase
void GenericXMLParserTest::setUp ()
{
	StartTrace(GenericXMLParserTest.setUp);
	istream *is = System::OpenStream("GenericXMLParserTestConfig", "any");
	if ( is ) {
		fConfig.Import( *is );
		delete is;
	} else {
		t_assertm( false, "could not read GenericXMLParserTestConfig.any" );
	}
}

void GenericXMLParserTest::tearDown ()
{
	StartTrace(GenericXMLParserTest.tearDown);
	fConfig = Anything();
}

void GenericXMLParserTest::simpleEmptyTag()
{
	StartTrace(GenericXMLParserTest.simpleEmptyTag);
	String input("<tag/>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	Anything expected;
	expected[0L]["tag"] = Anything();
	assertAnyEqual(expected, result);
	input = "<tag></tag>";
	IStringStream is2(input);
	result = p.Parse(is2);
	TraceAny(result, "result = ");
	assertAnyEqual(expected, result);
}
void GenericXMLParserTest::simpleAttributeTag()
{
	StartTrace(GenericXMLParserTest.simpleAttributeTag);
	String input("<tag at1='one' at2='2'/>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	Anything expected;
	expected[0L]["tag"]["at1"] = "one";
	expected[0L]["tag"]["at2"] = "2";

	assertAnyEqual(expected, result);
	input = "<tag at1='one' at2='2'></tag>";
	IStringStream is2(input);
	result = p.Parse(is2);
	TraceAny(result, "result = ");
	assertAnyEqual(expected, result);
}
void GenericXMLParserTest::simpleBodyTag()
{
	StartTrace(GenericXMLParserTest.simpleBodyTag);
	String input("<tag>body</tag>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	Anything expected;
	expected[0L]["tag"] = Anything();
	expected[0L][1L] = "body";
	assertAnyEqual(expected, result);
}
void GenericXMLParserTest::simpleAttributeBodyTag()
{
	StartTrace(GenericXMLParserTest.simpleAttributeBodyTag);
	String input("<tag at1='one'>body</tag>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	Anything expected;
	expected[0L]["tag"]["at1"] = "one";
	expected[0L].Append("body");

	assertAnyEqual(expected, result);
}
void GenericXMLParserTest::simpleTagWithComment()
{
	StartTrace(GenericXMLParserTest.simpleTagWithComment);
	String input("<tag at1='one'><!-- c1 -->body<!-- comment --></tag>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	Anything expected;
	expected[0L]["tag"]["at1"] = "one";
	Anything comment1;
	comment1["!--"] = " c1 ";
	expected[0L].Append(comment1);
	expected[0L].Append("body");
	Anything comment2;
	comment2["!--"] = " comment ";
	expected[0L].Append(comment2);

	assertAnyEqual(expected, result);
}
void GenericXMLParserTest::simpleNestedTags()
{
	StartTrace(GenericXMLParserTest.simpleNestedTags);
	String input("<tag>ob1<inner>body</inner>ob2</tag>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	Anything expected;
	expected[0L]["tag"] = Anything();
	expected[0L].Append("ob1");
	Anything inner;
	inner["inner"] = Anything();
	inner.Append("body");
	expected[0L].Append(inner);
	expected[0L].Append("ob2");
	assertAnyEqual(expected, result);
}
void GenericXMLParserTest::simpleExampleXML()
{
	StartTrace(GenericXMLParserTest.simpleExampleXML);
	String input("<?xml version=\"1.0\" encoding=\"ISO-8859-1\"?>"
				 "<note>\n<to>Tove</to>\n<from>Jani</from>\n<heading>Reminder</heading>\n"
				 "<body>Don't forget me this weekend!</body>\n</note>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	assertEqual("?xml", result[0L].SlotName(0L));
	assertEqual(TString(" ") << _QUOTE_(version="1.0" encoding="ISO-8859-1"), result[0L][0L].AsCharPtr());
	assertEqual("note", result[1L].SlotName(0L));
	assertEqual("\n", result[1L][1L].AsCharPtr());
	assertEqual("to", result[1L][2L].SlotName(0L));
	assertEqual("Tove", result[1L][2L][1L].AsCharPtr());
}
void GenericXMLParserTest::simpleDTDExampleXML()
{
	StartTrace(GenericXMLParserTest.simpleDTDExampleXML);
	String input("<?xml version=\"1.0\"?>"
				 "<!DOCTYPE note ["
				 "  <!ELEMENT note (to,from,heading,body)>"
				 "  <!ELEMENT to      (#PCDATA)>"
				 "  <!ELEMENT from    (#PCDATA)>"
				 "  <!ELEMENT heading (#PCDATA)>"
				 "  <!ELEMENT body    (#PCDATA)>"
				 "]>"
				 "<note>"
				 "  <to>Tove</to>"
				 "  <from>Jani</from>"
				 "  <heading>Reminder</heading>"
				 "  <body>Don't forget me this weekend</body>"
				 "</note>"
				);
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	assertEqual("?xml", result[0L].SlotName(0L));
	assertEqual(TString(" ") << _QUOTE_(version="1.0"), result[0L][0L].AsCharPtr());
	assertEqual("!DOCTYPE", result[1L].SlotName(0L));

	assertEqual("note", result[2L].SlotName(0L));
	assertEqual("to", result[2L][2L].SlotName(0L));
	assertEqual("Tove", result[2L][2L][1L].AsCharPtr());
}
void GenericXMLParserTest::simpleParsePrint()
{
	StartTrace(GenericXMLParserTest.simpleParsePrint);
	String input("<?xml version=\"1.0\"?>"
				 "<!DOCTYPE note ["
				 "<!ELEMENT note (to,from,heading,body)>"
				 "<!ELEMENT to      (#PCDATA)>"
				 "<!ELEMENT from    (#PCDATA)>"
				 "<!ELEMENT heading (#PCDATA)>"
				 "<!ELEMENT body    (#PCDATA)>"
				 "]>"
				 "<note>"
				 "  <to>Tove</to>"
				 "  <from>Jani</from>"
				 "  <heading>Reminder</heading>"
				 "  <body>Don't forget me this weekend</body>"
				 "</note>"
				);
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	OStringStream os;
	GenericXMLPrinter::PrintXml(os, result);
	assertEqual(input, os.str());
}

void GenericXMLParserTest::simpleXMLError()
{
	StartTrace(GenericXMLParserTest.simpleXMLError);
	String input("<tag></bar>");
	IStringStream is(input);
	GenericXMLParser p;
	Anything result = p.Parse(is);
	TraceAny(result, "result = ");
	t_assert(result.IsDefined("Errors"));
}

void GenericXMLParserTest::configuredTests()
{
	StartTrace(GenericXMLParserTest.configuredTests);

	ROAnything tConfig(fConfig["ConfiguredTests"]);
	t_assert(!tConfig.IsNull());
	ROAnything cConfig(tConfig[0L]);
	TString cName(tConfig.SlotName(0L));
	for (long i = 0; i < tConfig.GetSize(); i++, cConfig = tConfig[i], cName = tConfig.SlotName(i)) {
		String strInput;
		if ( cConfig["Input"].GetType() == AnyArrayType ) {
			for (long j = 0; j < cConfig["Input"].GetSize(); j++) {
				strInput << cConfig["Input"][j].AsString();
			}
		} else {
			strInput = cConfig["Input"].AsString();
		}
		IStringStream iss(strInput);
		GenericXMLParser p;
		Anything result = p.Parse(iss);
		assertAnyEqualm(cConfig["Expected"], result, cName);
	}
}

// builds up a suite of testcases, add a line for each testmethod
Test *GenericXMLParserTest::suite ()
{
	StartTrace(GenericXMLParserTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, GenericXMLParserTest, simpleEmptyTag);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleAttributeTag);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleBodyTag);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleAttributeBodyTag);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleTagWithComment);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleNestedTags);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleExampleXML);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleDTDExampleXML);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleXMLError);
	ADD_CASE(testSuite, GenericXMLParserTest, simpleParsePrint);
	ADD_CASE(testSuite, GenericXMLParserTest, configuredTests);

	return testSuite;
}
