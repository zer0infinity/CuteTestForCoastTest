/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "ParserTest.h"
//--- c-library modules used ---------------------------------------------------
#include <ctype.h>

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "System.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//---- ParserTest ---------------------------------------------------------
ParserTest::ParserTest (TString tname) : TestCase(tname)
{
}

void ParserTest::setUp ()
{
	Anything undefAny;
	emptyAny = undefAny;
	anyTemp0 = undefAny;
	anyTemp1 = undefAny;
	anyTemp2 = undefAny;
	anyTemp3 = undefAny;
	anyTemp4 = undefAny;
}

Test *ParserTest::worksuite ()
{
	TestSuite *testSuite = new TestSuite;
	return testSuite;
}

Test *ParserTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ParserTest, parseSimpleTypeCharPtr );
	ADD_CASE(testSuite, ParserTest, parseSimpleTypeDouble );
	ADD_CASE(testSuite, ParserTest, parseSimpleTypeLong );
	ADD_CASE(testSuite, ParserTest, parseSimpleTypeNull );
	ADD_CASE(testSuite, ParserTest, parseNumber );
	ADD_CASE(testSuite, ParserTest, parseOctal );
	ADD_CASE(testSuite, ParserTest, parseBinary );
	ADD_CASE(testSuite, ParserTest, parseMixedAnything );
	ADD_CASE(testSuite, ParserTest, importEmptyStream );
	ADD_CASE(testSuite, ParserTest, readWriteAnything );
	ADD_CASE(testSuite, ParserTest, parseTestFiles );
	ADD_CASE(testSuite, ParserTest, testSemantic0 );
	ADD_CASE(testSuite, ParserTest, testSemantic1 );
	ADD_CASE(testSuite, ParserTest, testSemantic2 );
	ADD_CASE(testSuite, ParserTest, testSemantic3 );
	ADD_CASE(testSuite, ParserTest, testSemantic4 );
	ADD_CASE(testSuite, ParserTest, testSemantic5 );
	ADD_CASE(testSuite, ParserTest, testSemantic6 );
	ADD_CASE(testSuite, ParserTest, testSemantic7 );
	ADD_CASE(testSuite, ParserTest, testSemantic8 );
	ADD_CASE(testSuite, ParserTest, testSemantic9 );

	ADD_CASE(testSuite, ParserTest, testSemantic10 );
	ADD_CASE(testSuite, ParserTest, testSemantic11 );
	ADD_CASE(testSuite, ParserTest, testSemantic12 );
	ADD_CASE(testSuite, ParserTest, testSemantic13 );
	ADD_CASE(testSuite, ParserTest, testSemantic14 );
	ADD_CASE(testSuite, ParserTest, testSemantic15 );
	ADD_CASE(testSuite, ParserTest, testSemantic16 );
	ADD_CASE(testSuite, ParserTest, testSemantic17 );
	ADD_CASE(testSuite, ParserTest, testSemantic18 );
	ADD_CASE(testSuite, ParserTest, testSemantic19 );

	ADD_CASE(testSuite, ParserTest, testSemantic20 );
	ADD_CASE(testSuite, ParserTest, testSemantic21 );
	ADD_CASE(testSuite, ParserTest, testSemantic22 );
	ADD_CASE(testSuite, ParserTest, testSemantic23 );
	ADD_CASE(testSuite, ParserTest, testSemantic24 );
	ADD_CASE(testSuite, ParserTest, testSemantic25 );
	ADD_CASE(testSuite, ParserTest, testSemantic26 );
	ADD_CASE(testSuite, ParserTest, testSemantic27 );
	ADD_CASE(testSuite, ParserTest, testSemantic28 );
	ADD_CASE(testSuite, ParserTest, testSemantic29 );

	ADD_CASE(testSuite, ParserTest, testSemantic30 );
	ADD_CASE(testSuite, ParserTest, testSemantic31 );
	ADD_CASE(testSuite, ParserTest, testSemantic32 );
	ADD_CASE(testSuite, ParserTest, testSemantic33 );

	ADD_CASE(testSuite, ParserTest, testQuotedSlotname ); // PS

	ADD_CASE(testSuite, ParserTest, slashSlotnames );

	return testSuite;
}

/*==================================================================================*/
/*                         Hilfsmethoden fuer Tests                                 */
/*==================================================================================*/
void ParserTest::checkImportExport( Anything any, String fileName )
{
	String buf1;
	String buf2;
	Anything anyHlp;

	OStringStream os1( &buf1 );
	any.Export( os1 );
	IStringStream is1( buf1 );
	anyHlp.Import( is1 );
	OStringStream os2( &buf2 );
	anyHlp.Export( os2 );
	assertCharPtrEqual( buf1, buf2 );
	if ( buf1 != buf2 ) {
	}
}

void ParserTest::writeResult( String *input , long nrOfElt, char *path, char *ext )
{
	ostream		*os = System::OpenOStream( path, ext, ios::trunc);
	if ( os ) {
		Anything emptyAny1, anyTest, anyTests;

		ParserTest::anyOutput = emptyAny1;
		ParserTest::lineCounter = 1;

		long i;
		for ( i = 0; i < nrOfElt; i++ ) {
			IStringStream is(input[i]);
			anyTest.Import(is);
			anyTests.Append( anyTest );
		}
		scanAnything( anyTests );

		*os << ParserTest::anyOutput;
		delete os;

		ParserTest::anyOutput = emptyAny1;
		ParserTest::lineCounter = 1;
	} else {
		String tmp0 = "write ";
		tmp0.Append( path );
		tmp0.Append( "." );
		tmp0.Append( ext );
		String tmp1 = "could not write ";
		tmp1.Append( path );
		tmp1.Append( "." );
		tmp1.Append( ext );
		assertEqual( (const char *)tmp0, (const char *)tmp1 );
	}
}

void  ParserTest::scanAnything( Anything any0 )
{
	long		i, iMax = any0.GetSize();
	String		slotNm;
	Anything	anyTest;
	String buffer;

	for ( i = 0; i < iMax; i++ ) {
		anyTest = any0[i];

		slotNm = "";
		slotNm.Append( ParserTest::lineCounter++ );
		slotNm.Append( ") " );
		slotNm.Append( any0.SlotName(i) );
		slotNm.Append( " Size:" );
		slotNm.Append( anyTest.GetSize() );
		slotNm.Append( "; Type:" );

		switch ( anyTest.GetType() ) {
			case AnyLongType: {
				slotNm.Append( "eLong" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsLong();
			}
			break;

			case AnyDoubleType: {
				slotNm.Append( "eDouble" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsDouble();
			}
			break;

			case AnyNullType: {
				slotNm.Append( "eNull" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = "NULL";
			}
			break;

			case AnyCharPtrType: {
				slotNm.Append( "eCharPtr" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsCharPtr();
			}
			break;

			case AnyArrayType: {
				slotNm.Append( "eArray" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = "...";
				ParserTest::scanAnything( anyTest );
			}
			break;

			case AnyVoidBufType: {
				slotNm.Append( "eVoidBuf" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = anyTest.AsCharPtr();
			}
			break;

			case AnyObjectType: {
				slotNm.Append( "eObject" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = anyTest;
			}
			break;

			default: {
				String str("???");
				slotNm.Append( "???" );
				slotNm.Append( "; Value: " );
				ParserTest::anyOutput[ (const char *)slotNm ] = str;//"???";//String("???");
			}
			break;
		}
	}
}

Anything ParserTest::storeAndReload( Anything any )
{
	// Store and reload 'any'
	String buf;
	OStringStream os( &buf );
	any.Export( os );
	IStringStream is( buf );
	Anything anyResult;
	anyResult.Import( is );
	return( anyResult );
}

/*==================================================================================*/
/*                         Tests von SimpleAny                                      */
/*==================================================================================*/
void ParserTest::IntParseSimpleTypeLong(const String &inp, long exp)
{
	Anything anyTest;

	IStringStream is0(inp);
	anyTest.Import( is0 );
	t_assert( anyTest.GetType() == AnyLongType );
	assertEqual(exp, anyTest.AsLong());
	assertEqual(1,  anyTest.GetSize());
}

void ParserTest::parseSimpleTypeLong ()
{
	Anything anyTest;

	IntParseSimpleTypeLong("0", 0);
	IntParseSimpleTypeLong("1234567890", 	1234567890);
	IntParseSimpleTypeLong("-1234567890", 	-1234567890);
	IntParseSimpleTypeLong("783051076", 	783051076L);

	// test unsigned semantic for hex and oct
	IntParseSimpleTypeLong("0x2eac6944",	(long)783051076UL);
	IntParseSimpleTypeLong("05653064504", 	(long)783051076UL);
	IntParseSimpleTypeLong("0xcffe007f", 	(long)3489529983UL);
	IntParseSimpleTypeLong("031777400177", 	(long)3489529983UL);

	String input4 		= "9999999999999";
	IStringStream is4(input4);
	anyTest.Import( is4 ); // mustn't crash
	//t_assert( anyTest.AsLong() == 1316134911 ); // bad test result platform dependant
	t_assert( anyTest.GetType() == AnyLongType );
	t_assert( anyTest.GetSize() == 1 );

	String input5 		= "-9999999999999";
	IStringStream is5(input5);
	anyTest.Import( is5 );// mustn't crash
	//t_assert( anyTest.AsLong() == -1316134911 );// bad test result platform dependant
	t_assert( anyTest.GetType() == AnyLongType );
	t_assert( anyTest.GetSize() == 1 );

	String inputx = "0xfffffffe";
	IStringStream isx(inputx);
	anyTest.Import( isx );
	assertEqual(0xfffffffe, anyTest.AsLong(0));
	t_assert( anyTest.GetType() == AnyLongType );
	t_assert( anyTest.GetSize() == 1 );

}

void ParserTest::parseSimpleTypeDouble ()
{
	Anything anyTest;

	String input0 = "0.0";
	IStringStream is0(input0);
	anyTest.Import( is0 );
	t_assert( anyTest.AsDouble() == 0 );
	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

#ifndef __370__
	String input1 = "1.0123456789E+300";
#else
	String input1 = "1.0123456789E+30";
#endif

	IStringStream is1(input1);
	anyTest.Import( is1 );
#ifndef __370__
	t_assert( anyTest.AsDouble() == 1.0123456789E+300 );
#else
	t_assert( anyTest.AsDouble() == 1.0123456789E+30 );
#endif

	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

#ifndef __370__
	String input3 = "-1.0123456789E+300";
#else
	String input3 = "-1.0123456789E+30";
#endif
	IStringStream is3(input3);
	anyTest.Import( is3 );
#ifndef __370__
	t_assert( anyTest.AsDouble() == -1.0123456789E+300 );
#else
	t_assert( anyTest.AsDouble() == -1.0123456789E+30 );
#endif

	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

#ifndef __370__
	String input4 		= "1.0123456789E-300";
#else
	String input4 		= "1.0123456789E-30";
#endif
	IStringStream is4(input4);
	anyTest.Import( is4 );
#ifndef __370__
	t_assert( anyTest.AsDouble() == 1.0123456789E-300 );
#else
	t_assert( anyTest.AsDouble() == 1.0123456789E-30 );
#endif

	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

#ifndef __370__
	String input5 		= "-1.0123456789E-300";
#else
	String input5 		= "-1.0123456789E-30";
#endif
	IStringStream is5(input5);
	anyTest.Import( is5 );
#ifndef __370__
	t_assert( anyTest.AsDouble() == -1.0123456789E-300 );
#else
	t_assert( anyTest.AsDouble() == -1.0123456789E-30 );
#endif

	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

	String input6 		= "-1.0123456789E-350";
	IStringStream is6(input6);
	anyTest.Import( is6 );
	t_assert( anyTest.AsDouble() == 0 );
	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

	String input7 		= "1.0123456789E-350";
	IStringStream is7(input7);
	anyTest.Import( is7 );
	t_assert( anyTest.AsDouble() == 0 );
	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

	String input8 		= "1.0123456789E+350";
	IStringStream is8(input8);
	anyTest.Import( is8 );
	if ( anyTest.AsCharPtr() ) {
		String tt = anyTest.AsString();
		//assertCharPtrEqual( anyTest.AsCharPtr(), "Infinity" );
#if defined(WIN32)
		t_assert((tt == "1.#INF") ); // Inf vs Infinity for operator<<(double)
#else
		tt.Trim(3);
		tt.ToLower();
		tt.ReplaceAt(0, "i", 1); // the last one is for SunC++5.0
		assertEqual("inf", tt);
		//t_assert((tt=="Infinity") || (tt=="infinity") || (tt=="Inf")); // Inf vs Infinity for operator<<(double)
#endif
	}
	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );

	String input9 		= "-1.0123456789E+350";
	IStringStream is9(input9);
	anyTest.Import( is9 );
	if ( anyTest.AsCharPtr() ) {
		String tt = anyTest.AsString();

#if defined(WIN32)
		t_assert((tt == "-1.#INF") ); // Inf vs Infinity for operator<<(double)
#else
		tt.Trim(4);
		tt.ToLower();
		tt.ReplaceAt(1, "i", 1); // the last one is for SunC++5.0
		assertEqual("-inf", tt);
#endif

	}
	t_assert( anyTest.GetType() == AnyDoubleType );
	t_assert( anyTest.GetSize() == 1 );
}

void ParserTest::importEmptyStream()
{
	String input = "";
	IStringStream is(input);
	Anything any0;
	any0.Import( is );
	t_assert( any0.GetSize() == 0 );
	t_assert( any0.GetType() == AnyNullType );
}

void ParserTest::assertParsedAsDouble(const char *in, double val, int id)
/* in: str: a String which most probably contains a doubleAny
	   val: intended value of the resulting double
		id: identifier for the string in the definition
 what: checks if the Anything parsed from str contains a double
*/
{
	Anything any;
	String str(in);
	IStringStream is(&str);

	is >> any;
	assertEqual(AnyDoubleType, any.GetType());

	if (AnyDoubleType == any.GetType()) {
		assertDoublesEqual(any.AsDouble(), val, 1E-8);
	} else {
		assertEqual(-1, id);
		// crutch until we can add some info, identify whats wrong
	} // if

} // assertParsedAsDouble

void ParserTest::assertParsedAsLong(const char *in, long val, int id)
/* in: str: a String which most probably contains a longAny
	   val: intended value of the resulting long
		id: identifier for the string in the definition
 what: checks if the Anything parsed from str contains a long
*/
{
	Anything any;
	String str(in);
	IStringStream is(&str);

	is >> any;
	assertEqual(AnyLongType, any.GetType());

	if (AnyLongType == any.GetType()) {
		assertEqual(val, any.AsLong());
	} else {
		assertEqual(-1, id);
		// crutch until we can add some info, identify whats wrong
	} // if

} // assertParsedAsDouble

void ParserTest::assertParsedAsString(const char *in, int id)
/* in: str: a String which most probably contains a StringAny
			(also the value we expect)
		id: identifier for the string in the definition
 what: checks if the Anything parsed from in contains a String
*/
{
	Anything any;
	String str(in);
	IStringStream is(&str);

	is >> any;
	assertEqual(AnyCharPtrType, any.GetType());

	if (AnyCharPtrType == any.GetType()) {
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

void ParserTest::parseNumber()
{
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
	parseDoubleNum( 456.5e23, 23);
	parseDoubleNum(456.5e23 , 24);
	parseLongNum(+3213, 25);
	parseLongNum(+3213 , 26);
	parseLongNum( +3213, 27);
	parseDoubleNum(+234347., 28);
	parseDoubleNum(+436784.344, 29);
	parseLongNum(-5354, 30);
	parseLongNum(-5354 , 31);
	parseDoubleNum(-4324., 32);
	parseDoubleNum(-4324. , 33);
	parseDoubleNum(-324.432, 34);
	parseLongNum(4324, 35);
	parseDoubleNum(54359., 36);
	parseDoubleNum(23.543, 37);
	parseDoubleNum(23.543 , 38);
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

void ParserTest::parseOctal()
{
	String input[100];
	input[0]  = "00";
	input[1]  = "04";
	input[2]  = "07";
	input[3]  = "000";
	input[4]  = "001";
	input[5]  = "010";
	input[6]  = "045";
	input[7]  = "075";
	input[8]  = "067";
	input[9]  = "084";
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

	ParserTest::writeResult( &input[0L], 45, "tmp/AnythingOctal", "res" );
}

void ParserTest::parseBinary()
{
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
	input[30] = "[3;\0x10123]";
	input[31] = "[3;\0x13123]";
	input[32] = "[2364663;3]";
	input[33] = "[3;]]";

	ParserTest::writeResult( &input[0L], 34, "tmp/AnythingBinary", "res" );
}

void ParserTest::parseSimpleTypeNull ()
{
	Anything anyTest;

	String input0 = "";
	IStringStream is0(input0);
	anyTest.Import( is0 );						//ERROR: 0:0:Anything::Import :syntax error
	t_assert( anyTest.GetType() == AnyNullType );
	t_assert( anyTest.GetSize() == 0 );
	t_assert( anyTest.AsDouble() == 0 );
	t_assert( anyTest.AsCharPtr() == NULL );
	//assertCharPtrEqual( anyTest.AsCharPtr(), " " );	// ABORT ????

	String input1 = "*";
	IStringStream is1(input1);
	anyTest.Import( is1 );
	//t_assert( anyTest.AsDouble() == 0 );
	t_assert( anyTest.GetType() == AnyNullType );
	t_assert( anyTest.GetSize() == 0 );
	t_assert( anyTest.AsDouble() == 0 );
	t_assert( anyTest.AsCharPtr() == NULL );

	// \01 ist nicht erlaubt --> eNull
	String input2 = "\01";
	IStringStream is2(input2);
	anyTest.Import( is2 );
	t_assert( anyTest.GetType() == AnyNullType );
	t_assert( anyTest.GetSize() == 0 );
	t_assert( anyTest.AsDouble() == 0 );
}

void ParserTest::parseSimpleTypeCharPtr ()
{
	Anything anyTest;
	long i, j;
	char ch0, ch1;

	// Beginning with a number
	//
	// Theoretisch falsch ABER nicht problematisch:  ist es gew�nscht ????
	//
	String input0 = "1234567890L";
	IStringStream is0(input0);
	anyTest.Import( is0 );
	t_assert( anyTest.GetType() == AnyCharPtrType );		// Because of the L at the end
	t_assert( anyTest.GetSize() == 1 );
	if ( anyTest.AsCharPtr() ) {
		assertCharPtrEqual( anyTest.AsCharPtr(), "1234567890L" );
	}

	// Beginning with a char: OK
	String input1 = "aAbB01234";
	IStringStream is1(input1);
	anyTest.Import( is1 );
	t_assert( anyTest.GetType() == AnyCharPtrType );
	t_assert( anyTest.GetSize() == 1 );
	if ( anyTest.AsCharPtr() ) {
		assertCharPtrEqual( anyTest.AsCharPtr(), "aAbB01234" );
	}

	// Beginning with a '\"' (allowed buffer): OK
	String input2 = "\"aAbB01234\"";
	IStringStream is2(input2);
	anyTest.Import( is2 );
	t_assert( anyTest.GetType() == AnyCharPtrType );
	t_assert( anyTest.GetSize() == 1 );
	if ( anyTest.AsCharPtr() ) {
		assertCharPtrEqual( anyTest.AsCharPtr(), "aAbB01234" );
	}

	// Beginning with a '"' (not allowed buffer:  the not allowed part is simply cut off)
	//
	// Ist es gew�nscht????
	String input3 = "\"aAbB01\"234\"";
	IStringStream is3(input3);
	anyTest.Import( is3 );
	t_assert( anyTest.GetType() == AnyCharPtrType );
	t_assert( anyTest.GetSize() == 1 );
	if ( anyTest.AsCharPtr() ) {
		assertCharPtrEqual( anyTest.AsCharPtr(), "aAbB01" );
	}

	// falsche Syntax:  alle char ohne Esc-Seq.
	// Fehler-Meldung:   .any:2 syntax error: invalid string constant (missing quote)
	// Inhalt undef
	//
	String input4;
	input4.Append("\"");
	for ( i = 1; i < 256; i++ ) {
		input4.Append( (char)i );
	}
	input4.Append("\"");
	IStringStream is4(input4);
	anyTest.Import( is4 );
	t_assert( anyTest.GetType() == AnyCharPtrType );
	t_assert( anyTest.GetSize() == 1 );

	// korrekte Syntax (alle char mit Esc-Seq.)
	String input5;
	input5.Append("\"");
	for ( i = 0; i < 16; i++ ) {
		ch0 = i + '0';
		if ( i > 9 ) {
			ch0 = 'A' + i - 10;
		}

		for ( j = 0; j < 16; j++ ) {
			ch1 = j + '0';
			if ( j > 9 ) {
				ch1 = 'A' + j - 10;
			}
			if ( ( i != 0) || (j != 0) ) {
				input5.Append( '\\' );
				input5.Append( 'x' );
				input5.Append( ch0 );
				input5.Append( ch1 );
			}
		}
	}
	input5.Append("\"");
	IStringStream is5( input5 );
	anyTest.Import( is5 );
	t_assert( anyTest.GetType() == AnyCharPtrType );
	t_assert( anyTest.GetSize() == 1 );
	for ( i = 1; i < 256; i++ ) {
		t_assert( anyTest.AsCharPtr()[i-1] == (char)i );
	}

	// korrekte Syntax (printables: ohne Esc-Seq;  non-printable: mit Esc-Seq)
	String input6;
	input6.Append("\"");
	for ( i = 0; i < 16; i++ ) {
		ch0 = i + '0';
		if ( i > 9 ) {
			ch0 = 'A' + i - 10;
		}

		for ( j = 0; j < 16; j++ ) {
			ch1 = j + '0';
			if ( j > 9 ) {
				ch1 = 'A' + j - 10;
			}

			if ( ( i != 0 ) || (j != 0) ) {
				if ( isprint( (unsigned char)i * 16 + j) ) {
					if ( (char)(i * 16 + j) == '"' ) {
						input6.Append( '\\' );   //   \" sonst wird " als EndOfString interpretiert
					}
					input6.Append( (char)(i * 16 + j) );
				} else {
					input6.Append( '\\' );
					input6.Append( 'x' );
					input6.Append( ch0 );
					input6.Append( ch1 );
				}
			}
		}
	}
	input6.Append("\"");
	IStringStream is6( input6 );
	anyTest.Import( is6 );
	t_assert( anyTest.GetType() == AnyCharPtrType );
	t_assert( anyTest.GetSize() == 1 );

	for ( i = 1; i < 256; i++ ) {
		t_assert( (unsigned char)(anyTest.AsCharPtr()[i-1]) == (unsigned char)i );
	}

	// Falsche Syntax:  Beginning with a printable char without '\"'
	// Syntasch error bemerkt --> NullAnything
	String input7 = "!abc";
	IStringStream is7(input7);
	anyTest.Import( is7 );
	t_assert( anyTest.GetType() == AnyNullType );
	t_assert( anyTest.GetSize() == 0 );
	if ( anyTest.AsCharPtr() ) {
		assertCharPtrEqual( anyTest.AsCharPtr(), "!abc" );
	}
	t_assert( anyTest.GetType() == AnyNullType);
	t_assert( anyTest.GetSize() == 0 );
}

/*==================================================================================*/
/*                    Tests von geschachteten Anythings                             */
/*==================================================================================*/
void ParserTest::parseMixedAnything ()
{
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
	anyB.Append( anyCnew );
	anyBnew["B"] = anyB;
	anyApart.Append(0);
	anyApart.Append(1);
	anyApart.Append(2);
	anyApart.Append(3);
	anyApart.Append(4);
	anyApart.Append( anyBnew );
	anyA.Append(123);
	anyA.Append("123L");
	anyA.Append("String");
	anyA.Append( anyApart );
	anyAnew["A"] = anyA;
	anyAnew["G"] = 123;
	anyAnew["H"] = 1234;
	anyAnew["I"] = "String";

	ostream *osHlp = System::OpenOStream("tmp/AnyTest", "txt", ios::trunc);
	if ( osHlp ) {
		*osHlp << anyAnew;
		delete osHlp;
	} else {
		assertEqual("'write tmp/AnyTest.txt'", "'could not write tmp/AnyTest.txt'" );
	}

	istream *isHlp = System::OpenStream("tmp/AnyTest", "txt");
	if ( !isHlp ) {
		assertEqual("'read tmp/AnyTest.txt'", "'could not read tmp/AnyTest.txt'" );
	} else {	// if ( isHlp )
		Anything anyHlp;
		anyHlp.Import( *isHlp );
		delete isHlp;

		ostream *osNew = System::OpenOStream("tmp/AnyTestNew", "txt", ios::trunc);
		if ( osNew ) {
			*osNew << anyHlp;
			delete osNew;
		} else {
			assertEqual("'write tmp/AnyTestNew.txt'", "'could not write tmp/AnyTestNew.txt'" );
		}

		anyHlp["A"].SlotName(0L);
		t_assert( anyHlp["A"].At(0L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(0L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(0L).AsLong() == 123 );

		anyHlp["A"].SlotName(1L);
		t_assert( anyHlp["A"].At(1L).GetType() == AnyCharPtrType );
		t_assert( anyHlp["A"].At(1L).GetSize() == 1 );
		if ( anyHlp["A"].At(1L).AsCharPtr() ) {
			assertCharPtrEqual( anyHlp["A"].At(1L).AsCharPtr(), "123L" );
		}

		anyHlp["A"].SlotName(2L);
		t_assert( anyHlp["A"].At(2L).GetType() == AnyCharPtrType );
		t_assert( anyHlp["A"].At(2L).GetSize() == 1 );
		if ( anyHlp["A"].At(2L).AsCharPtr() ) {
			assertCharPtrEqual( anyHlp["A"].At(2L).AsCharPtr(), "String" );
		}

		//anyHlp["A"].At[3].SlotName(0L);
		t_assert( anyHlp["A"].At(3L).At(0L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(3L).At(0L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(0L).AsLong() == 0 );

		//anyHlp["A"].At[3].SlotName(1L);
		t_assert( anyHlp["A"].At(3L).At(1L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(3L).At(1L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(1L).AsLong() == 1 );

		//anyHlp["A"].At[3].SlotName(2L);
		t_assert( anyHlp["A"].At(3L).At(2L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(3L).At(2L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(2L).AsLong() == 2 );

		//anyHlp["A"].At[3].SlotName(3L);
		t_assert( anyHlp["A"].At(3L).At(3L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(3L).At(3L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(3L).AsLong()  == 3 );

		//anyHlp["A"].At[3].SlotName(4L);
		t_assert( anyHlp["A"].At(3L).At(4L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(3L).At(4L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(4L).AsLong()  == 4 );

		anyHlp["A"].At(3L).At(5L)["B"].SlotName(0L);
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(0L).GetType() == AnyCharPtrType );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(0L).GetSize() == 1 );
		if ( anyHlp["A"].At(3L).At(5L)["B"].At(0L).AsCharPtr() ) {
			assertCharPtrEqual( anyHlp["A"].At(3L).At(5L)["B"].At(0L).AsCharPtr(), "ABC" );
		}

		anyHlp["A"].At(3L).At(5L)["B"].SlotName(1L);
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(1L).GetType() == AnyCharPtrType );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(1L).GetSize() == 1 );
		if ( anyHlp["A"].At(3L).At(5L)["B"].At(1L).AsCharPtr() ) {
			assertCharPtrEqual( anyHlp["A"].At(3L).At(5L)["B"].At(1L).AsCharPtr(), "BCD" );
		}

		anyHlp["A"].At(3L).At(5L)["B"].SlotName(2L);
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(2L).GetType() == AnyCharPtrType );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(2L).GetSize() == 1 );
		if ( anyHlp["A"].At(3L).At(5L)["B"].At(2L).AsCharPtr() ) {
			assertCharPtrEqual( anyHlp["A"].At(3L).At(5L)["B"].At(2L).AsCharPtr(), "CDE" );
		}

		anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].SlotName(0L);
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(0L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(0L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(0L).AsLong() == 1 );

		anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].SlotName(1L);
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(1L).GetType() == AnyLongType );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(1L).GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["D"]["E"].At(1L).AsLong() == 2 );

		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["F"].GetType() == AnyDoubleType );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["F"].GetSize() == 1 );
		t_assert( anyHlp["A"].At(3L).At(5L)["B"].At(3L)["C"]["F"].AsDouble() == 1e34 );

		t_assert( anyHlp["G"].GetType() == AnyLongType );
		t_assert( anyHlp["G"].GetSize() == 1 );
		t_assert( anyHlp["G"].AsLong() == 123 );

		t_assert( anyHlp["H"].GetType() == AnyLongType );
		t_assert( anyHlp["H"].GetSize() == 1 );
		t_assert( anyHlp["H"].AsLong() == 1234 );
		if ( anyHlp["I"].AsCharPtr() ) {
			assertCharPtrEqual( anyHlp["I"].AsCharPtr(), "String" );
		}
	}
}

/*==================================================================================*/
/*                    Tests, ob Anythings richtig interpretiert                     */
/*==================================================================================*/

void  ParserTest::testSemantic0()
{
	// Init an anything-object for testing
	Anything any = emptyAny;
	any.Append('\01');						//	0
	any.Append("\01");						//	1
	any.Append("\\\\");						//	2
	any.Append("\\\\x41");					//	3
	any.Append("\x41");						//	4		\x41 = "A"
	any["{}"] = 1;							//	5
	any["SlotName0"] = emptyAny;			//	6
	any["SlotName1"] = "{}";				//	7
	any["SlotName2"] = "\"\"{}\"\"";		//	8

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any[0L].GetType() == AnyLongType );
	t_assert( any[0L].AsLong() == 1L );
	t_assert( any[0L] == '\x01' );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[1L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[1L].AsCharPtr(), "\x01" );
	if (any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[2L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[2L].AsCharPtr(), "\\\\" );
	if (any.SlotName(2L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[3L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[3L].AsCharPtr(), "\\\\x41" );
	if (any.SlotName(3L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[4L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[4L].AsCharPtr(), "\x41" );
	if (any.SlotName(4L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any["{}"].GetType() == AnyLongType );
	t_assert( any["{}"].AsLong() == 1L );
	t_assert( any[5L].GetType() == AnyLongType );
	t_assert( any[5L].AsLong() == 1L );
	assertCharPtrEqual( any.SlotName(5L), "{}" );
	t_assert( any.FindIndex("{}") == 5 );

	t_assert( any["SlotName0"].GetType() == AnyNullType );
	t_assert( any[6L].GetType() == AnyNullType );
	assertCharPtrEqual( any.SlotName(6L), "SlotName0" );
	t_assert( any.FindIndex("SlotName0") == 6 );

	t_assert( any["SlotName1"].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any["SlotName1"].AsCharPtr(), "{}" );
	t_assert( any[7L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[7L].AsCharPtr(), "{}" );
	assertCharPtrEqual( any.SlotName(7L), "SlotName1" );
	t_assert( any.FindIndex("SlotName1") == 7 );

	t_assert( any["SlotName2"].GetType() == AnyCharPtrType );

	char testString[10] = "\"\"{}\"\"";
	assertCharPtrEqual( any["SlotName2"].AsCharPtr(), testString ); // dummy
	t_assert( any[8L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[8L].AsCharPtr(), testString );
	assertCharPtrEqual( any.SlotName(8L), "SlotName2" );
	t_assert( any.FindIndex("SlotName2") == 8 );
}
// testSemantic0

void ParserTest::testSemantic1()
{
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["SlotName0"] = "\"\"";						//	0
	any.Append( emptyAny );							//	1
	any.Append( "\"\"" );							//	2
	any["#ab"] = 3;									//	3
	any["a#b"] = 4;									//	4
	any["ab#"] = 5;									//	5 {
	any["}ab"] = 6;									//	6 {
	any["a}b"] = 7;									//	7 {
	any["ab}"] = 8;									//	8
	any["{ab"] = 9;									//	9 }
	any["a{b"] = 10;								//	10 }
	any["ab{"] = 11;								//	11 }

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 12 );
	t_assert( any["SlotName0"].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetType() == AnyCharPtrType );

	char testString[10] = "\"\"";
	assertCharPtrEqual( any["SlotName0"].AsCharPtr(), testString );
	assertCharPtrEqual( any[0L].AsCharPtr(), testString );
	//assertCharPtrEqual( any["SlotName0"].AsCharPtr(), "\"\"" );
	//assertCharPtrEqual( any[0L].AsCharPtr(), "\"\"" );
	assertCharPtrEqual( any.SlotName(0L), "SlotName0" );
	t_assert( any.FindIndex("SlotName0") == 0 );

	t_assert( any[1L].GetType() == AnyNullType );
	if (any.SlotName(1L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert(0 == 1);
	}

	t_assert( any[2L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[2L].AsCharPtr(), testString );
	//assertCharPtrEqual( any[2L].AsCharPtr(), "\"\"" );
	if (any.SlotName(2L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert(0 == 1);
	}

	t_assert( any["#ab"].GetType() == AnyLongType );
	t_assert( any["#ab"].AsLong() == 3 );
	t_assert( any[3L].GetType() == AnyLongType );
	t_assert( any[3L].AsLong() == 3 );
	assertCharPtrEqual( any.SlotName(3L), "#ab" );
	t_assert( any.FindIndex("#ab") == 3 );

	t_assert( any["a#b"].GetType() == AnyLongType );
	t_assert( any["a#b"].AsLong() == 4 );
	t_assert( any[4L].GetType() == AnyLongType );
	t_assert( any[4L].AsLong() == 4 );
	assertCharPtrEqual( any.SlotName(4L), "a#b" );
	t_assert( any.FindIndex("a#b") == 4 );

	t_assert( any["ab#"].GetType() == AnyLongType );
	t_assert( any["ab#"].AsLong() == 5 );
	t_assert( any[5L].GetType() == AnyLongType );
	t_assert( any[5L].AsLong() == 5 );
	assertCharPtrEqual( any.SlotName(5L), "ab#" );
	t_assert( any.FindIndex("ab#") == 5 );
	// {
	t_assert( any["}ab"].GetType() == AnyLongType );	// {
	t_assert( any["}ab"].AsLong() == 6 );
	t_assert( any[6L].GetType() == AnyLongType );
	t_assert( any[6L].AsLong() == 6 );	// {
	assertCharPtrEqual( any.SlotName(6L), "}ab" );	// {
	t_assert( any.FindIndex("}ab") == 6 );
	// {
	t_assert( any["a}b"].GetType() == AnyLongType ); // {
	t_assert( any["a}b"].AsLong() == 7 );
	t_assert( any[7L].GetType() == AnyLongType );
	t_assert( any[7L].AsLong() == 7 );	// {
	assertCharPtrEqual( any.SlotName(7L), "a}b" );	// {
	t_assert( any.FindIndex("a}b") == 7 );
	// {
	t_assert( any["ab}"].GetType() == AnyLongType ); // {
	t_assert( any["ab}"].AsLong() == 8 );
	t_assert( any[8L].GetType() == AnyLongType );
	t_assert( any[8L].AsLong() == 8 ); // {
	assertCharPtrEqual( any.SlotName(8L), "ab}" ); // {
	t_assert( any.FindIndex("ab}") == 8 );

	t_assert( any["{ab"].GetType() == AnyLongType );// }
	t_assert( any["{ab"].AsLong() == 9 );// }
	t_assert( any[9L].GetType() == AnyLongType );
	t_assert( any[9L].AsLong() == 9 );
	assertCharPtrEqual( any.SlotName(9L), "{ab" );// }
	t_assert( any.FindIndex("{ab") == 9 );// }

	t_assert( any["a{b"].GetType() == AnyLongType );// }
	t_assert( any["a{b"].AsLong() == 10 );// }
	t_assert( any[10L].GetType() == AnyLongType );
	t_assert( any[10L].AsLong() == 10 );
	assertCharPtrEqual( any.SlotName(10L), "a{b" );// }
	t_assert( any.FindIndex("a{b") == 10 );// }

	t_assert( any["ab{"].GetType() == AnyLongType );// }
	t_assert( any["ab{"].AsLong() == 11 );// }
	t_assert( any[11L].GetType() == AnyLongType );
	t_assert( any[11L].AsLong() == 11 );
	assertCharPtrEqual( any.SlotName(11L), "ab{" );// }
	t_assert( any.FindIndex("ab{") == 11 );// }
}
// testSemantic1

void  ParserTest::testSemantic2() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	anyTemp0.Append("b");
	anyTemp0.Append(1);
	any["a"] = anyTemp0;
	any.Append(9);
//	any = {
//			/a {b 1}
//			9
//		  };

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 2 );
	t_assert( any["a"].GetType() == AnyArrayType );
	t_assert( any["a"].GetSize() == 2 );
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[0L].GetSize() == 2 );
	if ( assertEqual( AnyCharPtrType, any["a"][0L].GetType() ) ) {
		assertCharPtrEqual( any["a"][0L].AsCharPtr(), "b" );
	}
	t_assert( any["a"][1L].GetType() == AnyLongType );
	t_assert( any["a"][1L].AsLong() == 1 );
	if ( assertEqual( AnyCharPtrType, any[0L][0L].GetType() ) ) {
		assertCharPtrEqual( any[0L][0L].AsCharPtr(), "b" );
	}
	t_assert( any[0L][1L].GetType() == AnyLongType );
	t_assert( any[0L][1L].AsLong() == 1 );
	t_assert( any[1L].GetType() ==  AnyLongType );
	t_assert( any[1L].AsLong() == 9 );
	assertCharPtrEqual( any.SlotName(0L), "a" );
	t_assert( any.FindIndex("a") == 0 );
	if ( any["a"].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any["a"].SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any[0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any[0L].SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

}
// testSemantic2

void  ParserTest::testSemantic3() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	anyTemp0.Append(2);
	any["ab"] = anyTemp0;
	any.Append(0);
//	{
//		/ab {
//				2
//			}
//		0
//	}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 2 );
	t_assert( any["ab"].GetType() == AnyArrayType );
	t_assert( any["ab"].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[0L].GetSize() == 1 );
	t_assert( any["ab"][0L].GetType() == AnyLongType );
	t_assert( any["ab"][0L].AsLong() == 2 );
	t_assert( any[0L][0L].GetType() == AnyLongType );
	t_assert( any[0L][0L].AsLong() == 2 );
	t_assert( any[1L].GetType() ==  AnyLongType );
	t_assert( any[1L].AsLong() == 0 );
	assertCharPtrEqual( any.SlotName(0L), "ab" );
	t_assert( any.FindIndex("ab") == 0 );
	if ( any["ab"].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any[0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic3

void  ParserTest::testSemantic4() {
	// Init an anything-object for testing
	Anything any = emptyAny;//{
	anyTemp0.Append( "2}6" );
	any["ab"] = anyTemp0;
//	{ Anything-Object filled with Stream =
//	{
//	  /ab {
//		"2}6"
//	  }
//	}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );
	t_assert( any["ab"].GetType() == AnyArrayType );
	t_assert( any["ab"].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[0L].GetSize() == 1 );
	t_assert( any["ab"][0L].GetType() == AnyCharPtrType );//{
	assertCharPtrEqual( any["ab"][0L].AsCharPtr(), "2}6" );
	t_assert( any[0L][0L].GetType() == AnyCharPtrType );//{
	assertCharPtrEqual( any[0L][0L].AsCharPtr(), "2}6" );
	assertCharPtrEqual( any.SlotName(0L), "ab" );
	t_assert( any.FindIndex("ab") == 0 );
	if ( any["ab"].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any[0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic4

void  ParserTest::testSemantic5() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["{ab" ] = "2}7";
//	Anything-Object filled with Stream =
//	{
//  	/"{ab" "2}7"
//	}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );
	t_assert( any["{ab"].GetType() == AnyCharPtrType );// }
	t_assert( any["{ab"].GetSize() == 1 );// }
	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any["{ab"].AsCharPtr(), "2}7" );//{
	assertCharPtrEqual( any[0L].AsCharPtr(), "2}7" );
	assertCharPtrEqual( any.SlotName(0L), "{ab" );// }
	t_assert( any.FindIndex("{ab") == 0 );// }
}
}
// testSemantic5
void  ParserTest::testSemantic6() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any[_QUOTE_(a{b) ] = _QUOTE_(2}8);
//	Anything-Object filled with Stream =
//	{
//  	/"a{b" "2}8"
//	}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );
	t_assert( any["a{b"].GetType() == AnyCharPtrType );// }
	t_assert( any["a{b"].GetSize() == 1 );// }
	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any["a{b"].AsCharPtr(), "2}8" );//{
	assertCharPtrEqual( any[0L].AsCharPtr(), "2}8" );
	assertCharPtrEqual( any.SlotName(0L), "a{b" );// }
	t_assert( any.FindIndex("a{b") == 0 );// }
}
// testSemantic6

void ParserTest::testSemantic7() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any[_QUOTE_(ab{) ] = _QUOTE_(2}9);
//	Anything-Object filled with Stream =
//	{
//  	/"ab{" "2}9"
//	}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );
	t_assert( any["ab{"].GetType() == AnyCharPtrType );// }
	t_assert( any["ab{"].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any["ab{"].AsCharPtr(), "2}9" );
	assertCharPtrEqual( any[0L].AsCharPtr(), "2}9" );
	assertCharPtrEqual( any.SlotName(0L), "ab{" );// }
	t_assert( any.FindIndex("ab{") == 0 );// }
}
// testSemantic7

void ParserTest::testSemantic8() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["\"Slot\"Name\""] = 33;				//	0
	any["Slot\"Name\""] = 34;				//	1
	any.Append("/Slot\"Name");				//	2
	any.Append(35);							//	3
//	Anything-Object filled with Stream =
//	{
//  	/"Slot\"Name" 33
//		/Slot\"Name"  34
//		"/Slot\"Name"
//		35
//	}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 4 );
	String slot("\"Slot\"Name\"");
	t_assert( any[slot].GetType() == AnyLongType );
	t_assert( any[slot].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyLongType );
	t_assert( any[0L].GetSize() == 1 );
	t_assert( any[slot].AsLong() == 33 );
	t_assert( any[0L].AsLong() == 33 );
	assertCharPtrEqual( any.SlotName(0L), slot );
	t_assert( any.FindIndex(slot) == 0 );
	String slot2("Slot\"Name\"");
	t_assert( any[slot2].GetType() == AnyLongType );
	t_assert( any[slot2].GetSize() == 1 );
	t_assert( any[1L].GetType() == AnyLongType );
	t_assert( any[1L].GetSize() == 1 );
	t_assert( any[slot2].AsLong() == 34 );
	t_assert( any[1L].AsLong() == 34 );
	assertCharPtrEqual( any.SlotName(1L), slot2 );

	t_assert( any.FindIndex(slot2) == 1L );

	String slot3("/Slot\"Name");
	t_assert( any[2L].GetType() == AnyCharPtrType );
	t_assert( any[2L].GetSize() == 1 );
	assertCharPtrEqual( any[2L].AsCharPtr(), slot3 );
	if ( any.SlotName(2L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	t_assert( any[3L].GetType() == AnyLongType );
	t_assert( any[3L].GetSize() == 1 );
	t_assert( any[3L].AsLong() == 35 );
	if ( any.SlotName(3L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic8

void ParserTest::testSemantic9() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["/ab"] = 9L;							//	0
	any["a/b"] = 10L;						//	1
	any["ab/"] = 11L;						//	2
	any["0"] = "d";							//	3

// Anything-Object filled with Stream =
//{
//  /"/ab" 9
//  /"a/b" 10
//  /"ab/" 11
//	/"0"   "d"
//}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 4 );
	t_assert( any["/ab"].GetType() == AnyLongType );
	t_assert( any["/ab"].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyLongType );
	t_assert( any[0L].GetSize() == 1 );
	t_assert( any["/ab"].AsLong() == 9 );
	t_assert( any[0L].AsLong() == 9 );
	assertCharPtrEqual( any.SlotName(0L), "/ab" );
	t_assert( any.FindIndex("/ab") == 0 );

	t_assert( any["a/b"].GetType() == AnyLongType );
	t_assert( any["a/b"].GetSize() == 1 );
	t_assert( any[1L].GetType() == AnyLongType );
	t_assert( any[1L].GetSize() == 1 );
	t_assert( any["a/b"].AsLong() == 10 );
	t_assert( any[1L].AsLong() == 10 );
	assertCharPtrEqual( any.SlotName(1L), "a/b" );
	t_assert( any.FindIndex("a/b") == 1 );

	t_assert( any["ab/"].GetType() == AnyLongType );
	t_assert( any["ab/"].GetSize() == 1 );
	t_assert( any[2L].GetType() == AnyLongType );
	t_assert( any[2L].GetSize() == 1 );
	t_assert( any["ab/"].AsLong() == 11 );
	t_assert( any[2L].AsLong() == 11 );
	assertCharPtrEqual( any.SlotName(2L), "ab/" );
	t_assert( any.FindIndex("ab/") == 2 );

	t_assert( any["0"].GetType() == AnyCharPtrType );
	t_assert( any["0"].GetSize() == 1 );
	t_assert( any[3L].GetType() == AnyCharPtrType );
	t_assert( any[3L].GetSize() == 1 );
	assertCharPtrEqual( any["0"].AsCharPtr(), "d"  );
	assertCharPtrEqual( any[3L].AsCharPtr(), "d" );
	assertCharPtrEqual( any.SlotName(3L), "0" );
	t_assert( any.FindIndex("0") == 3 );

	// Test if anything-objects can change correctly
	any["ab/"] = "2/9";						//	2
	any["a/b"] = "2/8";						//	1
	any["/ab"] = "2/7";						//	0

// Anything-Object filled with Stream =
//{
//  /"/ab" "2/7"
//  /"a/b" "2/8"
//  /"ab/" "2/9"
//}

	t_assert( any["/ab"].GetType() == AnyCharPtrType );
	t_assert( any["/ab"].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any["/ab"].AsCharPtr(), "2/7");
	assertCharPtrEqual( any[0L].AsCharPtr(), "2/7" );
	assertCharPtrEqual( any.SlotName(0L), "/ab" );
	t_assert( any.FindIndex("/ab") == 0 );

	t_assert( any["a/b"].GetType() == AnyCharPtrType );
	t_assert( any["a/b"].GetSize() == 1 );
	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any["a/b"].AsCharPtr(), "2/8");
	assertCharPtrEqual( any[1L].AsCharPtr(), "2/8" );
	assertCharPtrEqual( any.SlotName(1L), "a/b" );
	t_assert( any.FindIndex("a/b") == 1 );

	t_assert( any["ab/"].GetType() == AnyCharPtrType );
	t_assert( any["ab/"].GetSize() == 1 );
	t_assert( any[2L].GetType() == AnyCharPtrType );
	t_assert( any[2L].GetSize() == 1 );
	assertCharPtrEqual( any["ab/"].AsCharPtr(), "2/9");
	assertCharPtrEqual( any[2L].AsCharPtr(), "2/9" );
	assertCharPtrEqual( any.SlotName(2L), "ab/" );
	t_assert( any.FindIndex("ab/") == 2 );

	t_assert( any["0"].GetType() == AnyCharPtrType );
	t_assert( any["0"].GetSize() == 1 );
	t_assert( any[3L].GetType() == AnyCharPtrType );
	t_assert( any[3L].GetSize() == 1 );
	assertCharPtrEqual( any["0"].AsCharPtr(), "d"  );
	assertCharPtrEqual( any[3L].AsCharPtr(), "d" );
	assertCharPtrEqual( any.SlotName(3L), "0" );
	t_assert( any.FindIndex("0") == 3 );
}
// testSemantic9

void ParserTest::testSemantic10() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["a"] = emptyAny;		//	0
	any["a{"] = "}";			//	1
	any["{a"] = "}";			//	2 {
	any["}"] = "b";				//	3
	any["{"] = "c";				//	4 }
// Anything-Object filled with Stream =
//{
//  /a 		*
//  /"a{"	"}"
//	/"{a"	"}"
//  /"}"	"b"
//	/"{"	"c"
//}
// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any["a"].GetType() == AnyNullType );
	t_assert( any["a"].GetSize() == 0 );
	t_assert( any[0L].GetType() == AnyNullType );
	t_assert( any[0L].GetSize() == 0 );
	assertCharPtrEqual( any.SlotName(0L), "a" );
	t_assert( any.FindIndex("a") == 0L );

	t_assert( any["a{"].GetType() == AnyCharPtrType );// }
	t_assert( any["a{"].GetSize() == 1 );// }
	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any["a{"].AsCharPtr(), "}"  );//{
	assertCharPtrEqual( any[1L].AsCharPtr(), "}" );
	assertCharPtrEqual( any.SlotName(1L), "a{" );// }
	t_assert( any.FindIndex("a{") == 1L );// }

	t_assert( any["{a"].GetType() == AnyCharPtrType );// }
	t_assert( any["{a"].GetSize() == 1 );// }
	t_assert( any[2L].GetType() == AnyCharPtrType );
	t_assert( any[2L].GetSize() == 1 );
	assertCharPtrEqual( any["{a"].AsCharPtr(), "}"  );//{
	assertCharPtrEqual( any[2L].AsCharPtr(), "}" );
	assertCharPtrEqual( any.SlotName(2L), "{a" );// }
	t_assert( any.FindIndex("{a") == 2L );// }
	//{
	t_assert( any["}"].GetType() == AnyCharPtrType );//{
	t_assert( any["}"].GetSize() == 1 );
	t_assert( any[3L].GetType() == AnyCharPtrType );
	t_assert( any[3L].GetSize() == 1 );//{
	assertCharPtrEqual( any[_QUOTE_(})].AsCharPtr(), "b"  );
	assertCharPtrEqual( any[3L].AsCharPtr(), "b" );//{
	assertCharPtrEqual( any.SlotName(3L), "}" );//{
	t_assert( any.FindIndex("}") == 3L );

	t_assert( any["{"].GetType() == AnyCharPtrType );// }
	t_assert( any["{"].GetSize() == 1 );// }
	t_assert( any[4L].GetType() == AnyCharPtrType );
	t_assert( any[4L].GetSize() == 1 );
	assertCharPtrEqual( any["{"].AsCharPtr(), "c"  );// }
	assertCharPtrEqual( any[4L].AsCharPtr(), "c" );
	assertCharPtrEqual( any.SlotName(4L), "{" );// }
	t_assert( any.FindIndex("{") == 4L );// }
}
// testSemantic10

void ParserTest::testSemantic11() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["\"Slo{t}Name\""] = "\"a\\\"{}c\"";				//	0 {
	any["\"Slot}Name\""] = "\"a\\\"{}c\"";				//	1

// {Anything-Object filled with Stream =
//{
//  /"Slo{t}Name" "a\"{}c"
//  /"Slot}Name" "a\"{}c"
//}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	String slot("\"Slo{t}Name\"");
	t_assert( any[slot].GetType() == AnyCharPtrType );
	t_assert( any[slot].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );

	char testString[10] = "\"a\\\"{}c\"";
	assertCharPtrEqual( any[slot].AsCharPtr(), testString );
	assertCharPtrEqual( any[0L].AsCharPtr(), testString );
	assertCharPtrEqual( any.SlotName(0L), slot );
	t_assert( any.FindIndex(slot) == 0L );	//{
	String slot2("\"Slot}Name\"");
	t_assert( any[slot2].GetType() == AnyCharPtrType );
	t_assert( any[slot2].GetSize() == 1 );
	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any[slot2].AsCharPtr(), testString );
	assertCharPtrEqual( any[1L].AsCharPtr(), testString );
	assertCharPtrEqual( any.SlotName(1L), slot2 );
	t_assert( any.FindIndex(slot2) == 1L );
}
// testSemantic11

void ParserTest::testSemantic12() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	anyTemp0.Append("a");
	any.Append( emptyAny );
	any["Test155"] = anyTemp0;
// Anything-Object filled with Stream =
//{
//  *
//  /Test155 {
//    "a"
//  }
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any[0L].GetType() == AnyNullType );
	t_assert( any[0L].GetSize() == 0 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any["Test155"].GetType() == AnyArrayType );
	t_assert( any["Test155"].GetSize() == 1 );
	t_assert( any[1L].GetType() == AnyArrayType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any["Test155"][0L].AsCharPtr(), "a" );
	assertCharPtrEqual( any[1L][0L].AsCharPtr(), "a" );
	assertCharPtrEqual( any.SlotName(1L), "Test155" );
	if ( any["Test155"].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any[1L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic12

void ParserTest::testSemantic13() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	anyTemp0.Append("a");
	any["SlotName"] = emptyAny;
	any["Test155"] = anyTemp0;
// Anything-Object filled with Stream =
//{
//  /SlotName *
//  /Test155 {
//    "a"
//  }
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any[0L].GetType() == AnyNullType );
	t_assert( any[0L].GetSize() == 0 );
	t_assert( any["SlotName"].GetType() == AnyNullType );
	t_assert( any["SlotName"].GetSize() == 0 );
	assertCharPtrEqual( any.SlotName(0L), "SlotName" );

	t_assert( any["Test155"].GetType() == AnyArrayType );
	t_assert( any["Test155"].GetSize() == 1 );
	t_assert( any[1L].GetType() == AnyArrayType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any["Test155"][0L].AsCharPtr(), "a" );
	assertCharPtrEqual( any[1L][0L].AsCharPtr(), "a" );
	assertCharPtrEqual( any.SlotName(1L), "Test155" );
	if ( any["Test155"].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any[1L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic13

void ParserTest::testSemantic14() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any.Append( emptyAny );
	any.Append( emptyAny );
	any["a"] = emptyAny;
	any["b"] = emptyAny;
	any["c"] = emptyAny;
// Anything-Object filled with Stream =
//{
//  *
//  *
//  /a	*
//  /b	*
//  /c	*
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any[0L].GetType() == AnyNullType );
	t_assert( any[0L].GetSize() == 0 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[1L].GetType() == AnyNullType );
	t_assert( any[1L].GetSize() == 0 );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[2L].GetType() == AnyNullType );
	t_assert( any[2L].GetSize() == 0 );
	t_assert( any["a"].GetType() == AnyNullType );
	t_assert( any["a"].GetSize() == 0 );
	assertCharPtrEqual( any.SlotName(2L), "a" );
	t_assert( any.FindIndex("a") == 2L );

	t_assert( any[3L].GetType() == AnyNullType );
	t_assert( any[3L].GetSize() == 0 );
	t_assert( any["b"].GetType() == AnyNullType );
	t_assert( any["b"].GetSize() == 0 );
	assertCharPtrEqual( any.SlotName(3L), "b" );
	t_assert( any.FindIndex("b") == 3L );

	t_assert( any[4L].GetType() == AnyNullType );
	t_assert( any[4L].GetSize() == 0 );
	t_assert( any["c"].GetType() == AnyNullType );
	t_assert( any["c"].GetSize() == 0 );
	assertCharPtrEqual( any.SlotName(4L), "c" );
	t_assert( any.FindIndex("c") == 4L );
}
// testSemantic14

void ParserTest::testSemantic15() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any = "{}";
// Anything-Object filled with Stream =
//{}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyCharPtrType );
	t_assert( any.GetSize() == 1 );
	assertCharPtrEqual( any.AsCharPtr(), "{}" );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	// Check if anything-objects can change correctly
	any = 34L;
	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyLongType );
	t_assert( any.GetSize() == 1 );
	t_assert( any.AsLong() == 34 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	// Check if anything-objects can change correctly
	any = 34.3E-56;
	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyDoubleType );
	t_assert( any.GetSize() == 1 );
	t_assert( any.AsDouble() == 34.3E-56 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic15

void ParserTest::testSemantic16() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	anyTemp0["Test147Dup"] = "a";
	anyTemp0.Append( 3L );
	any["Test147"] = anyTemp0;
// Anything-Object filled with Stream =
//{
//  /Test147 {
//    /Test147Dup "a"
//	  3
//  }
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );

	t_assert( any["Test147"].GetType() == AnyArrayType );
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any["Test147"].GetSize() == 2 );
	t_assert( any[0L].GetSize() == 2 );
	assertCharPtrEqual( any.SlotName(0L), "Test147" );
	t_assert( any.FindIndex("Test147") == 0L );

	t_assert( any["Test147"][0L].GetType() == AnyCharPtrType );
	t_assert( any["Test147"]["Test147Dup"].GetType() == AnyCharPtrType );
	t_assert( any[0L]["Test147Dup"].GetType() == AnyCharPtrType );
	t_assert( any[0L][0L].GetType() == AnyCharPtrType );

	t_assert( any["Test147"][0L].GetSize() == 1 );
	t_assert( any["Test147"]["Test147Dup"].GetSize() == 1 );
	t_assert( any[0L]["Test147Dup"].GetSize() == 1 );
	t_assert( any[0L][0L].GetSize() == 1 );

	assertCharPtrEqual( any["Test147"][0L].AsCharPtr(), "a" );
	assertCharPtrEqual( any["Test147"]["Test147Dup"].AsCharPtr(), "a" );
	assertCharPtrEqual( any[0L]["Test147Dup"].AsCharPtr(), "a" );
	assertCharPtrEqual( any[0L][0L].AsCharPtr(), "a" );

	assertCharPtrEqual( any["Test147"].SlotName(0L), "Test147Dup" );
	assertCharPtrEqual( any[0L].SlotName(0L), "Test147Dup" );

	t_assert( any["Test147"].FindIndex("Test147Dup") == 0L );
	t_assert( any[0L].FindIndex("Test147Dup") == 0L );

	t_assert( any["Test147"][1L].GetType() == AnyLongType );
	t_assert( any[0L][1L].GetType() == AnyLongType );

	t_assert( any["Test147"][1L].GetSize() == 1 );
	t_assert( any[0L][1L].GetSize() == 1 );

	if ( any["Test147"].SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
	if ( any[0L].SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic16

void ParserTest::testSemantic17() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["Test145"] = emptyAny;
// Anything-Object filled with Stream =
//{
//  /Test145 { }
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );
	t_assert( any.FindIndex("Test145") == 0 );
	assertCharPtrEqual( any.SlotName(0L), "Test145" );

	t_assert( any["Test145"].GetType() == AnyNullType );
	t_assert( any[0L].GetType() == AnyNullType );
	t_assert( any["Test145"].GetSize() == 0 );
	t_assert( any[0L].GetSize() == 0 );
}
// testSemantic17

void ParserTest::testSemantic18() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	anyTemp0 = emptyAny;
	anyTemp1.Append(anyTemp0);
	anyTemp2.Append(anyTemp1);
	anyTemp3.Append(anyTemp2);
	any["Test143"].Append(anyTemp3);
// Anything-Object filled with Stream =
//{
//  /Test143 {
//    {
//      {
//        {
//			*
//        }
//      }
//    }
//  }
//}
//
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );
	t_assert( any.FindIndex("Test143") == 0 );
	assertCharPtrEqual( any.SlotName(0L), "Test143" );

	t_assert( any["Test143"].GetType() == AnyArrayType );
	t_assert( any["Test143"].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any.SlotName(0L), "Test143" );

	t_assert( any[0L][0L].GetType() == AnyArrayType );
	t_assert( any[0L][0L].GetSize() == 1 );
	if ( any["Test143"].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[0L][0L][0L].GetType() == AnyArrayType );
	t_assert( any[0L][0L][0L].GetSize() == 1 );
	if ( any["Test143"][0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[0L][0L][0L][0L].GetType() == AnyArrayType );
	t_assert( any[0L][0L][0L][0L].GetSize() == 1 );
	if ( any["Test143"][0L][0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[0L][0L][0L][0L][0L].GetType() == AnyNullType );
	t_assert( any[0L][0L][0L][0L][0L].GetSize() == 0 );
	if ( any["Test143"][0L][0L][0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic18

void ParserTest::testSemantic19() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["Test150"] = "abc";
	any.Append("dfg");
	any.Append("\\String1\"");
	any.Append("\\String2");
	any.Append("/String3\"");
	any.Append("/String4");
	any.Append("\\");
	any.Append("/");
// Anything-Object filled with Stream =
//{
//  /Test150 "abc"	(/Test150 abc)
//  "dfg"			(dfg)
//	"\String1\""	(\String1")
//	"\String2"		(\String2)
//	"/String3\""	(/String3")
//	"/String4"		(/String4)
//	"\\"			(\)
//	"\/"			(/)
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 8 );
	t_assert( any.FindIndex("Test150") == 0 );
	assertCharPtrEqual( any.SlotName(0L), "Test150" );

	t_assert( any["Test150"].GetType() == AnyCharPtrType );
	t_assert( any["Test150"].GetSize() == 1 );
	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any[0L].AsCharPtr(), "abc" );
	assertCharPtrEqual( any["Test150"].AsCharPtr(), "abc" );
	assertCharPtrEqual( any.SlotName(0L), "Test150" );
	t_assert( any.FindIndex("Test150") == 0L );

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any[1L].AsCharPtr(), "dfg" );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[2L].GetType() == AnyCharPtrType );
	t_assert( any[2L].GetSize() == 1 );
	assertCharPtrEqual( any[2L].AsCharPtr(), "\\String1\"" );//"
	if ( any.SlotName(2L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[3L].GetType() == AnyCharPtrType );
	t_assert( any[3L].GetSize() == 1 );
	assertCharPtrEqual( any[3L].AsCharPtr(), "\\String2" );
	if ( any.SlotName(3L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[4L].GetType() == AnyCharPtrType );
	t_assert( any[4L].GetSize() == 1 );
	assertCharPtrEqual( any[4L].AsCharPtr(), "/String3\"" );//"
	if ( any.SlotName(4L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[5L].GetType() == AnyCharPtrType );
	t_assert( any[5L].GetSize() == 1 );
	assertCharPtrEqual( any[5L].AsCharPtr(), "/String4" );
	if ( any.SlotName(5L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[6L].GetType() == AnyCharPtrType );
	t_assert( any[6L].GetSize() == 1 );
	assertCharPtrEqual( any[6L].AsCharPtr(), "\\" );
	if ( any.SlotName(6L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[7L].GetType() == AnyCharPtrType );
	t_assert( any[7L].GetSize() == 1 );
	assertCharPtrEqual( any[7L].AsCharPtr(), "/" );
	if ( any.SlotName(7L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic19

void ParserTest::testSemantic20() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any.Append( "\"\\xGH\"" );
	any.Append( "\"\\GH\"");
// Anything-Object filled with Stream =
//{
//  "\\xGH"
//  "\\GH"
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 2 );

	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	char testString[10] = "\"\\xGH\"";
	assertCharPtrEqual( any[0L].AsCharPtr(), testString );
	//assertCharPtrEqual( any[0L].AsCharPtr(), "\"\\xGH\"" );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );

	char testString2[10] = "\"\\GH\"";
	assertCharPtrEqual( any[1L].AsCharPtr(), testString2 );
	//assertCharPtrEqual( any[1L].AsCharPtr(), "\"\\GH\"" );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic20

void ParserTest::testSemantic21() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["SlotName0"] = "abc";
	any["SlotName1"] = "de\x0E";
	any["SlotName2"] = "fg\0e";	// 0 will terminate the C-string upon construction
	any["SlotName3"] = "hi\x00";
	any["SlotName4"] = "jm\00";
	any["SlotName5"] = String((void *)"n\00p", 3);	// assign a String with \0
	t_assert( any["SlotName5"].AsString() == String((void *)"n\0p", 3));
// Anything-Object filled with Stream =
//{
//  /SlotName0 "abc"
//  /SlotName1 "de\x0E"
//  /SlotName2 "fg\x0e"
//	/SlotName3 "hi\x00"
//	/SlotName4 "jm\00"
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 6 );

	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any["SlotName0"].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	t_assert( any["SlotName0"].GetSize() == 1 );
	assertCharPtrEqual( any[0L].AsCharPtr(), "abc" );
	assertCharPtrEqual( any.SlotName(0L), "SlotName0" );

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any["SlotName1"].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	t_assert( any["SlotName1"].GetSize() == 1 );
	assertCharPtrEqual( any[1L].AsCharPtr(), "de\x0E" );
	assertCharPtrEqual( any.SlotName(1L), "SlotName1" );

	t_assert( any[2L].GetType() == AnyCharPtrType );
	t_assert( any["SlotName2"].GetType() == AnyCharPtrType );
	t_assert( any[2L].GetSize() == 1 );
	t_assert( any["SlotName2"].GetSize() == 1 );
	assertCharPtrEqual( any[2L].AsCharPtr(), "fg" );	// 0ctal, end of C-string
	assertCharPtrEqual( any[2L].AsCharPtr(), "fg\0e" );
	assertCharPtrEqual( any.SlotName(2L), "SlotName2" );

	t_assert( any[3L].GetType() == AnyCharPtrType );
	t_assert( any["SlotName3"].GetType() == AnyCharPtrType );
	t_assert( any[3L].GetSize() == 1 );
	t_assert( any["SlotName3"].GetSize() == 1 );
	assertCharPtrEqual( any[3L].AsCharPtr(), "hi" );
	assertCharPtrEqual( any.SlotName(3L), "SlotName3" );

	t_assert( any[4L].GetType() == AnyCharPtrType );
	t_assert( any["SlotName4"].GetType() == AnyCharPtrType );
	t_assert( any[4L].GetSize() == 1 );
	t_assert( any["SlotName4"].GetSize() == 1 );
	assertCharPtrEqual( any[4L].AsCharPtr(), "jm" );
	assertCharPtrEqual( any.SlotName(4L), "SlotName4" );

	t_assert( any["SlotName5"].GetType() == AnyCharPtrType );
	t_assert( any["SlotName5"].AsString() == String((void *)"n\0p", 3));
	// write and readback of \0 successful
}
// testSemantic21

void ParserTest::testSemantic22() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["SlotName0"] = "# Kein Kommentar";
	any["SlotName1"] = "\x23 Kein Kommentar";
// Anything-Object filled with Stream =
//{
//  /SlotName0 " # Kein Kommentar "
//  /SlotName1 " # Kein Kommentar "
//}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 2 );

	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any["SlotName0"].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	t_assert( any["SlotName0"].GetSize() == 1 );
	assertCharPtrEqual( any[0L].AsCharPtr(), "# Kein Kommentar" );
	assertCharPtrEqual( any.SlotName(0L), "SlotName0" );

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any["SlotName1"].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	t_assert( any["SlotName1"].GetSize() == 1 );
	assertCharPtrEqual( any[1L].AsCharPtr(), "# Kein Kommentar" );
	assertCharPtrEqual( any.SlotName(1L), "SlotName1" );
}
// testSemantic22

void ParserTest::testSemantic23() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any['\x30'] = "a";		// any[0..47] = *;   any[48] = "a"
	any["\x30"] = "b";		// any["\x30"] = "b" (index 49)
	any['\x3'] = "c";		// any[3] = "c"
	any["\x3"] = "d";		// any["\3"] = "d" (index 50)
	any['\30'] = "e";		// any[24] = "e"
	any["\30"] = "f";		// any["\30"] = "f" (index 51)
	any["\3"] = "g";		// any["\3"] = "h" (index 52)

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	long i, iMax = any.GetSize();

	for ( i = 0; i < iMax; i++ ) {
		switch (i) {
			case 48: {
				t_assert( any[i].GetType() == AnyCharPtrType );
				t_assert( any[i].GetSize() == 1 );
				assertCharPtrEqual( any[i].AsCharPtr(), "a" );
				if ( any.SlotName(i) != NULL ) {
					// SlotName should be NULL but it is not:  errMsg
					t_assert( 0 == 1 );
				}
			}
			break;

			case 49: {
				t_assert( any[i].GetType() == AnyCharPtrType );
				t_assert( any[i].GetSize() == 1 );
				assertCharPtrEqual( any[i].AsCharPtr(), "b" );
				t_assert( any["\x30"].GetType() == AnyCharPtrType );
				t_assert( any["\x30"].GetSize() == 1 );
				assertCharPtrEqual( any["\x30"].AsCharPtr(), "b" );
				assertCharPtrEqual( any.SlotName(i), "\x30" );
				t_assert( any.FindIndex("\x30") == i );

			}
			break;

			case 3: {
				t_assert( any[i].GetType() == AnyCharPtrType );
				t_assert( any[i].GetSize() == 1 );
				assertCharPtrEqual( any[i].AsCharPtr(), "c" );
				if ( any.SlotName(i) != NULL ) {
					// SlotName should be NULL but it is not:  errMsg
					t_assert( 0 == 1 );
				}
			}
			break;

			case 50: {
				t_assert( any[i].GetType() == AnyCharPtrType );
				t_assert( any[i].GetSize() == 1 );
				assertCharPtrEqual( any[i].AsCharPtr(), "g" );
				t_assert( any["\x3"].GetType() == AnyCharPtrType );
				t_assert( any["\x3"].GetSize() == 1 );
				assertCharPtrEqual( any["\x3"].AsCharPtr(), "g" );
				assertCharPtrEqual( any.SlotName(i), "\x3" );
				t_assert( any.FindIndex("\x3") == i );

			}
			break;

			case 24: {
				t_assert( any[i].GetType() == AnyCharPtrType );
				t_assert( any[i].GetSize() == 1 );
				assertCharPtrEqual( any[i].AsCharPtr(), "e" );
				if ( any.SlotName(i) != NULL ) {
					// SlotName should be NULL but it is not:  errMsg
					t_assert( 0 == 1 );
				}
			}
			break;

			case 51: {
				t_assert( any[i].GetType() == AnyCharPtrType );
				t_assert( any[i].GetSize() == 1 );
				assertCharPtrEqual( any[i].AsCharPtr(), "f" );
				t_assert( any["\30"].GetType() == AnyCharPtrType );
				t_assert( any["\30"].GetSize() == 1 );
				assertCharPtrEqual( any["\30"].AsCharPtr(), "f" );
				assertCharPtrEqual( any.SlotName(i), "\30" );
				t_assert( any.FindIndex("\30") == i );
			}
			break;

			case 52: {
				t_assert( any[i].GetType() == AnyCharPtrType );
				t_assert( any[i].GetSize() == 1 );
				assertCharPtrEqual( any[i].AsCharPtr(), "g" );
				t_assert( any["\3"].GetType() == AnyCharPtrType );
				t_assert( any["\3"].GetSize() == 1 );
				assertCharPtrEqual( any["\3"].AsCharPtr(), "d" );
				assertCharPtrEqual( any.SlotName(i), "\3" );
				t_assert( any.FindIndex("\3") == i );
			}
			break;

			default: {
				t_assert( any[i].GetType() == AnyNullType );
				t_assert( any[i].GetSize() == 0 );
				if ( any.SlotName(i) != NULL ) {
					// SlotName should be NULL but it is not:  errMsg
					t_assert( 0 == 1 );
				}
			}
			break;
		}
	}
}
// testSemantic23

void ParserTest::testSemantic24() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any.Append( "/a" );
	any.Append( " String" );
	any["a"] = " String";
	any[" String"] = "a";
// Anything-Object filled with Stream =
//{
//  "/a"
//  " String"
//	/a	" String"
//	/" String"	"a"
//}

	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 4 );

	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any[0L].AsCharPtr(), "/a" );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any[1L].AsCharPtr(), " String" );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}

	t_assert( any[2L].GetType() == AnyCharPtrType );
	t_assert( any["a"].GetType() == AnyCharPtrType );
	t_assert( any[2L].GetSize() == 1 );
	t_assert( any["a"].GetSize() == 1 );
	assertCharPtrEqual( any[2L].AsCharPtr(), " String" );
	assertCharPtrEqual( any["a"].AsCharPtr(), " String" );
	assertCharPtrEqual( any.SlotName(2L), "a" );
	t_assert( any.FindIndex("a") == 2L );

	t_assert( any[3L].GetType() == AnyCharPtrType );
	t_assert( any[" String"].GetType() == AnyCharPtrType );
	t_assert( any[3L].GetSize() == 1 );
	t_assert( any[" String"].GetSize() == 1 );
	assertCharPtrEqual( any[3L].AsCharPtr(), "a" );
	assertCharPtrEqual( any[" String"].AsCharPtr(), "a" );
	assertCharPtrEqual( any.SlotName(3L), " String" );
	t_assert( any.FindIndex(" String") == 3L );
}
// testSemantic24

void ParserTest::testSemantic25() {
	// Init an anything-object for testing
	Anything any = emptyAny;

	any["a"] = "A";
	any.Append("B");
	// Change something
	any[1L] = any[0L];
// Anything-Object filled with Stream =
//{
//  /a	"A"
//  "A"
//}
//
// NOT
//{
//  /a	"A"
//  /a	"A"
//}
//
// NOT
//{
//  /a	"A"
//}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 2 );

	t_assert( any[0L].GetType() == AnyCharPtrType );
	t_assert( any[0L].GetSize() == 1 );
	assertCharPtrEqual( any[0L].AsCharPtr(), "A" );
	t_assert( any["a"].GetType() == AnyCharPtrType );
	t_assert( any["a"].GetSize() == 1 );
	assertCharPtrEqual( any["a"].AsCharPtr(), "A" );
	assertCharPtrEqual( any.SlotName(0L), "a" );
	t_assert( any.FindIndex("a") == 0L );

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any[1L].AsCharPtr(), "A" );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( 0 == 1 );
	}
}
// testSemantic25

void ParserTest::testSemantic26() {
	// Init an anything-object for testing
	Anything any = emptyAny;

	any["a"] = "A";
	any["a"]["a"] = "B";
	any["a"]["a"]["a"] = "C";
	any["a"]["a"]["a"]["a"] = "D";
// Anything-Object filled with Stream =
//	{
//	  /a {				//	0
//		"A"
//		/a {			//	1
//		  "B"
//		  /a {			//	2
//			"C"
//			/a "D"		//	3
//		  }
//		}
//	  }
//	}
	// Store and reload 'any'
	Anything any1 = ParserTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any.GetSize() == 1 );

	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[0L].GetSize() == 2 );
	assertCharPtrEqual( any[0L][0L].AsCharPtr(), "A" );
	t_assert( any["a"].GetType() == AnyArrayType );
	t_assert( any["a"].GetSize() == 2 );
	assertCharPtrEqual( any["a"][0L].AsCharPtr(), "A" );
	assertCharPtrEqual( any.SlotName(0L), "a" );
	t_assert( any.FindIndex("a") == 0L );

	t_assert( any[0L][1L].GetType() == AnyArrayType );
	t_assert( any[0L][1L].GetSize() == 2 );
	assertCharPtrEqual( any[0L][1L][0L].AsCharPtr(), "B" );
	t_assert( any["a"]["a"].GetType() == AnyArrayType );
	t_assert( any["a"]["a"].GetSize() == 2 );
	assertCharPtrEqual( any["a"]["a"][0L].AsCharPtr(), "B" );
	assertCharPtrEqual( any["a"].SlotName(1L), "a" );
	t_assert( any["a"].FindIndex("a") == 1L );

	t_assert( any[0L][1L][1L].GetType() == AnyArrayType );
	t_assert( any[0L][1L][1L].GetSize() == 2 );
	assertCharPtrEqual( any[0L][1L][1L][0L].AsCharPtr(), "C" );
	t_assert( any["a"]["a"]["a"].GetType() == AnyArrayType );
	t_assert( any["a"]["a"]["a"].GetSize() == 2 );
	assertCharPtrEqual( any["a"]["a"]["a"][0L].AsCharPtr(), "C" );
	assertCharPtrEqual( any["a"]["a"].SlotName(1L), "a" );
	t_assert( any["a"]["a"].FindIndex("a") == 1L );

	t_assert( any[0L][1L][1L][1L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[0L][1L][1L].SlotName(1L), "a" );
	t_assert( any[0L][1L][1L].FindIndex("D") < 0L );	// Because the type is eCharPtr and not eArray
}
// testSemantic26

void ParserTest::testSemantic27() {
	// Init an anything-object for testing
	Anything any;

	anyTemp0["a"] = 0L;
	anyTemp1["a"] = 1L;
	any["a"].Append(anyTemp0);
	any["a"].Append(anyTemp1);
	any["a"].Append(anyTemp0.DeepClone());
	// Hier wird explizit eine Kopie erzeugt!

//	Manipulationen von anyTemp0 und anyTemp1 modifizieren any.
	anyTemp0["a"] = 2L;
	anyTemp1["a"] = 3L;
	t_assert( any["a"][0L]["a"].AsLong() == anyTemp0["a"].AsLong() );
	t_assert( any["a"][1L]["a"].AsLong() == anyTemp1["a"].AsLong() );
	t_assert( any["a"][2L]["a"].AsLong() != anyTemp0["a"].AsLong() );

//  bestimmte Manipulation in any f�hren zu Ver�nderungen in anyTemp0/1
//  sofern per referenzen zugewiesen wurde.
	any["a"][0L]["b"] = 32L;
	any["a"][1L]["c"] = 43L;
	any["a"][2L]["d"] = 54L;
	t_assert( any["a"][0L]["b"].AsLong() == anyTemp0["b"].AsLong() );
	t_assert( any["a"][1L]["c"].AsLong() == anyTemp1["c"].AsLong() );
	t_assert( !(anyTemp0.IsDefined("d")) );

//	der Link kann auch wieder unterbrochen werden.
any["a"].Remove(0L);		// Vorsicht, andere Eintr�ge werden geschoben
	t_assert( any["a"][0L]["a"] == anyTemp1["a"].AsLong());
}
//	testSemantic27

void ParserTest::testSemantic28Prep( Anything *any ) {
	Anything *anyHlp0, *anyHlp1;
	anyHlp0 = new Anything();
	anyHlp1 = new Anything();

	(*anyHlp0)["a"] = 0L;
	(*anyHlp1)["b"] = 1L;
	any->Append( *anyHlp0 );
	any->Append( *anyHlp1 );

	(*anyHlp0)["a"] = 2L;
	(*anyHlp1)["b"] = 3L;

	delete anyHlp0;
	delete anyHlp1;
	anyHlp0 = 0;
	anyHlp1 = 0;
}
//	testSemantic28Prep

void ParserTest::testSemantic28() {
	// Init an anything-object for testing
	Anything *any;
	any = new Anything();

	ParserTest::testSemantic28Prep( any );

	delete any;
}
//	testSemantic28

void ParserTest::testSemantic29Prep( Anything *any ) {
	Anything anyHlp0, anyHlp1;

	anyHlp0["a"] = 0L;
	anyHlp1["b"] = 1L;
	any->Append( anyHlp0 );
	any->Append( anyHlp1 );

	anyHlp0["a"] = 2L;
	anyHlp1["b"] = 3L;
}
//	testSemantic29Prep

void ParserTest::testSemantic29() {
	// Init an anything-object for testing
	Anything *any;
	any = new Anything();

	ParserTest::testSemantic29Prep( any );

	delete any;
}
//	testSemantic29

void ParserTest::testSemantic30Prep( Anything &any ) {
	Anything anyHlp0, anyHlp1;

	anyHlp0["a"] = 0L;
	anyHlp1["b"] = 1L;
	any.Append( anyHlp0 );
	any.Append( anyHlp1 );

	anyHlp0["a"] = 2L;
	anyHlp1["b"] = 3L;
}
//	testSemantic30Prep

void ParserTest::testSemantic30() {
	// Init an anything-object for testing
	Anything any;

	ParserTest::testSemantic30Prep( any );
}
//	testSemantic30

void ParserTest::testSemantic31() {
	// Init an anything-object for testing
	Anything any, anyHlp0, anyHlp1;

	anyHlp0["a"] = 0L;
	anyHlp1["b"] = 1L;
	any.Append( anyHlp0 );
	any.Append( anyHlp1 );

	anyHlp0["a"] = 2L;
	anyHlp1["b"] = 3L;
}
//	testSemantic31

void ParserTest::testSemantic32() {
	// Init an anything-object for testing
	Anything any;

	anyTemp0["a"] = 0L;
	anyTemp1["a"] = 1L;
	any["a"].Append(anyTemp0);
	any["a"].Append(anyTemp1);
	any["a"].Append(anyTemp0.DeepClone());
	// Hier wird explizit eine Kopie erzeugt!

//	der Link kann auch wieder unterbrochen werden.
	any["a"].Remove(1L);
	t_assert( anyTemp0["a"].AsLong() == 0 );
	t_assert( anyTemp1["a"].AsLong() == 1 );
	t_assert( any[0L].AsLong() == 0L );
	t_assert( any[1L].AsLong() == 0L );
	t_assert( any[2L].GetType() == AnyNullType );

	any["a"].Remove(1L);
	t_assert( anyTemp0["a"].AsLong() == 0 );
	t_assert( anyTemp1["a"].AsLong() == 1 );
	t_assert( any[0L].AsLong() == 0L );
	t_assert( any[1L].GetType() == AnyNullType );
	t_assert( any[2L].GetType() == AnyNullType );

}
//	testSemantic32

void ParserTest::testSemantic33() {
	// Init an anything-object for testing
	Anything any;

	// If the type if anyTemp0/1 is eArray, any.Append( anyTemp0/1 ) makes a copy per reference.
	// Changes in anyTemp0/1 cause changes in "any" automatically
	anyTemp0["a"] = 0L;
	anyTemp1["b"] = 1L;
	any.Append(anyTemp0);
	any.Append(anyTemp1);
	t_assert( anyTemp0.GetType() == AnyArrayType );
	t_assert( anyTemp1.GetType() == AnyArrayType );
	t_assert( any.GetType() == AnyArrayType );
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[1L].GetType() == AnyArrayType );
	t_assert( any[0L]["a"].AsLong() == anyTemp0["a"].AsLong() );
	t_assert( any[1L]["b"].AsLong() == anyTemp1["b"].AsLong() );
	// Change something
	anyTemp0["a"] = 2L;
	anyTemp1["b"] = 3L;
	// Check "any":  because any[0/1] is a reference to anyTemp0/1, "any" has changed, too.
	t_assert( any[0L]["a"].AsLong() == anyTemp0["a"].AsLong() );
	t_assert( any[1L]["b"].AsLong() == anyTemp1["b"].AsLong() );

	// Change the type of anyTemp0/1 from eArray to eLong;
	anyTemp0 = 4L;
	anyTemp1 = 5L;
	t_assert( anyTemp0.GetType() == AnyLongType );
	t_assert( anyTemp1.GetType() == AnyLongType );
	t_assert( anyTemp0.AsLong() == 4 );
	t_assert( anyTemp1.AsLong() == 5 );
	// Because anyTemp0/1 are not eArray, they are not referenced by "any" anymore.
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[1L].GetType() == AnyArrayType );
	t_assert( any[0L]["a"].AsLong() == 2 );
	t_assert( any[1L]["b"].AsLong() == 3 );

	// Do a change
	anyTemp0 = 6L;
	anyTemp1 = 7L;
	t_assert( anyTemp0.AsLong() == 6 );
	t_assert( anyTemp1.AsLong() == 7 );
	// Because anyTemp0/1 are not eArray, they are not referenced by "any" anymore.
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[1L].GetType() == AnyArrayType );
	t_assert( any[0L]["a"].AsLong() == 2 );
	t_assert( any[1L]["b"].AsLong() == 3 );

	// Do a change
	anyTemp0["a"] = 8L;
	anyTemp1["b"] = 9L;
	t_assert( anyTemp0.GetType() == AnyArrayType );
	t_assert( anyTemp1.GetType() == AnyArrayType );
	t_assert( anyTemp0["a"].AsLong() == 8 );
	t_assert( anyTemp1["b"].AsLong() == 9 );
	// Because anyTemp0/1 are not eArray, they are not referenced by "any" anymore.
	t_assert( any[0L].GetType() == AnyArrayType );
	t_assert( any[1L].GetType() == AnyArrayType );
	t_assert( any[0L]["a"].AsLong() == 2 );
	t_assert( any[1L]["b"].AsLong() == 3 );

}
//	testSemantic33
// testquotedslotname PS 7.6.99
void ParserTest::testQuotedSlotname() {
	// test if OutputContext::PrintKey matches parser
#define TESTQUOTE(xslot) \
    {    \
		Anything a;\
        a[ #xslot] = 1;\
        String s;\
        OStringStream os(&s);\
        a.PrintOn(os,false);\
        os.flush();\
        char testString[40]= "{/\"" #xslot "\" 1}";\
        assertEqual(testString ,s);\
        Anything b;\
        IStringStream is(s);\
        b.Import(is);\
        t_assert(b.IsDefined(#xslot));\
        assertEqual(b[#xslot].AsLong(),1);\
    }
// end of macro -- use it
	TESTQUOTE(needs quote);
	TESTQUOTE(1234);
	TESTQUOTE(1startswithnumber);
	TESTQUOTE(another {quote);
//    a = Anything();
//    a["1234"]= 2;
//
//    assertEqual
}

/*==================================================================================*/
/*                    Tests von Imports+Exports von Anythings                       */
/*==================================================================================*/
void ParserTest::readWriteAnything() {
	Anything anyTest, anyTestNew;
	String input[200];
	input[0]  = "{ \"\01\" }";
	input[1]  = "{ \"\\01\" }";
	input[2]  = "{ \"\\\\\" }";
	input[3]  = "{ \"\\\\x41\" }";
	input[4]  = "{ /\"{}\"  1 }";
	input[5]  = "{ /\"\"{}\"\"  1 }";
	input[6]  = "{ /a {} }";
	input[7]  = "{ /b \"{}\" }";
	input[8]  = "{ /c \"\"{}\"\" }";
	input[9]  = "{ /#ab  0 }";
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
	input[132] = "{ \"\\x01\\x02\\x03\\x04\\x05\\x06\\x07\\x08\\x09\\x0A\\x0B\\x0C\\x0D\\x0E\\x10\\x1A\\x20\\x2A\\x70\\x77\\x7A\\x80\\x87\\x88\\x90\\x98\" }";
	input[133] = "{ \"\\01\\02\\03\\04\\05\\06\\07\\08\\09\\0A\\0B\\0C\\0D\\0E\\10\\1A\\20\\2A\\70\\77\\7A\\80\\87\\88\\90\\98\\9A\\88\\89\\FF\" }";
	input[134] = "{ \\01 \\02 \\07 \\08 \\09 \\0A \\0B \\0C \\0D \\0E \\10 \\1A \\20 \\2A \\70 \\77 \\7A \\80 \\87 \\88 \\90 \\98 \\9A \\88 \\89 \\FF }";
	input[135] = "{ \"\\01 \\02 \\07 \\08 \\09 \\0A \\0B \\0C \\0D \\0E \\10 \\1A \\20 \\2A \\70 \\77 \\7A \\80 \\87 \\88 \\90 \\98 \\9A \\88 \\89 \\FF\" }";
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
	input[160] = "{ /SlotName0 0 # /A {} ######}\
				# \"a\" ###########}\
				## 1.2 ##########}\
				### [3;123] #########}\
				/SlotName1  1\
				#####\
				#######\
}";
	input[161] = "{ /SlotName0  \" # Kein Kommentar \" }";
	//input[162] = "{ /SlotName0  Kein\#Kommentar }";
	input[162] = "{ /SlotName0  Kein\\#Kommentar }";
	input[163] = "{ /SlotName0  Kein\x23Kommentar }";		// \x23 == #
	input[164] = "{ /SlotName0  Kein\\x23Kommentar }";
	input[165] = "{/\"Hex\\xGHI\" \"\\xghi\\xhal\\xfgh\"}";
	input[166] = "{/\"\\xABG\" \"\\x323\\xFF3\\xFGH\xABG\"}";

	long i;
	for ( i = 0; i < 167; i++ )

	{
		IStringStream is(input[i]);
		anyTest.Import( is );

		// Ueberpruefe, ob Imports+Exports funktionnieren
		String fileName;
		fileName.Append( "readWriteAnything at index " );
		fileName.Append( i );
		ParserTest::checkImportExport( anyTest, fileName );
	}
}

/*==================================================================================*/
/*                               Miscellaneous                                      */
/*==================================================================================*/
void ParserTest::parseTestFiles() {
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

	for ( testCnt = 0; testCnt < 21; testCnt++ ) {
		istream *is = System::OpenStream( (const char *)fileNames[testCnt], "txt" );
		if ( is ) {
			// Lese die Anything-Datei ein
			Anything any0;
			any0.Import( *is );
			delete is;
			// Untersuche die Semantik des eingelesenen Anything-Objektes
			ParserTest::anyOutput = emptyAny1;
			ParserTest::lineCounter = 1;
			scanAnything( any0 );
			// Schreibe das Ergebnis in eine Datei
			String tmp = "tmp/";
			tmp.Append(fileNames[testCnt]);
			ostream *os = System::OpenOStream( (const char *)tmp, "res", ios::trunc );
			if ( os ) {
				*os << ParserTest::anyOutput;
				delete os;
				ParserTest::anyOutput = emptyAny1;
				ParserTest::lineCounter = 1;
				// Ueberpruefe, ob Imports+Exports funktionnieren
				ParserTest::checkImportExport( any0, tmp ); //fileNames[testCnt] );
			} else {
				String tmp0 = "write ";
				tmp0.Append( tmp );
				tmp0.Append( ".res" );
				String tmp1 = "could not write ";
				tmp1.Append( tmp );
				tmp1.Append( ".res" );
				assertEqual( (const char *)tmp0, (const char *)tmp1 );
			}
		} else {
			String tmp0 = "read ";
			tmp0.Append( fileNames[testCnt] );
			tmp0.Append( ".txt" );
			String tmp1 = "could not read ";
			tmp1.Append( fileNames[testCnt] );
			tmp1.Append( ".txt" );
			assertEqual( (const char *)tmp0, (const char *)tmp1 );
		}
	}
}

void ParserTest::slashSlotnames()
/* what: test Anything parsing with slashes in slotnames
*/
{
	{
		String buf("{ /\"Test/Slot\" 12 }");
		IStringStream is(&buf);
		Anything any;

		is >> any;
		assertEqual("Test/Slot", any.SlotName(0));
		assertEqual(12, any["Test/Slot"].AsLong(-1));
	}
	{
		String buf("{ /Test/Slot 12 }");
		// a slash in a slotname terminates the slot with a null any as value
		// (an error message is written)
		IStringStream is(&buf);
		Anything any;

		is >> any;
		assertEqual("Test", any.SlotName(0));
		t_assert(any["Test"].IsNull());
	}
	{
		String buf("{ //Test 12 }");
		// a slash at the beginning is ignored (but generates an error message)
		IStringStream is(&buf);
		Anything any;

		is >> any;
		assertEqual("Test", any.SlotName(0));
		assertEqual(12, any["Test"].AsLong(-1));	// contains an empty any
	}
} // slashSlotnames
