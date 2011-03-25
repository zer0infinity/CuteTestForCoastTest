/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnythingParserSemanticTest.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "SystemFile.h"
#include "IFAObject.h"

using namespace Coast;

#include "TestSuite.h"

//---- AnythingParserSemanticTest ---------------------------------------------------------
AnythingParserSemanticTest::AnythingParserSemanticTest(TString tname) :
	TestCaseType(tname), lineCounter(0) {
}

void AnythingParserSemanticTest::setUp ()
{
	Anything undefAny;
	emptyAny = undefAny;
	anyTemp0 = undefAny;
	anyTemp1 = undefAny;
	anyTemp2 = undefAny;
	anyTemp3 = undefAny;
	anyTemp4 = undefAny;
}

Test *AnythingParserSemanticTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic0Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic1Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic2Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic3Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic4Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic5Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic6Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic7Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic8Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic9Test);

	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic10Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic11Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic12Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic13Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic14Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic15Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic16Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic17Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic18Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic19Test);

	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic20Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic21Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic22Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic23Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic24Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic25Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic26Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic27Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic28Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic29Test);

	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic30Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic31Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic32Test);
	ADD_CASE(testSuite, AnythingParserSemanticTest, Semantic33Test);

	ADD_CASE(testSuite, AnythingParserSemanticTest, QuotedSlotnameTest);

	ADD_CASE(testSuite, AnythingParserSemanticTest, slashSlotnames );

	return testSuite;
}

/*==================================================================================*/
/*                         Hilfsmethoden fuer Tests                                 */
/*==================================================================================*/
void AnythingParserSemanticTest::checkImportExport( Anything any, String fileName )
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

void AnythingParserSemanticTest::writeResult( String *input , long nrOfElt, char *path, char *ext )
{
	std::ostream *os = System::OpenOStream( path, ext, std::ios::trunc);
	if ( os ) {
		Anything emptyAny1, anyTest, anyTests;

		AnythingParserSemanticTest::anyOutput = emptyAny1;
		AnythingParserSemanticTest::lineCounter = 1;

		long i;
		for ( i = 0; i < nrOfElt; i++ ) {
			IStringStream is(input[i]);
			anyTest.Import(is);
			anyTests.Append( anyTest );
		}
		scanAnything( anyTests );

		*os << AnythingParserSemanticTest::anyOutput;
		delete os;

		AnythingParserSemanticTest::anyOutput = emptyAny1;
		AnythingParserSemanticTest::lineCounter = 1;
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

void  AnythingParserSemanticTest::scanAnything( Anything any0 )
{
	long		i, iMax = any0.GetSize();
	String		slotNm;
	Anything	anyTest;
	String buffer;

	for ( i = 0; i < iMax; i++ ) {
		anyTest = any0[i];

		slotNm = "";
		slotNm.Append( AnythingParserSemanticTest::lineCounter++ );
		slotNm.Append( ") " );
		slotNm.Append( any0.SlotName(i) );
		slotNm.Append( " Size:" );
		slotNm.Append( anyTest.GetSize() );
		slotNm.Append( "; Type:" );

		switch ( anyTest.GetType() ) {
			case AnyLongType: {
				slotNm.Append( "eLong" );
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = anyTest.AsLong();
			}
			break;

			case AnyDoubleType: {
				slotNm.Append( "eDouble" );
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = anyTest.AsDouble();
			}
			break;

			case AnyNullType: {
				slotNm.Append( "eNull" );
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = "NULL";
			}
			break;

			case AnyCharPtrType: {
				slotNm.Append( "eCharPtr" );
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = anyTest.AsCharPtr();
			}
			break;

			case AnyArrayType: {
				slotNm.Append( "eArray" );
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = "...";
				AnythingParserSemanticTest::scanAnything( anyTest );
			}
			break;

			case AnyVoidBufType: {
				slotNm.Append( "eVoidBuf" );
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = anyTest.AsCharPtr();
			}
			break;

			case AnyObjectType: {
				slotNm.Append( "eObject" );
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = anyTest;
			}
			break;

			default: {
				String str("???");//lint !e585
				slotNm.Append( "???" );//lint !e585
				slotNm.Append( "; Value: " );
				AnythingParserSemanticTest::anyOutput[ (const char *)slotNm ] = str;//"???";//String("???");
			}
			break;
		}
	}
}

Anything AnythingParserSemanticTest::storeAndReload( Anything any )
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

void  AnythingParserSemanticTest::Semantic0Test()
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any[0L].GetType() == AnyLongType );
	t_assert( any[0L].AsLong() == 1L );
	t_assert( any[0L] == '\x01' );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
	t_assert( any[1L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[1L].AsCharPtr(), "\x01" );
	if (any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[2L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[2L].AsCharPtr(), "\\\\" );
	if (any.SlotName(2L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
	t_assert( any[3L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[3L].AsCharPtr(), "\\\\x41" );
	if (any.SlotName(3L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
	t_assert( any[4L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[4L].AsCharPtr(), "\x41" );
	if (any.SlotName(4L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
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

void AnythingParserSemanticTest::Semantic1Test()
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}

	t_assert( any[2L].GetType() == AnyCharPtrType );
	assertCharPtrEqual( any[2L].AsCharPtr(), testString );
	//assertCharPtrEqual( any[2L].AsCharPtr(), "\"\"" );
	if (any.SlotName(2L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
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

void  AnythingParserSemanticTest::Semantic2Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}
	if ( any["a"].SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
	if ( any[0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
	if ( any[0L].SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

}
// testSemantic2

void  AnythingParserSemanticTest::Semantic3Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}
	if ( any[0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic3

void  AnythingParserSemanticTest::Semantic4Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}
	if ( any[0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic4

void  AnythingParserSemanticTest::Semantic5Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["{ab" ] = "2}7";
//	Anything-Object filled with Stream =
//	{
//  	/"{ab" "2}7"
//	}

	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
// testSemantic5

void  AnythingParserSemanticTest::Semantic6Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any[_QUOTE_(a{b) ] = _QUOTE_(2}8);
//	Anything-Object filled with Stream =
//	{
//  	/"a{b" "2}8"
//	}

	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic7Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any[_QUOTE_(ab{) ] = _QUOTE_(2}9);
//	Anything-Object filled with Stream =
//	{
//  	/"ab{" "2}9"
//	}

	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic8Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}
	t_assert( any[3L].GetType() == AnyLongType );
	t_assert( any[3L].GetSize() == 1 );
	t_assert( any[3L].AsLong() == 35 );
	if ( any.SlotName(3L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic8

void AnythingParserSemanticTest::Semantic9Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic10Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic11Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic12Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any[0L].GetType() == AnyNullType );
	t_assert( any[0L].GetSize() == 0 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
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
		t_assert( false );
	}
	if ( any[1L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic12

void AnythingParserSemanticTest::Semantic13Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}
	if ( any[1L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic13

void AnythingParserSemanticTest::Semantic14Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any[0L].GetType() == AnyNullType );
	t_assert( any[0L].GetSize() == 0 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[1L].GetType() == AnyNullType );
	t_assert( any[1L].GetSize() == 0 );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
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

void AnythingParserSemanticTest::Semantic15Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any = "{}";
// Anything-Object filled with Stream =
//{}
	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyCharPtrType );
	t_assert( any.GetSize() == 1 );
	assertCharPtrEqual( any.AsCharPtr(), "{}" );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	// Check if anything-objects can change correctly
	any = 34L;
	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyLongType );
	t_assert( any.GetSize() == 1 );
	t_assert( any.AsLong() == 34 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	// Check if anything-objects can change correctly
	any = 34.3E-56;
	// Check if the anything-object is the expected one
	t_assert( any.GetType() == AnyDoubleType );
	t_assert( any.GetSize() == 1 );
	t_assert( any.AsDouble() == 34.3E-56 );
	if ( any.SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic15

void AnythingParserSemanticTest::Semantic16Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}
	if ( any[0L].SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic16

void AnythingParserSemanticTest::Semantic17Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["Test145"] = emptyAny;
// Anything-Object filled with Stream =
//{
//  /Test145 { }
//}
	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic18Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}

	t_assert( any[0L][0L][0L].GetType() == AnyArrayType );
	t_assert( any[0L][0L][0L].GetSize() == 1 );
	if ( any["Test143"][0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[0L][0L][0L][0L].GetType() == AnyArrayType );
	t_assert( any[0L][0L][0L][0L].GetSize() == 1 );
	if ( any["Test143"][0L][0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[0L][0L][0L][0L][0L].GetType() == AnyNullType );
	t_assert( any[0L][0L][0L][0L][0L].GetSize() == 0 );
	if ( any["Test143"][0L][0L][0L].SlotName(0L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic18

void AnythingParserSemanticTest::Semantic19Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}

	t_assert( any[2L].GetType() == AnyCharPtrType );
	t_assert( any[2L].GetSize() == 1 );
	assertCharPtrEqual( any[2L].AsCharPtr(), "\\String1\"" );//"
	if ( any.SlotName(2L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[3L].GetType() == AnyCharPtrType );
	t_assert( any[3L].GetSize() == 1 );
	assertCharPtrEqual( any[3L].AsCharPtr(), "\\String2" );
	if ( any.SlotName(3L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[4L].GetType() == AnyCharPtrType );
	t_assert( any[4L].GetSize() == 1 );
	assertCharPtrEqual( any[4L].AsCharPtr(), "/String3\"" );//"
	if ( any.SlotName(4L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[5L].GetType() == AnyCharPtrType );
	t_assert( any[5L].GetSize() == 1 );
	assertCharPtrEqual( any[5L].AsCharPtr(), "/String4" );
	if ( any.SlotName(5L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[6L].GetType() == AnyCharPtrType );
	t_assert( any[6L].GetSize() == 1 );
	assertCharPtrEqual( any[6L].AsCharPtr(), "\\" );
	if ( any.SlotName(6L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}

	t_assert( any[7L].GetType() == AnyCharPtrType );
	t_assert( any[7L].GetSize() == 1 );
	assertCharPtrEqual( any[7L].AsCharPtr(), "/" );
	if ( any.SlotName(7L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic19

void AnythingParserSemanticTest::Semantic20Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );

	char testString2[10] = "\"\\GH\"";
	assertCharPtrEqual( any[1L].AsCharPtr(), testString2 );
	//assertCharPtrEqual( any[1L].AsCharPtr(), "\"\\GH\"" );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
	}
}
// testSemantic20

void AnythingParserSemanticTest::Semantic21Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic22Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic23Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
					t_assert( false );
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
					t_assert( false );
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
					t_assert( false );
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
					t_assert( false );
				}
			}
			break;
		}
	}
}
// testSemantic23

void AnythingParserSemanticTest::Semantic24Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}

	t_assert( any[1L].GetType() == AnyCharPtrType );
	t_assert( any[1L].GetSize() == 1 );
	assertCharPtrEqual( any[1L].AsCharPtr(), " String" );
	if ( any.SlotName(1L) != NULL ) {
		// SlotName should be NULL but it is not:  errMsg
		t_assert( false );
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

void AnythingParserSemanticTest::Semantic25Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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
		t_assert( false );
	}
}
// testSemantic25

void AnythingParserSemanticTest::Semantic26Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload( any );
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

void AnythingParserSemanticTest::Semantic27Test() {
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

void AnythingParserSemanticTest::testSemantic28Prep( Anything *any ) {
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
}
//	testSemantic28Prep

void AnythingParserSemanticTest::Semantic28Test() {
	// Init an anything-object for testing
	Anything *any;
	any = new Anything();

	AnythingParserSemanticTest::testSemantic28Prep( any );

	delete any;
}
//	testSemantic28

void AnythingParserSemanticTest::testSemantic29Prep( Anything *any ) {
	Anything anyHlp0, anyHlp1;

	anyHlp0["a"] = 0L;
	anyHlp1["b"] = 1L;
	any->Append( anyHlp0 );
	any->Append( anyHlp1 );

	anyHlp0["a"] = 2L;
	anyHlp1["b"] = 3L;
}
//	testSemantic29Prep

void AnythingParserSemanticTest::Semantic29Test() {
	// Init an anything-object for testing
	Anything *any;
	any = new Anything();

	AnythingParserSemanticTest::testSemantic29Prep( any );

	delete any;
}
//	testSemantic29

void AnythingParserSemanticTest::testSemantic30Prep( Anything &any ) {
	Anything anyHlp0, anyHlp1;

	anyHlp0["a"] = 0L;
	anyHlp1["b"] = 1L;
	any.Append( anyHlp0 );
	any.Append( anyHlp1 );

	anyHlp0["a"] = 2L;
	anyHlp1["b"] = 3L;
}
//	testSemantic30Prep

void AnythingParserSemanticTest::Semantic30Test() {
	// Init an anything-object for testing
	Anything any;

	AnythingParserSemanticTest::testSemantic30Prep( any );
}
//	testSemantic30

void AnythingParserSemanticTest::Semantic31Test() {
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

void AnythingParserSemanticTest::Semantic32Test() {
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

void AnythingParserSemanticTest::Semantic33Test() {
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

void AnythingParserSemanticTest::slashSlotnames()
/* what: test Anything parsing with slashes in slotnames
*/
{
	{
		String buf("{ /\"Test/Slot\" 12 }");
		IStringStream is(buf);
		Anything any;

		is >> any;
		assertEqual("Test/Slot", any.SlotName(0));
		assertEqual(12, any["Test/Slot"].AsLong(-1));
	}
	{
		String buf("{ /Test/Slot 12 }");
		// a slash in a slotname terminates the slot with a null any as value
		// (an error message is written)
		IStringStream is(buf);
		Anything any;

		is >> any;
		assertEqual("Test", any.SlotName(0));
		t_assert(any["Test"].IsNull());
	}
	{
		String buf("{ //Test 12 }");
		// a slash at the beginning is ignored (but generates an error message)
		IStringStream is(buf);
		Anything any;

		is >> any;
		assertEqual("Test", any.SlotName(0));
		assertEqual(12, any["Test"].AsLong(-1));	// contains an empty any
	}
} // slashSlotnames

// testquotedslotname PS 7.6.99
void AnythingParserSemanticTest::QuotedSlotnameTest() {
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
