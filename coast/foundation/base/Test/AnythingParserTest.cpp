/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingParserTest.h"
#include "TestSuite.h"
#include "StringStream.h"
#include "SystemFile.h"
#include "IFAObject.h"
#include <iostream>

using namespace coast;

AnythingParserTest::AnythingParserTest(TString tname) :
		TestCaseType(tname), lineCounter(0) {
}

void AnythingParserTest::setUp() {
	Anything undefAny;
	emptyAny = undefAny;
	anyTemp0 = undefAny;
	anyTemp1 = undefAny;
	anyTemp2 = undefAny;
	anyTemp3 = undefAny;
	anyTemp4 = undefAny;
}

Test *AnythingParserTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnythingParserTest, parseSimpleTypeCharPtr );
	ADD_CASE(testSuite, AnythingParserTest, parseSimpleTypeDouble);
	ADD_CASE(testSuite, AnythingParserTest, parseSimpleTypeLong);
	ADD_CASE(testSuite, AnythingParserTest, parseSimpleTypeNull );
	ADD_CASE(testSuite, AnythingParserTest, parseNumber );
	ADD_CASE(testSuite, AnythingParserTest, parseOctal );
	ADD_CASE(testSuite, AnythingParserTest, parseBinary );
	ADD_CASE(testSuite, AnythingParserTest, parseMixedAnything );
	ADD_CASE(testSuite, AnythingParserTest, importEmptyStream );
	ADD_CASE(testSuite, AnythingParserTest, readWriteAnything );
	ADD_CASE(testSuite, AnythingParserTest, parseTestFiles );
	ADD_CASE(testSuite, AnythingParserTest, testObjectParsing );
	ADD_CASE(testSuite, AnythingParserTest, testNumberParsing );
	return testSuite;
}

void AnythingParserTest::checkImportExport(Anything any, String fileName) {
	String buf1;
	String buf2;
	Anything anyHlp;
	OStringStream os1(&buf1);
	any.Export(os1);
	IStringStream is1(buf1);
	anyHlp.Import(is1);
	OStringStream os2(&buf2);
	anyHlp.Export(os2);
	assertCharPtrEqual(buf1, buf2);
}

void AnythingParserTest::writeResult(String *input, long nrOfElt, const char *path, const char *ext) {
	std::ostream *os = system::OpenOStream(path, ext, std::ios::trunc);
	if (os) {
		Anything emptyAny1, anyTest, anyTests;
		AnythingParserTest::anyOutput = emptyAny1;
		AnythingParserTest::lineCounter = 1;
		long i;
		for (i = 0; i < nrOfElt; i++) {
			IStringStream is(input[i]);
			anyTest.Import(is);
			anyTests.Append(anyTest);
		}
		scanAnything(anyTests);
		*os << AnythingParserTest::anyOutput;
		delete os;
		AnythingParserTest::anyOutput = emptyAny1;
		AnythingParserTest::lineCounter = 1;
	} else {
		String tmp0 = "write ";
		tmp0.Append(path);
		tmp0.Append(".");
		tmp0.Append(ext);
		String tmp1 = "could not write ";
		tmp1.Append(path);
		tmp1.Append(".");
		tmp1.Append(ext);
		assertEqual((const char * )tmp0, (const char * )tmp1);
	}
}

void AnythingParserTest::scanAnything(Anything any0) {
	long i, iMax = any0.GetSize();
	String slotNm;
	Anything anyTest;
	String buffer;
	for (i = 0; i < iMax; i++) {
		anyTest = any0[i];
		slotNm = "";
		slotNm.Append(AnythingParserTest::lineCounter++);
		slotNm.Append(") ");
		slotNm.Append(any0.SlotName(i));
		slotNm.Append(" Size:");
		slotNm.Append(anyTest.GetSize());
		slotNm.Append("; Type:");
		switch (anyTest.GetType()) {
		case AnyLongType: {
			slotNm.Append( "eLong" );
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsLong();
		}
		break;

		case AnyDoubleType: {
			slotNm.Append( "eDouble" );
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsDouble();
		}
		break;

		case AnyNullType: {
			slotNm.Append( "eNull" );
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = "NULL";
		}
		break;

		case AnyCharPtrType: {
			slotNm.Append( "eCharPtr" );
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsCharPtr();
		}
		break;

		case AnyArrayType: {
			slotNm.Append( "eArray" );
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = "...";
			AnythingParserTest::scanAnything( anyTest );
		}
		break;

		case AnyVoidBufType: {
			slotNm.Append( "eVoidBuf" );
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsCharPtr();
		}
		break;

		case AnyObjectType: {
			slotNm.Append( "eObject" );
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = anyTest;
		}
		break;

		default: {
			String str("???"); //lint !e585
			slotNm.Append( "???" );//lint !e585
			slotNm.Append( "; Value: " );
			AnythingParserTest::anyOutput[ (const char *)slotNm ] = str;
		}
		break;
	}
}
}

Anything AnythingParserTest::storeAndReload(Anything any) {
	String buf;
	OStringStream os(&buf);
	any.Export(os);
	IStringStream is(buf);
	Anything anyResult;
	anyResult.Import(is);
	return (anyResult);
}

void AnythingParserTest::IntParseSimpleTypeLong(const String &inp, long exp) { //lint !e578
	Anything anyTest;
	IStringStream is0(inp);
	anyTest.Import(is0);
	assertEqual(AnyLongType, anyTest.GetType());
	assertEqual(exp, anyTest.AsLong());
	assertEqual(1, anyTest.GetSize());
}

void AnythingParserTest::parseSimpleTypeLong() {
	Anything anyTest;

	IntParseSimpleTypeLong("0", 0);
	IntParseSimpleTypeLong("1234567890", 1234567890);
	IntParseSimpleTypeLong("-1234567890", -1234567890);
	IntParseSimpleTypeLong("783051076", 783051076L);

	// test unsigned semantic for hex and oct
	IntParseSimpleTypeLong("0x2eac6944", static_cast<long>(783051076UL));
	IntParseSimpleTypeLong("05653064504", static_cast<long>(783051076UL));
	IntParseSimpleTypeLong("0xcffe007f", static_cast<long>(3489529983UL));
	IntParseSimpleTypeLong("031777400177", static_cast<long>(3489529983UL));

	String input4 = "9999999999999";
	IStringStream is4(input4);
	anyTest.Import(is4);
	assertEqual(AnyNullType, anyTest.GetType());
	assertEqual(0, anyTest.GetSize());

	String input5 = "-9999999999999";
	IStringStream is5(input5);
	anyTest.Import(is5);
	assertEqual(AnyNullType, anyTest.GetType());
	assertEqual(0, anyTest.GetSize());

	String inputx = "0xfffffffe";
	IStringStream isx(inputx);
	anyTest.Import(isx);
	assertEqual(0xfffffffe, static_cast<unsigned long>(anyTest.AsLong(0)));	//lint !e569
	assertEqual(AnyLongType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());
}

void AnythingParserTest::parseSimpleTypeDouble() {
	Anything anyTest;

	String input0 = "0.0";
	IStringStream is0(input0);
	anyTest.Import(is0);
	assertDoublesEqual(0.0, anyTest.AsDouble(), 0.00001);
	assertEqual(AnyDoubleType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());

	String input1 = "1.0123456789E+300";
	IStringStream is1(input1);
	anyTest.Import(is1);
	assertDoublesEqual(1.0123456789E+300, anyTest.AsDouble(), 0.00001);
	assertEqual(AnyDoubleType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());

	String input2 = "-1.0123456789E+300";
	IStringStream is2(input2);
	anyTest.Import(is2);
	assertDoublesEqual(-1.0123456789E+300, anyTest.AsDouble(), 0.00001);
	assertEqual(AnyDoubleType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());

	String input3 = "1.0123456789E-300";
	IStringStream is3(input3);
	anyTest.Import(is3);
	assertDoublesEqual(1.0123456789E-300, anyTest.AsDouble(), 0.00001);
	assertEqual(AnyDoubleType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());

	String input4 = "-1.0123456789E-300";
	IStringStream is4(input4);
	anyTest.Import(is4);
	assertDoublesEqual(-1.0123456789E-300, anyTest.AsDouble(), 0.00001);
	assertEqual(AnyDoubleType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());

	double const doubleDefault = 1.234;
	String input5 = "-1.0123456789E-350";
	IStringStream is5(input5);
	anyTest.Import(is5);
	assertDoublesEqual(doubleDefault, anyTest.AsDouble(doubleDefault), 0.00001);
	assertEqual(AnyNullType, anyTest.GetType());

	String input6 = "1.0123456789E-350";
	IStringStream is6(input6);
	anyTest.Import(is6);
	assertDoublesEqual(doubleDefault, anyTest.AsDouble(doubleDefault), 0.00001);
	assertEqual(AnyNullType, anyTest.GetType());

	String input7 = "1.0123456789E+350";
	IStringStream is7(input7);
	anyTest.Import(is7);
	assertDoublesEqual(doubleDefault, anyTest.AsDouble(doubleDefault), 0.00001);
	assertEqual(AnyNullType, anyTest.GetType());

	String input8 = "-1.0123456789E+350";
	IStringStream is8(input8);
	anyTest.Import(is8);
	assertDoublesEqual(doubleDefault, anyTest.AsDouble(doubleDefault), 0.00001);
	assertEqual(AnyNullType, anyTest.GetType());

	// test import of win32 3-digit exponent representation (printf %.e)
	String input9 = "1.0123456789E+020";
	IStringStream is9(input9);
	anyTest.Import(is9);
	assertDoublesEqual(1.0123456789E+20, anyTest.AsDouble(), 0.00001);
	assertEqual(AnyDoubleType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());
}

void AnythingParserTest::importEmptyStream() {
	String input = "";
	IStringStream is(input);
	Anything any0;
	any0.Import(is);
	assertEqual(0, any0.GetSize());
	assertEqual(AnyNullType, any0.GetType());
}

void AnythingParserTest::assertParsedAsDouble(const char *in, double val, int id)
/* in: str: a String which most probably contains a doubleAny
 val: intended value of the resulting double
 id: identifier for the string in the definition
 what: checks if the Anything parsed from str contains a double
 */
{
	Anything any;
	String str(in);
	IStringStream is(str);

	is >> any;
	assertEqual(AnyDoubleType, any.GetType());
	if (AnyDoubleType== any.GetType()) {
		assertDoublesEqual(any.AsDouble(), val, 1E-8);
	} else {
		assertEqual(-1, id);
		// crutch until we can add some info, identify whats wrong
	} // if

} // assertParsedAsDouble

void AnythingParserTest::assertParsedAsLong(const char *in, long val, int id)
/* in: str: a String which most probably contains a longAny
 val: intended value of the resulting long
 id: identifier for the string in the definition
 what: checks if the Anything parsed from str contains a long
 */
{
	Anything any;
	String str(in);
	IStringStream is(str);

	is >> any;
	assertEqual(AnyLongType, any.GetType());
	if (AnyLongType== any.GetType()) {
		assertEqual(val, any.AsLong());
	} else {
		assertEqual(-1, id);
		// crutch until we can add some info, identify whats wrong
	} // if

} // assertParsedAsDouble

void AnythingParserTest::assertParsedAsString(const char *in, int id)
/* in: str: a String which most probably contains a StringAny
 (also the value we expect)
 id: identifier for the string in the definition
 what: checks if the Anything parsed from in contains a String
 */
{
	Anything any;
	String str(in);
	IStringStream is(str);

	is >> any;
	assertEqual(AnyCharPtrType, any.GetType());
	if (AnyCharPtrType== any.GetType()) {
		assertEqual(in, any.AsCharPtr());
	} else {
		assertEqual(-1, id);
		// crutch until we can add some info, identify whats wrong
	} // if

} // assertParsedAsString

// convenient definitions for the following procedure
#define parseDoubleNum(val, id) \
	assertParsedAsDouble(_QUOTE_(val), val, id);

#define parseLongNum(val, id) \
	assertParsedAsLong(_QUOTE_(val), val, id);

#define parseString(val, id) \
	assertParsedAsString(_QUOTE_(val), id);

void AnythingParserTest::parseNumber() {
	parseDoubleNum(+23E+23, 1);
	parseDoubleNum(+432.E+12, 2);
	parseDoubleNum(+34.01e+12, 3);
	parseDoubleNum(+23E-23, 4);
	parseDoubleNum(+432.E-12, 5);
	parseDoubleNum(+34.02e-12, 6);
	parseDoubleNum(-23E-23, 8);
	parseDoubleNum(-432.E-12, 9);
	parseDoubleNum(-34.023e-12, 10);
	parseDoubleNum(-23E+23, 11);
	parseDoubleNum(-432.E+12, 12);
	parseDoubleNum(-34.045e+12, 13);
	parseDoubleNum(233e+23, 14);
	parseDoubleNum(321.e+21, 15);
	parseDoubleNum(456.5e+23, 16);
	parseDoubleNum(233e-23, 17);
	parseDoubleNum(321.e-21, 18);
	parseDoubleNum(456.5e-23, 19);
	parseDoubleNum(233e23, 20);
	parseDoubleNum(321.e21, 21);
	parseDoubleNum(456.5e23, 22);
	parseDoubleNum(456.5e23, 23);
	parseDoubleNum(456.5e23, 24);
	parseLongNum(+3213, 25);
	parseLongNum(+3213, 26);
	parseLongNum(+3213, 27);
	parseDoubleNum(+234347., 28);
	parseDoubleNum(+436784.344, 29);
	parseLongNum(-5354, 30);
	parseLongNum(-5354, 31);
	parseDoubleNum(-4324., 32);
	parseDoubleNum(-4324., 33);
	parseDoubleNum(-324.432, 34);
	parseLongNum(4324, 35);
	parseDoubleNum(54359., 36);
	parseDoubleNum(23.543, 37);
	parseDoubleNum(23.543, 38);
	parseString(21-23.54, 39);
	parseString(21.4354-21.2344, 40);
	parseString(+231.433-23432.3434, 41);
	parseDoubleNum(-132323E+23, 42);
	parseString(+23.234E-23Stop, 43);
	parseString(23.Stop, 44);
	parseString(21-23.54Stop, 45);
	parseString(21.4354+21.2344, 46);
	parseString(-132323EStop, 48);
	parseDoubleNum(123E4, 49);
	parseDoubleNum(0123E4, 50);
	parseDoubleNum(.0123, 51);
	parseDoubleNum(.0123E4, 52);
}

void AnythingParserTest::parseOctal() {
	String input[100];
	input[0] = "00";
	input[1] = "04";
	input[2] = "07";
	input[3] = "000";
	input[4] = "001";
	input[5] = "010";
	input[6] = "045";
	input[7] = "075";
	input[8] = "067";
	input[9] = "084";
	input[10] = "048";
	input[11] = "0000";
	input[12] = "0004";
	input[13] = "0050";
	input[14] = "0063";
	input[15] = "0300";
	input[16] = "0405";
	input[17] = "0450";
	input[18] = "0456";
	input[19] = "0458";
	input[20] = "0484";
	input[21] = "0845";
	input[22] = "00000";
	input[23] = "00003";
	input[24] = "00030";
	input[25] = "00042";
	input[26] = "00700";
	input[27] = "00560";
	input[28] = "00345";
	input[29] = "04000";
	input[30] = "04002";
	input[31] = "05060";
	input[32] = "04034";
	input[33] = "04200";
	input[34] = "06204";
	input[35] = "05250";
	input[36] = "02375";
	input[37] = "08";
	input[38] = "09";
	input[39] = "0-4";
	input[40] = "0-&4";
	input[41] = "04.45";
	input[42] = "04.3E+2";
	input[43] = "04.5e-3";
	input[44] = "04.";

	AnythingParserTest::writeResult(&input[0L], 45, "tmp/AnythingOctal", "res");
}

void AnythingParserTest::parseBinary() {
	String input[100];
	input[0] = "[;]";
	input[1] = "[]";
	input[2] = "[aa;578]";
	input[3] = "[A;B]";
	input[4] = "[3;234]";
	input[5] = "[0;546]";
	input[6] = "[3;123]";
	input[7] = "[ 3 ; 123 ]";
	input[8] = "[2;ftj]";
	input[9] = "[4;1230]";
	input[10] = "[5;-1.23]";
	input[11] = "[2;-1234]";
	input[12] = "[ 3 ;]";
	input[13] = "[+3;123]";
	input[14] = "[2;+1234]";
	input[15] = "[-2;1234]";
	input[16] = "[-2;-67]";
	input[17] = "[+3;+567]";
	input[18] = "[3.5;12345]";
	input[19] = "[-3.5;12345]";
	input[20] = "[3.5;12345]";
	input[21] = "[-3.5;-12345]";
	input[22] = "[00003;123]";
	input[23] = "[    3;123]";
	input[24] = "[3;]]";
	input[25] = "[3;]]]]";
	input[26] = "[3;]]]]]";
	input[27] = "[3]";
	input[28] = "[[3;123]";
	input[29] = "[3;   ]";
	input[30] = "[3;\0" "x10123]"; //lint !e840
	input[31] = "[3;\0" "x13123]"; //lint !e840
	input[32] = "[2364663;3]";
	input[33] = "[3;]]";

	AnythingParserTest::writeResult(&input[0L], 34, "tmp/AnythingBinary", "res");
}

void AnythingParserTest::parseSimpleTypeNull() {
	Anything anyTest;

	String input0 = "";
	IStringStream is0(input0);
	anyTest.Import(is0);						//ERROR: 0:0:Anything::Import :syntax error
	assertEqual(AnyNullType, anyTest.GetType());
	assertEqual(0, anyTest.GetSize());
	assertDoublesEqual(0.0, anyTest.AsDouble(), 0.00001);
	assertEqual(NULL, anyTest.AsCharPtr());
	//assertCharPtrEqual( anyTest.AsCharPtr(), " " );	// ABORT ????

	String input1 = "*";
	IStringStream is1(input1);
	anyTest.Import(is1);
	assertEqual(AnyNullType, anyTest.GetType());
	assertEqual(0, anyTest.GetSize());
	assertDoublesEqual(0.0, anyTest.AsDouble(), 0.00001);
	assertEqual(NULL, anyTest.AsCharPtr());

	// \01 ist nicht erlaubt --> eNull
	String input2 = "\01";
	IStringStream is2(input2);
	anyTest.Import(is2);
	assertEqual(AnyNullType, anyTest.GetType());
	assertEqual(0, anyTest.GetSize());
	assertDoublesEqual(0.0, anyTest.AsDouble(), 0.00001);
}

void AnythingParserTest::parseSimpleTypeCharPtr() {
	Anything anyTest;
	long i, j;
	char ch0, ch1;

	// Beginning with a number
	//
	// Theoretisch falsch ABER nicht problematisch:  ist es gew�nscht ????
	//
	String input0 = "1234567890L";
	IStringStream is0(input0);
	anyTest.Import(is0);
	assertEqual(AnyCharPtrType, anyTest.GetType());		// Because of the L at the end
	assertEqual(1, anyTest.GetSize());
	if (anyTest.AsCharPtr()) {
		assertCharPtrEqual(anyTest.AsCharPtr(), "1234567890L");
	}

	// Beginning with a char: OK
	String input1 = "aAbB01234";
	IStringStream is1(input1);
	anyTest.Import(is1);
	assertEqual(AnyCharPtrType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());
	if (anyTest.AsCharPtr()) {
		assertCharPtrEqual(anyTest.AsCharPtr(), "aAbB01234");
	}

	// Beginning with a '\"' (allowed buffer): OK
	String input2 = "\"aAbB01234\"";
	IStringStream is2(input2);
	anyTest.Import(is2);
	assertEqual(AnyCharPtrType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());
	if (anyTest.AsCharPtr()) {
		assertCharPtrEqual(anyTest.AsCharPtr(), "aAbB01234");
	}

	// Beginning with a '"' (not allowed buffer:  the not allowed part is simply cut off)
	//
	// Ist es gew�nscht????
	String input3 = "\"aAbB01\"234\"";
	IStringStream is3(input3);
	anyTest.Import(is3);
	assertEqual(AnyCharPtrType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());
	if (anyTest.AsCharPtr()) {
		assertCharPtrEqual(anyTest.AsCharPtr(), "aAbB01");
	}

	// falsche Syntax:  alle char ohne Esc-Seq.
	// Fehler-Meldung:   .any:2 syntax error: invalid string constant (missing quote)
	// Inhalt undef
	//
	String input4;
	input4.Append("\"");
	for (i = 1; i < 256; i++) {
		input4.Append((char) i);
	}
	input4.Append("\"");
	IStringStream is4(input4);
	anyTest.Import(is4);
	assertEqual(AnyCharPtrType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());
	// korrekte Syntax (alle char mit Esc-Seq.)
	String input5;
	input5.Append("\"");
	for (i = 0; i < 16; i++) {
		ch0 = i + '0';
		if (i > 9) {
			ch0 = 'A' + i - 10;
		}

		for (j = 0; j < 16; j++) {
			ch1 = j + '0';
			if (j > 9) {
				ch1 = 'A' + j - 10;
			}
			if ((i != 0) || (j != 0)) {
				input5.Append('\\');
				input5.Append('x');
				input5.Append(ch0);
				input5.Append(ch1);
			}
		}
	}
	input5.Append("\"");
	IStringStream is5(input5);
	anyTest.Import(is5);
	assertEqual(AnyCharPtrType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());
	for (i = 1; i < 256; i++) {
		assertEqual((char )i, anyTest.AsCharPtr()[i - 1]);
	}

	// korrekte Syntax (printables: ohne Esc-Seq;  non-printable: mit Esc-Seq)
	String input6;
	input6.Append("\"");
	for (i = 0; i < 16; i++) {
		ch0 = i + '0';
		if (i > 9) {
			ch0 = 'A' + i - 10;
		}

		for (j = 0; j < 16; j++) {
			ch1 = j + '0';
			if (j > 9) {
				ch1 = 'A' + j - 10;
			}

			if ((i != 0) || (j != 0)) {
				if (isprint((unsigned char) i * 16 + j)) {
					if ((char) (i * 16 + j) == '"') {
						input6.Append('\\');   //   \" sonst wird " als EndOfString interpretiert
					}
					input6.Append((char) (i * 16 + j));
				} else {
					input6.Append('\\');
					input6.Append('x');
					input6.Append(ch0);
					input6.Append(ch1);
				}
			}
		}
	}
	input6.Append("\"");
	IStringStream is6(input6);
	anyTest.Import(is6);
	assertEqual(AnyCharPtrType, anyTest.GetType());
	assertEqual(1, anyTest.GetSize());

	for (i = 1; i < 256; i++) {
		assertEqual((unsigned char )i, (unsigned char )(anyTest.AsCharPtr()[i - 1]));
	}

	// Falsche Syntax:  Beginning with a printable char without '\"'
	// Syntasch error bemerkt --> NullAnything
	String input7 = "!abc";
	IStringStream is7(input7);
	anyTest.Import(is7);
	assertEqual(AnyNullType, anyTest.GetType());
	assertEqual(0, anyTest.GetSize());
	if (anyTest.AsCharPtr()) {
		assertCharPtrEqual(anyTest.AsCharPtr(), "!abc");
	}
	assertEqual(AnyNullType, anyTest.GetType());
	assertEqual(0, anyTest.GetSize());
}

/*==================================================================================*/
/*                    Tests von geschachteten Anythings                             */
/*==================================================================================*/
void AnythingParserTest::parseMixedAnything() {
	Anything anyF, anyE, anyD, anyC, anyCnew, anyB, anyBnew, anyApart, anyA, anyAnew;

	anyE.Append(1);
	anyE.Append(2);
	anyD["E"] = anyE;
	anyC["D"] = anyD;
	anyF = 1e34;
	anyC["F"] = anyF;
	anyCnew["C"] = anyC;
	anyB.Append("ABC");
	anyB.Append("BCD");
	anyB.Append("CDE");
	anyB.Append(anyCnew);
	anyBnew["B"] = anyB;
	anyApart.Append(0);
	anyApart.Append(1);
	anyApart.Append(2);
	anyApart.Append(3);
	anyApart.Append(4);
	anyApart.Append(anyBnew);
	anyA.Append(123);
	anyA.Append("123L");
	anyA.Append("String");
	anyA.Append(anyApart);
	anyAnew["A"] = anyA;
	anyAnew["G"] = 123;
	anyAnew["H"] = 1234;
	anyAnew["I"] = "String";

	std::ostream *osHlp = system::OpenOStream("tmp/AnyTest", "txt", std::ios::trunc);
	if (osHlp) {
		*osHlp << anyAnew;
		delete osHlp;
	} else {
		assertEqual("'write tmp/AnyTest.txt'", "'could not write tmp/AnyTest.txt'");
	}

	std::istream *isHlp = system::OpenIStream("tmp/AnyTest", "txt");
	if (!isHlp) {
		assertEqual("'read tmp/AnyTest.txt'", "'could not read tmp/AnyTest.txt'");
	} else {	// if ( isHlp )
		Anything anyHlp;
		anyHlp.Import(*isHlp);
		delete isHlp;

		std::ostream *osNew = system::OpenOStream("tmp/AnyTestNew", "txt", std::ios::trunc);
		if (osNew) {
			*osNew << anyHlp;
			delete osNew;
		} else {
			assertEqual("'write tmp/AnyTestNew.txt'", "'could not write tmp/AnyTestNew.txt'");
		}

		anyHlp["A"].SlotName(0L);
		assertEqual(AnyLongType, anyHlp["A"].At(0L).GetType());
		assertEqual(1, anyHlp["A"].At(0L).GetSize());
		assertEqual(123, anyHlp["A"].At(0L).AsLong());

		anyHlp["A"].SlotName(1L);
		assertEqual(AnyCharPtrType, anyHlp["A"].At(1L).GetType());
		assertEqual(1, anyHlp["A"].At(1L).GetSize());
		if (anyHlp["A"].At(1L).AsCharPtr()) {
			assertCharPtrEqual(anyHlp["A"].At(1L).AsCharPtr(), "123L");
		}

		anyHlp["A"].SlotName(2L);
		assertEqual(AnyCharPtrType, anyHlp["A"].At(2L).GetType());
		assertEqual(1, anyHlp["A"].At(2L).GetSize());
		if (anyHlp["A"].At(2L).AsCharPtr()) {
			assertCharPtrEqual(anyHlp["A"].At(2L).AsCharPtr(), "String");
		}

		//anyHlp["A"].At[3].SlotName(0L);
		assertEqual(AnyLongType, anyHlp["A"].At(3L).At(0L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(0L).GetSize());
		assertEqual(0, anyHlp["A"].At(3L).At(0L).AsLong());

		//anyHlp["A"].At[3].SlotName(1L);
		assertEqual(AnyLongType, anyHlp["A"].At(3L).At(1L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(1L).GetSize());
		assertEqual(1, anyHlp["A"].At(3L).At(1L).AsLong());

		//anyHlp["A"].At[3].SlotName(2L);
		assertEqual(AnyLongType, anyHlp["A"].At(3L).At(2L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(2L).GetSize());
		assertEqual(2, anyHlp["A"].At(3L).At(2L).AsLong());

		//anyHlp["A"].At[3].SlotName(3L);
		assertEqual(AnyLongType, anyHlp["A"].At(3L).At(3L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(3L).GetSize());
		assertEqual(3, anyHlp["A"].At(3L).At(3L).AsLong());

		//anyHlp["A"].At[3].SlotName(4L);
		assertEqual(AnyLongType, anyHlp["A"].At(3L).At(4L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(4L).GetSize());
		assertEqual(4, anyHlp["A"].At(3L).At(4L).AsLong());

		anyHlp["A"].At(3L).At(5L)["B"].SlotName(0L);
		assertEqual(AnyCharPtrType, anyHlp["A"].At(3L).At(5L)["B"].At(0L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(5L)["B"].At(0L).GetSize());
		if (anyHlp["A"].At(3L).At(5L)["B"].At(0L).AsCharPtr()) {
			assertCharPtrEqual(anyHlp["A"].At(3L).At(5L)["B"].At(0L).AsCharPtr(), "ABC");
		}

		anyHlp["A"].At(3L).At(5L)["B"].SlotName(1L);
		assertEqual(AnyCharPtrType, anyHlp["A"].At(3L).At(5L)["B"].At(1L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(5L)["B"].At(1L).GetSize());
		if (anyHlp["A"].At(3L).At(5L)["B"].At(1L).AsCharPtr()) {
			assertCharPtrEqual(anyHlp["A"].At(3L).At(5L)["B"].At(1L).AsCharPtr(), "BCD");
		}

		anyHlp["A"].At(3L).At(5L)["B"].SlotName(2L);
		assertEqual(AnyCharPtrType, anyHlp["A"].At(3L).At(5L)["B"].At(2L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(5L)["B"].At(2L).GetSize());
		if (anyHlp["A"].At(3L).At(5L)["B"].At(2L).AsCharPtr()) {
			assertCharPtrEqual(anyHlp["A"].At(3L).At(5L)["B"].At(2L).AsCharPtr(), "CDE");
		}

		anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].SlotName(0L);
		assertEqual(AnyLongType, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(0L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(0L).GetSize());
		assertEqual(1, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(0L).AsLong());

		anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].SlotName(1L);
		assertEqual(AnyLongType, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(1L).GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(1L).GetSize());
		assertEqual(2, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(1L).AsLong());

		assertEqual(AnyDoubleType, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["F"].GetType());
		assertEqual(1, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["F"].GetSize());
		assertDoublesEqual(1e34, anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["F"].AsDouble(), 0.00001);

		assertEqual(AnyLongType, anyHlp["G"].GetType());
		assertEqual(1, anyHlp["G"].GetSize());
		assertEqual(123, anyHlp["G"].AsLong());

		assertEqual(AnyLongType, anyHlp["H"].GetType());
		assertEqual(1, anyHlp["H"].GetSize());
		assertEqual(1234, anyHlp["H"].AsLong());
		if (anyHlp["I"].AsCharPtr()) {
			assertCharPtrEqual(anyHlp["I"].AsCharPtr(), "String");
		}
	}
}

/*==================================================================================*/
/*                    Tests von Imports+Exports von Anythings                       */
/*==================================================================================*/
void AnythingParserTest::readWriteAnything() {
	Anything anyTest, anyTestNew;
	String input[200];
	input[0] = "{ \"\01\" }";
	input[1] = "{ \"\\01\" }";
	input[2] = "{ \"\\\\\" }";
	input[3] = "{ \"\\\\x41\" }";
	input[4] = "{ /\"{}\"  1 }";
	input[5] = "{ /\"\"{}\"\"  1 }";
	input[6] = "{ /a {} }";
	input[7] = "{ /b \"{}\" }";
	input[8] = "{ /c \"\"{}\"\" }";
	input[9] = "{ /#ab  0 }";
	input[10] = "{ /a#b  1 }";
	input[11] = "{ /ab#  2 }";
	input[12] = "{ /\"#ab\"  3 }";
	input[13] = "{ /\"a#b\"  4 }";
	input[14] = "{ /\"ab#\"  5 }";
	input[15] = "{ /}ab  6 }";
	input[16] = "{ /a}b  7 }";
	input[17] = "{ /ab}  8 }";
	input[18] = "{ /\"}ab\"  9 }";
	input[19] = "{ /\"a}b\"  10 }";
	input[20] = "{ /\"ab}\"  11 }";
	input[21] = "{ /{ab  12 }";
	input[22] = "{ /a{b  13 }";
	input[23] = "{ /ab{  14 }";
	input[24] = "{ /\"{ab\"  15 }";
	input[25] = "{ /\"a{b\"  16 }";
	input[26] = "{ /\"ab{\"  17 }";
	input[27] = "{ /{ab  1}8 }";
	input[28] = "{ /a{b  1}9 }";
	input[29] = "{ /ab{  2}0 }";
	input[30] = "{ /\"{ab\"  2}1 }";
	input[31] = "{ /\"a{b\"  2}2 }";
	input[32] = "{ /\"ab{\"  2}3 }";
	input[33] = "{ /{ab  \"2}4\" }";
	input[34] = "{ /a{b  \"2}5\" }";
	input[35] = "{ /ab{  \"2}6\" }}";
	input[36] = "{ /\"{ab\"  \"2}7\" }";
	input[37] = "{ /\"a{b\"  \"2}8\" }";
	input[38] = "{ /\"ab{\"  \"2}9\" }";
	input[39] = "{ /Slot\"Name  30 }";
	input[40] = "{ /Slot\\\"Name  31 }";
	input[41] = "{ /\"Slot\"Name\"  32 }";
	input[42] = "{ /\"Slot\\\"Name\"  33}";
	input[43] = "{ /Slot\x22Name  34 }";		// x22 = "
	input[44] = "{ /Slot\\\x22Name  35 }";
	input[45] = "{ /\"Slot\x22Name\"  36 }";
	input[46] = "{ /\"Slot\\\x22Name\"  37}";
	input[47] = "{ \"/Slot\"Name\" 38 }";
	input[48] = "{ \"/Slot\\\"Name\" 39 }";
	input[49] = "{ /Slot\"Name  40 }";
	input[50] = "{ /Slot\x22Name  41 }";
	input[51] = "{ //ab  6 }";
	input[52] = "{ /a/b  7 }";
	input[53] = "{ /ab/  8 }";
	input[54] = "{ /\"/ab\"  9 }";
	input[55] = "{ /\"a/b\"  10 }";
	input[56] = "{ /\"ab/\"  11 }";
	input[57] = "{ //ab  1/8 }";
	input[58] = "{ /a/b  1/9 }";
	input[59] = "{ /ab/  a/0 }";
	input[60] = "{ /\"/ab\"  2/1 }";
	input[61] = "{ /\"a/b\"  2/2 }";
	input[62] = "{ /\"ab/\"  2/3 }";
	input[63] = "{ //ab  \"2/4\" }";
	input[64] = "{ /a/b  \"2/5\" }";
	input[65] = "{ /ab/  \"2/6\" }";
	input[66] = "{ /\"/ab\"  \"2/7\" }";
	input[67] = "{ /\"a/b\"  \"2/8\" }";
	input[68] = "{ /\"ab/\"  \"2/9\" }";
	//input[69] = "{ /\x   \"a\" }";
	input[69] = "{ /\\x   \"a\" }";
	input[70] = "{ /\x3   \"b\" }";
	input[71] = "{ /\\x3   \"c\" }";
	input[72] = "{ /\x30   \"d\" }";
	input[73] = "{ /\\x30   \"e\" }";
	//input[72] = "{ /\"\x\"  \"d\" }";
	input[74] = "{ /\"\\x\"  \"f\" }";
	input[75] = "{ /\"\x3\"  \"g\" }";
	input[76] = "{ /\"\\x3\"  \"h\" }";
	input[77] = "{ /\"\x30\"  \"i\" }";
	input[78] = "{ /\"\\x30\"  \"l\" }";
	input[79] = "{ /\x22   \"m\" }";
	input[80] = "{ /\"\x22\"  \"n\" }";
	input[81] = "{ /\x22   \"}\" }";
	input[82] = "{ /\"\x22\"  \"}\" }";
	input[83] = "{ /a\x22   \"}\" }";
	input[84] = "{ /\"a\x22\"  \"}\" }";
	input[85] = "{ /a{   \"}\" }";
	input[86] = "{ /a{   } }";
	input[87] = "{ /\"a{\"   \"}\" }";
	input[88] = "{ /\"a{\"   } }";
	input[89] = "{ /\"}\" \"a\" }";
	input[90] = "{ /\"Slo{t}Name\"        \"a\\\"{}c\" }";
	input[91] = "{ /\"Slot}Name\"        \"a\\\"{}c\" }";
	input[92] = "{";
	input[93] = "{ /Test1 a";
	input[94] = "{ /Test1";
	input[95] = "{ a";
	input[96] = "{ /A { /B 12";
	input[97] = "{ /A { /B 12 }";
	input[98] = "{ /Test2";
	input[99] = "{ {";
	input[100] = "{ /";
	input[101] = "{{{}";
	input[102] = "}";
	input[103] = "{{}";
	input[104] = " a }";
	input[105] = "/Test156 a}";
	input[106] = "{ * /Test155 {a} }";
	input[107] = "{ ***** }";
	input[108] = "{	/* a }";
	input[109] = "{	/ /	}";
	input[110] = "{ // a }";
	input[111] = "{	/ / a }";
	input[112] = "{	/ Test154 a }";
	input[113] = "{/ a }";
	input[114] = "{ () }";
	input[115] = "{ Test149 }";
	input[116] = "{	/Test148 }";
	input[117] = "*";
	input[118] = "{}";
	input[119] = "{ /Test147 { /Test147Dup a } }";
	input[120] = "{ /Test146	a } }";
	input[121] = "/\"Test146Dup Size\"a\" ";
	input[122] = "{	/Test145 { } }";
	input[123] = "{	/Test144 {} }";
	input[124] = "{	/Test143 {{{{{}}}}}}";
	input[125] = "{	/Test153 \" /Test153Dup a }";
	input[126] = "{ /Test152 \"\"\"\"\"\"\"  /Test152Dup	a }";
	input[127] = "{	/Test151 12\"abc }";
	input[128] = "{	/Test150 \"abc\"dfg }";
	input[129] = "\"";
	input[130] = "{0{1{2{3{4{5{6{7{8{9{10{11{12{13{14{15{16{17{18{19{20{21{22{23{24{25{26{27{28{29}}}}}}}}}}}}}}}}}}}}}}}}}}}}}}";
//	input[131] = "{ \"\x01\x02\x03\x04\x05\x06\x07\x08\x09\x0A\x0B\x0C\x0D\x0E\x10\x1A\x20\x2A\x70\x77\x7A\x80\x87\x88\x90\x98\x9A\x88\x89\xFF\" }";
	input[131] = "{ \"This one above causes some trouble\" }";
	input[132] =
			"{ \"\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\x09\\x0A\\x0B\\x0C\\x0D\\x0E\\x10\\x1A\\x20\\x2A\\x70\\x77\\x7A\\x80\\x87\\x88\\x90\\x98\" }";
	input[133] = "{ \"\\01\\02\\03\\04\\05\\06\\07\\08\\09\\0A\\0B\\0C\\0D\\0E\\10\\1A\\20\\2A\\70\\77\\7A\\80\\87\\88\\90\\98\\9A\\88\\89\\FF\" }";
	input[134] =
			"{ \\01 \\02 \\07 \\08 \\09 \\0A \\0B \\0C \\0D \\0E \\10 \\1A \\20 \\2A \\70 \\77 \\7A \\80 \\87 \\88 \\90 \\98 \\9A \\88 \\89 \\FF }";
	input[135] =
			"{ \"\\01 \\02 \\07 \\08 \\09 \\0A \\0B \\0C \\0D \\0E \\10 \\1A \\20 \\2A \\70 \\77 \\7A \\80 \\87 \\88 \\90 \\98 \\9A \\88 \\89 \\FF\" }";
	input[136] = "{ \"\01\" }";
	input[137] = "{ \"\\01\" }";
	input[138] = "{ \"\\String\" }";
	input[139] = "{ \"\\\\\" }";
	input[140] = "{a}";
	input[141] = "{/A a}";
	input[142] = "a";
	input[143] = "{\"\\xGH\"}";
	input[144] = "{\"\\GH\"}";
	input[145] = "{/Slot\x00Name Wert}";
	input[146] = "{/SlotName We\x00rt}";
	input[147] = "{/Slot\x00Name We\x00rt}";
	input[148] = "{/SlotName0 \"abc\" /SlotName1 \"cd\x00e\"}";
	input[149] = "{/SlotName0 \"abc\" /SlotName1 \"cd\\x00e\"}";
	input[150] = "{/SlotName0 \"abc\" /SlotName1 \"cde\"}";
	input[151] = "{/SlotName0 1  /SlotName0 2 }";
	input[152] = "{	/SlotName0 0 #Kommentar 0\
				/SlotName1 1 #Kommentar 1\
				}";
	input[153] = "{	/SlotName0 0 #Kommentar 0\
				/SlotName1 1 #Kommentar 1 }";
	input[154] = "{	/SlotName0 0 #Kommentar 0   /SlotName1 1 #Kommentar 1 }";
	input[155] = " # { /SlotName0 0 }";
	input[156] = "{ #Kommentar /SlotName0 0 }";
	input[157] = "{ /SlotName0  #Kommentar  0 }";
	input[158] = "{ /SlotName0 #Kommentar\
				0\
				/SlotName1 1 }";
	input[159] = "{ /SlotName0 0 #Kommentar }";
	input[160] =
			"{ /SlotName0 0 # /A {} ######}\
				# \"a\" ###########}\
				## 1.2 ##########}\
				### [3;123] #########}\
				/SlotName1  1\
				#####\
				#######\
}";
	input[161] = "{ /SlotName0  \" # Kein Kommentar \" }";
	input[162] = "{ /SlotName0  Kein\\#Kommentar }";
	input[163] = "{ /SlotName0  Kein\x23Kommentar }";		// \x23 == #
	input[164] = "{ /SlotName0  Kein\\x23Kommentar }";
	input[165] = "{/\"Hex\\xGHI\" \"\\xghi\\xhal\\xfgh\"}";
	input[166] = "{/\"\\xABG\" \"\\x323\\xFF3\\xFGH\xABG\"}";

	long i;
	for (i = 0; i < 167; i++)

	{
		IStringStream is(input[i]);
		anyTest.Import(is);

		// Ueberpruefe, ob Imports+Exports funktionnieren
		String fileName;
		fileName.Append("readWriteAnything at index ");
		fileName.Append(i);
		AnythingParserTest::checkImportExport(anyTest, fileName);
	}
}

/*==================================================================================*/
/*                               Miscellaneous                                      */
/*==================================================================================*/
void AnythingParserTest::parseTestFiles() {
	// Parse the contents of a Testfile
	//------------------------------------------------------------------
	Anything anyEmpty;
	String fileNames[30];

	fileNames[0] = String("AnythingString");
	fileNames[1] = String("AnythingMixOK");
	fileNames[2] = String("AnythingMixKO1");
	fileNames[3] = String("AnythingMixKO2");
	fileNames[4] = String("AnythingTest0");
	fileNames[5] = String("AnythingTest1");
	fileNames[6] = String("AnythingTest2");
	fileNames[7] = String("AnythingTest3");
	fileNames[8] = String("AnythingTest4");
	fileNames[9] = String("AnythingTest5");
	fileNames[10] = String("AnythingTest6");
	fileNames[11] = String("AnythingTest7");
	fileNames[12] = String("AnythingTest8");
	fileNames[13] = String("AnythingTest9");
	fileNames[14] = String("AnythingTest10");
	fileNames[15] = String("AnythingTest11");
	fileNames[16] = String("AnythingTest12");
	fileNames[17] = String("AnythingTest13");
	fileNames[18] = String("AnythingTest14");
	fileNames[19] = String("AnythingTest15");
	fileNames[20] = String("AnythingTest16");

	long testCnt;
	Anything emptyAny1, anyHlp, anyTest, anyTests;

	for (testCnt = 0; testCnt < 21; testCnt++) {
		std::istream *is = system::OpenStream((const char *) fileNames[testCnt], "txt");
		if (is) {
			// Lese die Anything-Datei ein
			Anything any0;
			any0.Import(*is);
			delete is;
			// Untersuche die Semantik des eingelesenen Anything-Objektes
			AnythingParserTest::anyOutput = emptyAny1;
			AnythingParserTest::lineCounter = 1;
			scanAnything(any0);
			// Schreibe das Ergebnis in eine Datei
			String tmp = "tmp/";
			tmp.Append(fileNames[testCnt]);
			std::ostream *os = system::OpenOStream((const char *) tmp, "res", std::ios::trunc);
			if (os) {
				*os << AnythingParserTest::anyOutput;
				delete os;
				AnythingParserTest::anyOutput = emptyAny1;
				AnythingParserTest::lineCounter = 1;
				// Ueberpruefe, ob Imports+Exports funktionnieren
				AnythingParserTest::checkImportExport(any0, tmp); //fileNames[testCnt] );
			} else {
				String tmp0 = "write ";
				tmp0.Append(tmp);
				tmp0.Append(".res");
				String tmp1 = "could not write ";
				tmp1.Append(tmp);
				tmp1.Append(".res");
				assertEqual((const char * )tmp0, (const char * )tmp1);
			}
		} else {
			String tmp0 = "read ";
			tmp0.Append(fileNames[testCnt]);
			tmp0.Append(".txt");
			String tmp1 = "could not read ";
			tmp1.Append(fileNames[testCnt]);
			tmp1.Append(".txt");
			assertEqual((const char * )tmp0, (const char * )tmp1);
		}
	}
}

class myObject: public IFAObject {
public:
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return (IFAObject*) this;
	}
};

void AnythingParserTest::testObjectParsing() {
	{
		String buf("{/myObjectImpl &12345}");
		IStringStream is(buf);
		Anything any;
		t_assert(any.Import(is));
		assertEqual("myObjectImpl", any.SlotName(0));
		assertCompare(12345L, equal_to, reinterpret_cast<long>(any["myObjectImpl"].AsIFAObject(0)));
	}
	{
		String buf("{/myObjectImpl &-12345}");
		IStringStream is(buf);
		Anything any;
		t_assert(any.Import(is));
		assertEqual("myObjectImpl", any.SlotName(0));
		assertCompare(-12345L, equal_to, reinterpret_cast<long>(any["myObjectImpl"].AsIFAObject(0)));
	}
	{
		String buf("{/myObjectImpl &4294967295}");
		IStringStream is(buf);
		Anything any;
		t_assert(any.Import(is));
		assertEqual("myObjectImpl", any.SlotName(0));
		assertCompare(4294967295UL, equal_to, reinterpret_cast<unsigned long>(any["myObjectImpl"].AsIFAObject(0)));
	}
	myObject aObj;
	{
		String buf("{/myObjectImpl &");
		buf.Append((long) &aObj); //lint !e603
		buf.Append("}");
		IStringStream is(buf);
		Anything any;
		t_assert(any.Import(is));
		assertEqual("myObjectImpl", any.SlotName(0));
		assertCompare((IFAObject * )&aObj, equal_to, any["myObjectImpl"].AsIFAObject(0));
	}
	{
		String buf("{/myObjectImpl &");
		buf.Append((long) &aObj);
		buf.Append("/some content}");
		IStringStream is(buf);
		Anything any;
		t_assert(any.Import(is));
		assertCharPtrEqual("myObjectImpl", any.SlotName(0));
		assertCompare((IFAObject * )&aObj, equal_to, any["myObjectImpl"].AsIFAObject(0));
		assertCharPtrEqual("some", any.SlotName(1));
		assertEqual("content", any["some"].AsCharPtr());
	}
}

void AnythingParserTest::testNumberParsing() {
	{
		String buf("{/myLong 12345}");
		IStringStream is(buf);
		Anything any;
		t_assert(any.Import(is));
		assertCharPtrEqual("myLong", any.SlotName(0));
		assertCompare(12345L, equal_to, any["myLong"].AsLong(0));
	}
	{
		String buf("{/myLong 12345/some content}");
		IStringStream is(buf);
		Anything any;
		t_assert(any.Import(is));
		assertCharPtrEqual("myLong", any.SlotName(0));
		assertCompare(12345L, equal_to, any["myLong"].AsLong(0));
		assertCharPtrEqual("some", any.SlotName(1));
		assertEqual("content", any["some"].AsCharPtr());
	}
}
