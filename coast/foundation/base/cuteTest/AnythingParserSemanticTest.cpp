/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingParserSemanticTest.h"
#include "StringStream.h"
#include "SystemFile.h"
using namespace coast;

AnythingParserSemanticTest::AnythingParserSemanticTest() {
	Anything undefAny;
	emptyAny = undefAny;
	anyTemp0 = undefAny;
	anyTemp1 = undefAny;
	anyTemp2 = undefAny;
	anyTemp3 = undefAny;
	anyTemp4 = undefAny;
}

void AnythingParserSemanticTest::runAllTests(cute::suite &s) {
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic0Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic1Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic2Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic3Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic4Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic5Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic6Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic7Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic8Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic9Test));

	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic10Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic11Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic12Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic13Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic14Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic15Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic16Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic17Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic18Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic19Test));

	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic20Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic21Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic22Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic23Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic24Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic25Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic26Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic27Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic28Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic29Test));

	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic30Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic31Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic32Test));
	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, Semantic33Test));

	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, QuotedSlotnameTest));

	s.push_back(CUTE_SMEMFUN(AnythingParserSemanticTest, slashSlotnames));
}

/*==================================================================================*/
/*                         Hilfsmethoden fuer Tests                                 */
/*==================================================================================*/
void AnythingParserSemanticTest::checkImportExport(Anything any, String fileName) {
	String buf1;
	String buf2;
	Anything anyHlp;

	OStringStream os1(&buf1);
	any.Export(os1);
	IStringStream is1(buf1);
	anyHlp.Import(is1);
	OStringStream os2(&buf2);
	anyHlp.Export(os2);
	ASSERT_EQUAL( buf1, buf2 );
	if (buf1 != buf2) {
	}
}

void AnythingParserSemanticTest::writeResult(String *input, long nrOfElt, char *path, char *ext) {
	std::ostream *os = system::OpenOStream(path, ext, std::ios::trunc);
	if (os) {
		Anything emptyAny1, anyTest, anyTests;

		AnythingParserSemanticTest::anyOutput = emptyAny1;
		AnythingParserSemanticTest::lineCounter = 1;

		long i;
		for (i = 0; i < nrOfElt; i++) {
			IStringStream is(input[i]);
			anyTest.Import(is);
			anyTests.Append(anyTest);
		}
		scanAnything(anyTests);

		*os << AnythingParserSemanticTest::anyOutput;
		delete os;

		AnythingParserSemanticTest::anyOutput = emptyAny1;
		AnythingParserSemanticTest::lineCounter = 1;
	} else {
		String tmp0 = "write ";
		tmp0.Append(path);
		tmp0.Append(".");
		tmp0.Append(ext);
		String tmp1 = "could not write ";
		tmp1.Append(path);
		tmp1.Append(".");
		tmp1.Append(ext);
		ASSERT_EQUAL( (const char *)tmp0, (const char *)tmp1 );
	}
}

void AnythingParserSemanticTest::scanAnything(Anything any0) {
	long i, iMax = any0.GetSize();
	String slotNm;
	Anything anyTest;
	String buffer;

	for (i = 0; i < iMax; i++) {
		anyTest = any0[i];

		slotNm = "";
		slotNm.Append(AnythingParserSemanticTest::lineCounter++);
		slotNm.Append(") ");
		slotNm.Append(any0.SlotName(i));
		slotNm.Append(" Size:");
		slotNm.Append(anyTest.GetSize());
		slotNm.Append("; Type:");

		switch (anyTest.GetType()) {
			case AnyLongType: {
				slotNm.Append("eLong");
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = anyTest.AsLong();
			}
				break;

			case AnyDoubleType: {
				slotNm.Append("eDouble");
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = anyTest.AsDouble();
			}
				break;

			case AnyNullType: {
				slotNm.Append("eNull");
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = "NULL";
			}
				break;

			case AnyCharPtrType: {
				slotNm.Append("eCharPtr");
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = anyTest.AsCharPtr();
			}
				break;

			case AnyArrayType: {
				slotNm.Append("eArray");
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = "...";
				AnythingParserSemanticTest::scanAnything(anyTest);
			}
				break;

			case AnyVoidBufType: {
				slotNm.Append("eVoidBuf");
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = anyTest.AsCharPtr();
			}
				break;

			case AnyObjectType: {
				slotNm.Append("eObject");
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = anyTest;
			}
				break;

			default: {
				String str("???");//lint !e585
				slotNm.Append("???");//lint !e585
				slotNm.Append("; Value: ");
				AnythingParserSemanticTest::anyOutput[(const char *) slotNm] = str;//"???";//String("???");
			}
				break;
		}
	}
}

Anything AnythingParserSemanticTest::storeAndReload(Anything any) {
	// Store and reload 'any'
	String buf;
	OStringStream os(&buf);
	any.Export(os);
	IStringStream is(buf);
	Anything anyResult;
	anyResult.Import(is);
	return (anyResult);
}

void AnythingParserSemanticTest::Semantic0Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any.Append('\01'); //	0
	any.Append("\01"); //	1
	any.Append("\\\\"); //	2
	any.Append("\\\\x41"); //	3
	any.Append("\x41"); //	4		\x41 = "A"
	any["{}"] = 1; //	5
	any["SlotName0"] = emptyAny; //	6
	any["SlotName1"] = "{}"; //	7
	any["SlotName2"] = "\"\"{}\"\""; //	8

	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	ASSERT( any[0L].GetType() == AnyLongType );
	ASSERT( any[0L].AsLong() == 1L );
	ASSERT( any[0L] == '\x01' );
	if (any.SlotName(0L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	ASSERT( any[1L].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any[1L].AsCharPtr(), "\x01" );
	if (any.SlotName(1L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}

	ASSERT( any[2L].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any[2L].AsCharPtr(), "\\\\" );
	if (any.SlotName(2L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	ASSERT( any[3L].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any[3L].AsCharPtr(), "\\\\x41" );
	if (any.SlotName(3L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	ASSERT( any[4L].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any[4L].AsCharPtr(), "\x41" );
	if (any.SlotName(4L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	ASSERT( any["{}"].GetType() == AnyLongType );
	ASSERT( any["{}"].AsLong() == 1L );
	ASSERT( any[5L].GetType() == AnyLongType );
	ASSERT( any[5L].AsLong() == 1L );
	ASSERT_EQUAL( any.SlotName(5L), "{}" );
	ASSERT( any.FindIndex("{}") == 5 );

	ASSERT( any["SlotName0"].GetType() == AnyNullType );
	ASSERT( any[6L].GetType() == AnyNullType );
	ASSERT_EQUAL( any.SlotName(6L), "SlotName0" );
	ASSERT( any.FindIndex("SlotName0") == 6 );

	ASSERT( any["SlotName1"].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any["SlotName1"].AsCharPtr(), "{}" );
	ASSERT( any[7L].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any[7L].AsCharPtr(), "{}" );
	ASSERT_EQUAL( any.SlotName(7L), "SlotName1" );
	ASSERT( any.FindIndex("SlotName1") == 7 );

	ASSERT( any["SlotName2"].GetType() == AnyCharPtrType );

	char testString[10] = "\"\"{}\"\"";
	ASSERT_EQUAL( any["SlotName2"].AsCharPtr(), testString ); // dummy
	ASSERT( any[8L].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any[8L].AsCharPtr(), testString );
	ASSERT_EQUAL( any.SlotName(8L), "SlotName2" );
	ASSERT( any.FindIndex("SlotName2") == 8 );
}
// testSemantic0

void AnythingParserSemanticTest::Semantic1Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["SlotName0"] = "\"\""; //	0
	any.Append(emptyAny); //	1
	any.Append("\"\""); //	2
	any["#ab"] = 3; //	3
	any["a#b"] = 4; //	4
	any["ab#"] = 5; //	5 {
	any["}ab"] = 6; //	6 {
	any["a}b"] = 7; //	7 {
	any["ab}"] = 8; //	8
	any["{ab"] = 9; //	9 }
	any["a{b"] = 10; //	10 }
	any["ab{"] = 11; //	11 }

	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	ASSERT( any.GetType() == AnyArrayType );
	ASSERT( any.GetSize() == 12 );
	ASSERT( any["SlotName0"].GetType() == AnyCharPtrType );
	ASSERT( any[0L].GetType() == AnyCharPtrType );

	char testString[10] = "\"\"";
	ASSERT_EQUAL( any["SlotName0"].AsCharPtr(), testString );
	ASSERT_EQUAL( any[0L].AsCharPtr(), testString );
	//ASSERT_EQUAL( any["SlotName0"].AsCharPtr(), "\"\"" );
	//ASSERT_EQUAL( any[0L].AsCharPtr(), "\"\"" );
	ASSERT_EQUAL( any.SlotName(0L), "SlotName0" );
	ASSERT( any.FindIndex("SlotName0") == 0 );

	ASSERT( any[1L].GetType() == AnyNullType );
	if (any.SlotName(1L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}

	ASSERT( any[2L].GetType() == AnyCharPtrType );
	ASSERT_EQUAL( any[2L].AsCharPtr(), testString );
	//ASSERT_EQUAL( any[2L].AsCharPtr(), "\"\"" );
	if (any.SlotName(2L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}

	ASSERT( any["#ab"].GetType() == AnyLongType );
	ASSERT( any["#ab"].AsLong() == 3 );
	ASSERT( any[3L].GetType() == AnyLongType );
	ASSERT( any[3L].AsLong() == 3 );
	ASSERT_EQUAL( any.SlotName(3L), "#ab" );
	ASSERT( any.FindIndex("#ab") == 3 );

	ASSERT( any["a#b"].GetType() == AnyLongType );
	ASSERT( any["a#b"].AsLong() == 4 );
	ASSERT( any[4L].GetType() == AnyLongType );
	ASSERT( any[4L].AsLong() == 4 );
	ASSERT_EQUAL( any.SlotName(4L), "a#b" );
	ASSERT( any.FindIndex("a#b") == 4 );

	ASSERT( any["ab#"].GetType() == AnyLongType );
	ASSERT( any["ab#"].AsLong() == 5 );
	ASSERT( any[5L].GetType() == AnyLongType );
	ASSERT( any[5L].AsLong() == 5 );
	ASSERT_EQUAL( any.SlotName(5L), "ab#" );
	ASSERT( any.FindIndex("ab#") == 5 );
	// {
	ASSERT( any["}ab"].GetType() == AnyLongType ); // {
	ASSERT( any["}ab"].AsLong() == 6 );
	ASSERT( any[6L].GetType() == AnyLongType );
	ASSERT( any[6L].AsLong() == 6 ); // {
	ASSERT_EQUAL( any.SlotName(6L), "}ab" ); // {
	ASSERT( any.FindIndex("}ab") == 6 );
	// {
	ASSERT( any["a}b"].GetType() == AnyLongType ); // {
	ASSERT( any["a}b"].AsLong() == 7 );
	ASSERT( any[7L].GetType() == AnyLongType );
	ASSERT( any[7L].AsLong() == 7 ); // {
	ASSERT_EQUAL( any.SlotName(7L), "a}b" ); // {
	ASSERT( any.FindIndex("a}b") == 7 );
	// {
	ASSERT( any["ab}"].GetType() == AnyLongType ); // {
	ASSERT( any["ab}"].AsLong() == 8 );
	ASSERT( any[8L].GetType() == AnyLongType );
	ASSERT( any[8L].AsLong() == 8 ); // {
	ASSERT_EQUAL( any.SlotName(8L), "ab}" ); // {
	ASSERT( any.FindIndex("ab}") == 8 );

	ASSERT( any["{ab"].GetType() == AnyLongType );// }
	ASSERT( any["{ab"].AsLong() == 9 );// }
	ASSERT( any[9L].GetType() == AnyLongType );
	ASSERT( any[9L].AsLong() == 9 );
	ASSERT_EQUAL( any.SlotName(9L), "{ab" );// }
	ASSERT( any.FindIndex("{ab") == 9 );// }

	ASSERT( any["a{b"].GetType() == AnyLongType );// }
	ASSERT( any["a{b"].AsLong() == 10 );// }
	ASSERT( any[10L].GetType() == AnyLongType );
	ASSERT( any[10L].AsLong() == 10 );
	ASSERT_EQUAL( any.SlotName(10L), "a{b" );// }
	ASSERT( any.FindIndex("a{b") == 10 );// }

	ASSERT( any["ab{"].GetType() == AnyLongType );// }
	ASSERT( any["ab{"].AsLong() == 11 );// }
	ASSERT( any[11L].GetType() == AnyLongType );
	ASSERT( any[11L].AsLong() == 11 );
	ASSERT_EQUAL( any.SlotName(11L), "ab{" );// }
	ASSERT( any.FindIndex("ab{") == 11 );// }
}
// testSemantic1

void AnythingParserSemanticTest::Semantic2Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	ASSERT( any.GetType() == AnyArrayType );
	ASSERT( any.GetSize() == 2 );
	ASSERT( any["a"].GetType() == AnyArrayType );
	ASSERT( any["a"].GetSize() == 2 );
	ASSERT( any[0L].GetType() == AnyArrayType );
	ASSERT( any[0L].GetSize() == 2 );
	ASSERT_EQUAL( AnyCharPtrType, any["a"][0L].GetType() );
	ASSERT_EQUAL( any["a"][0L].AsCharPtr(), "b" );
	ASSERT( any["a"][1L].GetType() == AnyLongType );
	ASSERT( any["a"][1L].AsLong() == 1 );
	ASSERT_EQUAL( AnyCharPtrType, any[0L][0L].GetType() );
	ASSERT_EQUAL( any[0L][0L].AsCharPtr(), "b" );
	ASSERT( any[0L][1L].GetType() == AnyLongType );
	ASSERT( any[0L][1L].AsLong() == 1 );
	ASSERT( any[1L].GetType() == AnyLongType );
	ASSERT( any[1L].AsLong() == 9 );
	ASSERT_EQUAL( any.SlotName(0L), "a" );
	ASSERT( any.FindIndex("a") == 0 );
	if (any["a"].SlotName(0L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	if (any["a"].SlotName(1L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	if (any[0L].SlotName(0L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	if (any[0L].SlotName(1L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	if (any.SlotName(1L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}

}
// testSemantic2

void AnythingParserSemanticTest::Semantic3Test() {
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
	Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	ASSERT( any.GetType() == AnyArrayType );
	ASSERT( any.GetSize() == 2 );
	ASSERT( any["ab"].GetType() == AnyArrayType );
	ASSERT( any["ab"].GetSize() == 1 );
	ASSERT( any[0L].GetType() == AnyArrayType );
	ASSERT( any[0L].GetSize() == 1 );
	ASSERT( any["ab"][0L].GetType() == AnyLongType );
	ASSERT( any["ab"][0L].AsLong() == 2 );
	ASSERT( any[0L][0L].GetType() == AnyLongType );
	ASSERT( any[0L][0L].AsLong() == 2 );
	ASSERT( any[1L].GetType() == AnyLongType );
	ASSERT( any[1L].AsLong() == 0 );
	ASSERT_EQUAL( any.SlotName(0L), "ab" );
	ASSERT( any.FindIndex("ab") == 0 );
	if (any["ab"].SlotName(0L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	if (any[0L].SlotName(0L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	if (any.SlotName(1L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
}
// testSemantic3

void AnythingParserSemanticTest::Semantic4Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;//{
	anyTemp0.Append("2}6");
	any["ab"] = anyTemp0;
	//	{ Anything-Object filled with Stream =
	//	{
	//	  /ab {
	//		"2}6"
	//	  }
	//	}

	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	ASSERT( any.GetType() == AnyArrayType );
	ASSERT( any.GetSize() == 1 );
	ASSERT( any["ab"].GetType() == AnyArrayType );
	ASSERT( any["ab"].GetSize() == 1 );
	ASSERT( any[0L].GetType() == AnyArrayType );
	ASSERT( any[0L].GetSize() == 1 );
	ASSERT( any["ab"][0L].GetType() == AnyCharPtrType );//{
	ASSERT_EQUAL( any["ab"][0L].AsCharPtr(), "2}6" );
	ASSERT( any[0L][0L].GetType() == AnyCharPtrType );//{
	ASSERT_EQUAL( any[0L][0L].AsCharPtr(), "2}6" );
	ASSERT_EQUAL( any.SlotName(0L), "ab" );
	ASSERT( any.FindIndex("ab") == 0 );
	if (any["ab"].SlotName(0L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
	if (any[0L].SlotName(0L) != NULL) {
		// SlotName should be NULL but it is not:  errMsg
		ASSERT( false );
	}
}
// testSemantic4

void AnythingParserSemanticTest::Semantic5Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any["{ab"] = "2}7";
	//	Anything-Object filled with Stream =
	//	{
	//  	/"{ab" "2}7"
	//	}

	// Store and reload 'any'
	Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
	any = emptyAny;
	any = any1;

	// Check if the anything-object is the expected one
	ASSERT( any.GetType() == AnyArrayType );
	ASSERT( any.GetSize() == 1 );
	ASSERT( any["{ab"].GetType() == AnyCharPtrType );// }
	ASSERT( any["{ab"].GetSize() == 1 );// }
	ASSERT( any[0L].GetType() == AnyCharPtrType );
	ASSERT( any[0L].GetSize() == 1 );
	ASSERT_EQUAL( any["{ab"].AsCharPtr(), "2}7" );//{
	ASSERT_EQUAL( any[0L].AsCharPtr(), "2}7" );
	ASSERT_EQUAL( any.SlotName(0L), "{ab" );// }
	ASSERT( any.FindIndex("{ab") == 0 );// }
}
// testSemantic5

void AnythingParserSemanticTest::Semantic6Test() {
	// Init an anything-object for testing
	Anything any = emptyAny;
	any[_QUOTE_(a{b)] = _QUOTE_(2}8);
		//	Anything-Object filled with Stream =
		//	{
		//  	/"a{b" "2}8"
		//	}

		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 1 );
		ASSERT( any["a{b"].GetType() == AnyCharPtrType );// }
		ASSERT( any["a{b"].GetSize() == 1 );// }
		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );
		ASSERT_EQUAL( any["a{b"].AsCharPtr(), "2}8" );//{
		ASSERT_EQUAL( any[0L].AsCharPtr(), "2}8" );
		ASSERT_EQUAL( any.SlotName(0L), "a{b" );// }
		ASSERT( any.FindIndex("a{b") == 0 );// }
	}
	// testSemantic6

	void AnythingParserSemanticTest::Semantic7Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		any[_QUOTE_(ab{)] = _QUOTE_(2}9);
			//	Anything-Object filled with Stream =
			//	{
			//  	/"ab{" "2}9"
			//	}

			// Store and reload 'any'
			Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
			any = emptyAny;
			any = any1;

			// Check if the anything-object is the expected one
			ASSERT( any.GetType() == AnyArrayType );
			ASSERT( any.GetSize() == 1 );
			ASSERT( any["ab{"].GetType() == AnyCharPtrType );// }
			ASSERT( any["ab{"].GetSize() == 1 );
			ASSERT( any[0L].GetType() == AnyCharPtrType );
			ASSERT( any[0L].GetSize() == 1 );
			ASSERT_EQUAL( any["ab{"].AsCharPtr(), "2}9" );
			ASSERT_EQUAL( any[0L].AsCharPtr(), "2}9" );
			ASSERT_EQUAL( any.SlotName(0L), "ab{" );// }
			ASSERT( any.FindIndex("ab{") == 0 );// }
		}
		// testSemantic7

		void AnythingParserSemanticTest::Semantic8Test() {
			// Init an anything-object for testing
			Anything any = emptyAny;
			any["\"Slot\"Name\""] = 33; //	0
			any["Slot\"Name\""] = 34; //	1
			any.Append("/Slot\"Name"); //	2
			any.Append(35); //	3
			//	Anything-Object filled with Stream =
			//	{
			//  	/"Slot\"Name" 33
			//		/Slot\"Name"  34
			//		"/Slot\"Name"
			//		35
			//	}

			// Store and reload 'any'
			Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
			any = emptyAny;
			any = any1;

			// Check if the anything-object is the expected one
			ASSERT( any.GetType() == AnyArrayType );
			ASSERT( any.GetSize() == 4 );
			String slot("\"Slot\"Name\"");
			ASSERT( any[slot].GetType() == AnyLongType );
			ASSERT( any[slot].GetSize() == 1 );
			ASSERT( any[0L].GetType() == AnyLongType );
			ASSERT( any[0L].GetSize() == 1 );
			ASSERT( any[slot].AsLong() == 33 );
			ASSERT( any[0L].AsLong() == 33 );
			ASSERT_EQUAL( any.SlotName(0L), slot );
			ASSERT( any.FindIndex(slot) == 0 );
			String slot2("Slot\"Name\"");
			ASSERT( any[slot2].GetType() == AnyLongType );
			ASSERT( any[slot2].GetSize() == 1 );
			ASSERT( any[1L].GetType() == AnyLongType );
			ASSERT( any[1L].GetSize() == 1 );
			ASSERT( any[slot2].AsLong() == 34 );
			ASSERT( any[1L].AsLong() == 34 );
			ASSERT_EQUAL( any.SlotName(1L), slot2 );

			ASSERT( any.FindIndex(slot2) == 1L );

			String slot3("/Slot\"Name");
			ASSERT( any[2L].GetType() == AnyCharPtrType );
			ASSERT( any[2L].GetSize() == 1 );
			ASSERT_EQUAL( any[2L].AsCharPtr(), slot3 );
			if (any.SlotName(2L) != NULL) {
				// SlotName should be NULL but it is not:  errMsg
				ASSERT( false );
			}
			ASSERT( any[3L].GetType() == AnyLongType );
			ASSERT( any[3L].GetSize() == 1 );
			ASSERT( any[3L].AsLong() == 35 );
			if (any.SlotName(3L) != NULL) {
				// SlotName should be NULL but it is not:  errMsg
				ASSERT( false );
			}
		}
		// testSemantic8

		void AnythingParserSemanticTest::Semantic9Test() {
			// Init an anything-object for testing
			Anything any = emptyAny;
			any["/ab"] = 9L; //	0
			any["a/b"] = 10L; //	1
			any["ab/"] = 11L; //	2
			any["0"] = "d"; //	3

			// Anything-Object filled with Stream =
			//{
			//  /"/ab" 9
			//  /"a/b" 10
			//  /"ab/" 11
			//	/"0"   "d"
			//}

			// Store and reload 'any'
			Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
			any = emptyAny;
			any = any1;

			// Check if the anything-object is the expected one
			ASSERT( any.GetType() == AnyArrayType );
			ASSERT( any.GetSize() == 4 );
			ASSERT( any["/ab"].GetType() == AnyLongType );
			ASSERT( any["/ab"].GetSize() == 1 );
			ASSERT( any[0L].GetType() == AnyLongType );
			ASSERT( any[0L].GetSize() == 1 );
			ASSERT( any["/ab"].AsLong() == 9 );
			ASSERT( any[0L].AsLong() == 9 );
			ASSERT_EQUAL( any.SlotName(0L), "/ab" );
			ASSERT( any.FindIndex("/ab") == 0 );

			ASSERT( any["a/b"].GetType() == AnyLongType );
			ASSERT( any["a/b"].GetSize() == 1 );
			ASSERT( any[1L].GetType() == AnyLongType );
			ASSERT( any[1L].GetSize() == 1 );
			ASSERT( any["a/b"].AsLong() == 10 );
			ASSERT( any[1L].AsLong() == 10 );
			ASSERT_EQUAL( any.SlotName(1L), "a/b" );
			ASSERT( any.FindIndex("a/b") == 1 );

			ASSERT( any["ab/"].GetType() == AnyLongType );
			ASSERT( any["ab/"].GetSize() == 1 );
			ASSERT( any[2L].GetType() == AnyLongType );
			ASSERT( any[2L].GetSize() == 1 );
			ASSERT( any["ab/"].AsLong() == 11 );
			ASSERT( any[2L].AsLong() == 11 );
			ASSERT_EQUAL( any.SlotName(2L), "ab/" );
			ASSERT( any.FindIndex("ab/") == 2 );

			ASSERT( any["0"].GetType() == AnyCharPtrType );
			ASSERT( any["0"].GetSize() == 1 );
			ASSERT( any[3L].GetType() == AnyCharPtrType );
			ASSERT( any[3L].GetSize() == 1 );
			ASSERT_EQUAL( any["0"].AsCharPtr(), "d" );
			ASSERT_EQUAL( any[3L].AsCharPtr(), "d" );
			ASSERT_EQUAL( any.SlotName(3L), "0" );
			ASSERT( any.FindIndex("0") == 3 );

			// Test if anything-objects can change correctly
			any["ab/"] = "2/9"; //	2
			any["a/b"] = "2/8"; //	1
			any["/ab"] = "2/7"; //	0

			// Anything-Object filled with Stream =
			//{
			//  /"/ab" "2/7"
			//  /"a/b" "2/8"
			//  /"ab/" "2/9"
			//}
			ASSERT( any["/ab"].GetType() == AnyCharPtrType );
			ASSERT( any["/ab"].GetSize() == 1 );
			ASSERT( any[0L].GetType() == AnyCharPtrType );
			ASSERT( any[0L].GetSize() == 1 );
			ASSERT_EQUAL( any["/ab"].AsCharPtr(), "2/7");
			ASSERT_EQUAL( any[0L].AsCharPtr(), "2/7" );
			ASSERT_EQUAL( any.SlotName(0L), "/ab" );
			ASSERT( any.FindIndex("/ab") == 0 );

			ASSERT( any["a/b"].GetType() == AnyCharPtrType );
			ASSERT( any["a/b"].GetSize() == 1 );
			ASSERT( any[1L].GetType() == AnyCharPtrType );
			ASSERT( any[1L].GetSize() == 1 );
			ASSERT_EQUAL( any["a/b"].AsCharPtr(), "2/8");
			ASSERT_EQUAL( any[1L].AsCharPtr(), "2/8" );
			ASSERT_EQUAL( any.SlotName(1L), "a/b" );
			ASSERT( any.FindIndex("a/b") == 1 );

			ASSERT( any["ab/"].GetType() == AnyCharPtrType );
			ASSERT( any["ab/"].GetSize() == 1 );
			ASSERT( any[2L].GetType() == AnyCharPtrType );
			ASSERT( any[2L].GetSize() == 1 );
			ASSERT_EQUAL( any["ab/"].AsCharPtr(), "2/9");
			ASSERT_EQUAL( any[2L].AsCharPtr(), "2/9" );
			ASSERT_EQUAL( any.SlotName(2L), "ab/" );
			ASSERT( any.FindIndex("ab/") == 2 );

			ASSERT( any["0"].GetType() == AnyCharPtrType );
			ASSERT( any["0"].GetSize() == 1 );
			ASSERT( any[3L].GetType() == AnyCharPtrType );
			ASSERT( any[3L].GetSize() == 1 );
			ASSERT_EQUAL( any["0"].AsCharPtr(), "d" );
			ASSERT_EQUAL( any[3L].AsCharPtr(), "d" );
			ASSERT_EQUAL( any.SlotName(3L), "0" );
			ASSERT( any.FindIndex("0") == 3 );
		}
		// testSemantic9

		void AnythingParserSemanticTest::Semantic10Test() {
			// Init an anything-object for testing
			Anything any = emptyAny;
			any["a"] = emptyAny; //	0
			any["a{"] = "}"; //	1
			any["{a"] = "}"; //	2 {
			any["}"] = "b"; //	3
			any["{"] = "c"; //	4 }
			// Anything-Object filled with Stream =
			//{
			//  /a 		*
			//  /"a{"	"}"
			//	/"{a"	"}"
			//  /"}"	"b"
			//	/"{"	"c"
			//}
			// Store and reload 'any'
			Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
			any = emptyAny;
			any = any1;

			// Check if the anything-object is the expected one
			ASSERT( any["a"].GetType() == AnyNullType );
			ASSERT( any["a"].GetSize() == 0 );
			ASSERT( any[0L].GetType() == AnyNullType );
			ASSERT( any[0L].GetSize() == 0 );
			ASSERT_EQUAL( any.SlotName(0L), "a" );
			ASSERT( any.FindIndex("a") == 0L );

			ASSERT( any["a{"].GetType() == AnyCharPtrType );// }
			ASSERT( any["a{"].GetSize() == 1 );// }
			ASSERT( any[1L].GetType() == AnyCharPtrType );
			ASSERT( any[1L].GetSize() == 1 );
			ASSERT_EQUAL( any["a{"].AsCharPtr(), "}" );//{
			ASSERT_EQUAL( any[1L].AsCharPtr(), "}" );
			ASSERT_EQUAL( any.SlotName(1L), "a{" );// }
			ASSERT( any.FindIndex("a{") == 1L );// }

			ASSERT( any["{a"].GetType() == AnyCharPtrType );// }
			ASSERT( any["{a"].GetSize() == 1 );// }
			ASSERT( any[2L].GetType() == AnyCharPtrType );
			ASSERT( any[2L].GetSize() == 1 );
			ASSERT_EQUAL( any["{a"].AsCharPtr(), "}" );//{
			ASSERT_EQUAL( any[2L].AsCharPtr(), "}" );
			ASSERT_EQUAL( any.SlotName(2L), "{a" );// }
			ASSERT( any.FindIndex("{a") == 2L );// }
			//{
			ASSERT( any["}"].GetType() == AnyCharPtrType );//{
			ASSERT( any["}"].GetSize() == 1 );
			ASSERT( any[3L].GetType() == AnyCharPtrType );
			ASSERT( any[3L].GetSize() == 1 );//{
			ASSERT_EQUAL( any[_QUOTE_(})].AsCharPtr(), "b" );
		ASSERT_EQUAL( any[3L].AsCharPtr(), "b" );//{
		ASSERT_EQUAL( any.SlotName(3L), "}" );//{
		ASSERT( any.FindIndex("}") == 3L );

		ASSERT( any["{"].GetType() == AnyCharPtrType );// }
		ASSERT( any["{"].GetSize() == 1 );// }
		ASSERT( any[4L].GetType() == AnyCharPtrType );
		ASSERT( any[4L].GetSize() == 1 );
		ASSERT_EQUAL( any["{"].AsCharPtr(), "c" );// }
		ASSERT_EQUAL( any[4L].AsCharPtr(), "c" );
		ASSERT_EQUAL( any.SlotName(4L), "{" );// }
		ASSERT( any.FindIndex("{") == 4L );// }
	}
	// testSemantic10

	void AnythingParserSemanticTest::Semantic11Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		any["\"Slo{t}Name\""] = "\"a\\\"{}c\""; //	0 {
		any["\"Slot}Name\""] = "\"a\\\"{}c\""; //	1

		// {Anything-Object filled with Stream =
		//{
		//  /"Slo{t}Name" "a\"{}c"
		//  /"Slot}Name" "a\"{}c"
		//}

		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		String slot("\"Slo{t}Name\"");
		ASSERT( any[slot].GetType() == AnyCharPtrType );
		ASSERT( any[slot].GetSize() == 1 );
		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );

		char testString[10] = "\"a\\\"{}c\"";
		ASSERT_EQUAL( any[slot].AsCharPtr(), testString );
		ASSERT_EQUAL( any[0L].AsCharPtr(), testString );
		ASSERT_EQUAL( any.SlotName(0L), slot );
		ASSERT( any.FindIndex(slot) == 0L ); //{
		String slot2("\"Slot}Name\"");
		ASSERT( any[slot2].GetType() == AnyCharPtrType );
		ASSERT( any[slot2].GetSize() == 1 );
		ASSERT( any[1L].GetType() == AnyCharPtrType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT_EQUAL( any[slot2].AsCharPtr(), testString );
		ASSERT_EQUAL( any[1L].AsCharPtr(), testString );
		ASSERT_EQUAL( any.SlotName(1L), slot2 );
		ASSERT( any.FindIndex(slot2) == 1L );
	}
	// testSemantic11

	void AnythingParserSemanticTest::Semantic12Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		anyTemp0.Append("a");
		any.Append(emptyAny);
		any["Test155"] = anyTemp0;
		// Anything-Object filled with Stream =
		//{
		//  *
		//  /Test155 {
		//    "a"
		//  }
		//}
		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any[0L].GetType() == AnyNullType );
		ASSERT( any[0L].GetSize() == 0 );
		if (any.SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any["Test155"].GetType() == AnyArrayType );
		ASSERT( any["Test155"].GetSize() == 1 );
		ASSERT( any[1L].GetType() == AnyArrayType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT_EQUAL( any["Test155"][0L].AsCharPtr(), "a" );
		ASSERT_EQUAL( any[1L][0L].AsCharPtr(), "a" );
		ASSERT_EQUAL( any.SlotName(1L), "Test155" );
		if (any["Test155"].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}
		if (any[1L].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any[0L].GetType() == AnyNullType );
		ASSERT( any[0L].GetSize() == 0 );
		ASSERT( any["SlotName"].GetType() == AnyNullType );
		ASSERT( any["SlotName"].GetSize() == 0 );
		ASSERT_EQUAL( any.SlotName(0L), "SlotName" );

		ASSERT( any["Test155"].GetType() == AnyArrayType );
		ASSERT( any["Test155"].GetSize() == 1 );
		ASSERT( any[1L].GetType() == AnyArrayType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT_EQUAL( any["Test155"][0L].AsCharPtr(), "a" );
		ASSERT_EQUAL( any[1L][0L].AsCharPtr(), "a" );
		ASSERT_EQUAL( any.SlotName(1L), "Test155" );
		if (any["Test155"].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}
		if (any[1L].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}
	}
	// testSemantic13

	void AnythingParserSemanticTest::Semantic14Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		any.Append(emptyAny);
		any.Append(emptyAny);
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any[0L].GetType() == AnyNullType );
		ASSERT( any[0L].GetSize() == 0 );
		if (any.SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[1L].GetType() == AnyNullType );
		ASSERT( any[1L].GetSize() == 0 );
		if (any.SlotName(1L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[2L].GetType() == AnyNullType );
		ASSERT( any[2L].GetSize() == 0 );
		ASSERT( any["a"].GetType() == AnyNullType );
		ASSERT( any["a"].GetSize() == 0 );
		ASSERT_EQUAL( any.SlotName(2L), "a" );
		ASSERT( any.FindIndex("a") == 2L );

		ASSERT( any[3L].GetType() == AnyNullType );
		ASSERT( any[3L].GetSize() == 0 );
		ASSERT( any["b"].GetType() == AnyNullType );
		ASSERT( any["b"].GetSize() == 0 );
		ASSERT_EQUAL( any.SlotName(3L), "b" );
		ASSERT( any.FindIndex("b") == 3L );

		ASSERT( any[4L].GetType() == AnyNullType );
		ASSERT( any[4L].GetSize() == 0 );
		ASSERT( any["c"].GetType() == AnyNullType );
		ASSERT( any["c"].GetSize() == 0 );
		ASSERT_EQUAL( any.SlotName(4L), "c" );
		ASSERT( any.FindIndex("c") == 4L );
	}
	// testSemantic14

	void AnythingParserSemanticTest::Semantic15Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		any = "{}";
		// Anything-Object filled with Stream =
		//{}
		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyCharPtrType );
		ASSERT( any.GetSize() == 1 );
		ASSERT_EQUAL( any.AsCharPtr(), "{}" );
		if (any.SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		// Check if anything-objects can change correctly
		any = 34L;
		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyLongType );
		ASSERT( any.GetSize() == 1 );
		ASSERT( any.AsLong() == 34 );
		if (any.SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		// Check if anything-objects can change correctly
		any = 34.3E-56;
		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyDoubleType );
		ASSERT( any.GetSize() == 1 );
		ASSERT( any.AsDouble() == 34.3E-56 );
		if (any.SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}
	}
	// testSemantic15

	void AnythingParserSemanticTest::Semantic16Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		anyTemp0["Test147Dup"] = "a";
		anyTemp0.Append(3L);
		any["Test147"] = anyTemp0;
		// Anything-Object filled with Stream =
		//{
		//  /Test147 {
		//    /Test147Dup "a"
		//	  3
		//  }
		//}
		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 1 );

		ASSERT( any["Test147"].GetType() == AnyArrayType );
		ASSERT( any[0L].GetType() == AnyArrayType );
		ASSERT( any["Test147"].GetSize() == 2 );
		ASSERT( any[0L].GetSize() == 2 );
		ASSERT_EQUAL( any.SlotName(0L), "Test147" );
		ASSERT( any.FindIndex("Test147") == 0L );

		ASSERT( any["Test147"][0L].GetType() == AnyCharPtrType );
		ASSERT( any["Test147"]["Test147Dup"].GetType() == AnyCharPtrType );
		ASSERT( any[0L]["Test147Dup"].GetType() == AnyCharPtrType );
		ASSERT( any[0L][0L].GetType() == AnyCharPtrType );

		ASSERT( any["Test147"][0L].GetSize() == 1 );
		ASSERT( any["Test147"]["Test147Dup"].GetSize() == 1 );
		ASSERT( any[0L]["Test147Dup"].GetSize() == 1 );
		ASSERT( any[0L][0L].GetSize() == 1 );

		ASSERT_EQUAL( any["Test147"][0L].AsCharPtr(), "a" );
		ASSERT_EQUAL( any["Test147"]["Test147Dup"].AsCharPtr(), "a" );
		ASSERT_EQUAL( any[0L]["Test147Dup"].AsCharPtr(), "a" );
		ASSERT_EQUAL( any[0L][0L].AsCharPtr(), "a" );

		ASSERT_EQUAL( any["Test147"].SlotName(0L), "Test147Dup" );
		ASSERT_EQUAL( any[0L].SlotName(0L), "Test147Dup" );

		ASSERT( any["Test147"].FindIndex("Test147Dup") == 0L );
		ASSERT( any[0L].FindIndex("Test147Dup") == 0L );

		ASSERT( any["Test147"][1L].GetType() == AnyLongType );
		ASSERT( any[0L][1L].GetType() == AnyLongType );

		ASSERT( any["Test147"][1L].GetSize() == 1 );
		ASSERT( any[0L][1L].GetSize() == 1 );

		if (any["Test147"].SlotName(1L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}
		if (any[0L].SlotName(1L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 1 );
		ASSERT( any.FindIndex("Test145") == 0 );
		ASSERT_EQUAL( any.SlotName(0L), "Test145" );

		ASSERT( any["Test145"].GetType() == AnyNullType );
		ASSERT( any[0L].GetType() == AnyNullType );
		ASSERT( any["Test145"].GetSize() == 0 );
		ASSERT( any[0L].GetSize() == 0 );
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 1 );
		ASSERT( any.FindIndex("Test143") == 0 );
		ASSERT_EQUAL( any.SlotName(0L), "Test143" );

		ASSERT( any["Test143"].GetType() == AnyArrayType );
		ASSERT( any["Test143"].GetSize() == 1 );
		ASSERT( any[0L].GetType() == AnyArrayType );
		ASSERT( any[0L].GetSize() == 1 );
		ASSERT_EQUAL( any.SlotName(0L), "Test143" );

		ASSERT( any[0L][0L].GetType() == AnyArrayType );
		ASSERT( any[0L][0L].GetSize() == 1 );
		if (any["Test143"].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[0L][0L][0L].GetType() == AnyArrayType );
		ASSERT( any[0L][0L][0L].GetSize() == 1 );
		if (any["Test143"][0L].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[0L][0L][0L][0L].GetType() == AnyArrayType );
		ASSERT( any[0L][0L][0L][0L].GetSize() == 1 );
		if (any["Test143"][0L][0L].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[0L][0L][0L][0L][0L].GetType() == AnyNullType );
		ASSERT( any[0L][0L][0L][0L][0L].GetSize() == 0 );
		if (any["Test143"][0L][0L][0L].SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 8 );
		ASSERT( any.FindIndex("Test150") == 0 );
		ASSERT_EQUAL( any.SlotName(0L), "Test150" );

		ASSERT( any["Test150"].GetType() == AnyCharPtrType );
		ASSERT( any["Test150"].GetSize() == 1 );
		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );
		ASSERT_EQUAL( any[0L].AsCharPtr(), "abc" );
		ASSERT_EQUAL( any["Test150"].AsCharPtr(), "abc" );
		ASSERT_EQUAL( any.SlotName(0L), "Test150" );
		ASSERT( any.FindIndex("Test150") == 0L );

		ASSERT( any[1L].GetType() == AnyCharPtrType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT_EQUAL( any[1L].AsCharPtr(), "dfg" );
		if (any.SlotName(1L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[2L].GetType() == AnyCharPtrType );
		ASSERT( any[2L].GetSize() == 1 );
		ASSERT_EQUAL( any[2L].AsCharPtr(), "\\String1\"" );//"
		if (any.SlotName(2L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[3L].GetType() == AnyCharPtrType );
		ASSERT( any[3L].GetSize() == 1 );
		ASSERT_EQUAL( any[3L].AsCharPtr(), "\\String2" );
		if (any.SlotName(3L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[4L].GetType() == AnyCharPtrType );
		ASSERT( any[4L].GetSize() == 1 );
		ASSERT_EQUAL( any[4L].AsCharPtr(), "/String3\"" );//"
		if (any.SlotName(4L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[5L].GetType() == AnyCharPtrType );
		ASSERT( any[5L].GetSize() == 1 );
		ASSERT_EQUAL( any[5L].AsCharPtr(), "/String4" );
		if (any.SlotName(5L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[6L].GetType() == AnyCharPtrType );
		ASSERT( any[6L].GetSize() == 1 );
		ASSERT_EQUAL( any[6L].AsCharPtr(), "\\" );
		if (any.SlotName(6L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[7L].GetType() == AnyCharPtrType );
		ASSERT( any[7L].GetSize() == 1 );
		ASSERT_EQUAL( any[7L].AsCharPtr(), "/" );
		if (any.SlotName(7L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}
	}
	// testSemantic19

	void AnythingParserSemanticTest::Semantic20Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		any.Append("\"\\xGH\"");
		any.Append("\"\\GH\"");
		// Anything-Object filled with Stream =
		//{
		//  "\\xGH"
		//  "\\GH"
		//}
		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 2 );

		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );
		char testString[10] = "\"\\xGH\"";
		ASSERT_EQUAL( any[0L].AsCharPtr(), testString );
		//ASSERT_EQUAL( any[0L].AsCharPtr(), "\"\\xGH\"" );
		if (any.SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[1L].GetType() == AnyCharPtrType );
		ASSERT( any[1L].GetSize() == 1 );

		char testString2[10] = "\"\\GH\"";
		ASSERT_EQUAL( any[1L].AsCharPtr(), testString2 );
		//ASSERT_EQUAL( any[1L].AsCharPtr(), "\"\\GH\"" );
		if (any.SlotName(1L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}
	}
	// testSemantic20

	void AnythingParserSemanticTest::Semantic21Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		any["SlotName0"] = "abc";
		any["SlotName1"] = "de\x0E";
		any["SlotName2"] = "fg\0e"; // 0 will terminate the C-string upon construction
		any["SlotName3"] = "hi\x00";
		any["SlotName4"] = "jm\00";
		any["SlotName5"] = String((void *) "n\00p", 3); // assign a String with \0
		ASSERT( any["SlotName5"].AsString() == String((void *)"n\0p", 3));
		// Anything-Object filled with Stream =
		//{
		//  /SlotName0 "abc"
		//  /SlotName1 "de\x0E"
		//  /SlotName2 "fg\x0e"
		//	/SlotName3 "hi\x00"
		//	/SlotName4 "jm\00"
		//}
		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 6 );

		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName0"].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );
		ASSERT( any["SlotName0"].GetSize() == 1 );
		ASSERT_EQUAL( any[0L].AsCharPtr(), "abc" );
		ASSERT_EQUAL( any.SlotName(0L), "SlotName0" );

		ASSERT( any[1L].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName1"].GetType() == AnyCharPtrType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT( any["SlotName1"].GetSize() == 1 );
		ASSERT_EQUAL( any[1L].AsCharPtr(), "de\x0E" );
		ASSERT_EQUAL( any.SlotName(1L), "SlotName1" );

		ASSERT( any[2L].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName2"].GetType() == AnyCharPtrType );
		ASSERT( any[2L].GetSize() == 1 );
		ASSERT( any["SlotName2"].GetSize() == 1 );
		ASSERT_EQUAL( any[2L].AsCharPtr(), "fg" ); // 0ctal, end of C-string
		ASSERT_EQUAL( any[2L].AsCharPtr(), "fg\0e" );
		ASSERT_EQUAL( any.SlotName(2L), "SlotName2" );

		ASSERT( any[3L].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName3"].GetType() == AnyCharPtrType );
		ASSERT( any[3L].GetSize() == 1 );
		ASSERT( any["SlotName3"].GetSize() == 1 );
		ASSERT_EQUAL( any[3L].AsCharPtr(), "hi" );
		ASSERT_EQUAL( any.SlotName(3L), "SlotName3" );

		ASSERT( any[4L].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName4"].GetType() == AnyCharPtrType );
		ASSERT( any[4L].GetSize() == 1 );
		ASSERT( any["SlotName4"].GetSize() == 1 );
		ASSERT_EQUAL( any[4L].AsCharPtr(), "jm" );
		ASSERT_EQUAL( any.SlotName(4L), "SlotName4" );

		ASSERT( any["SlotName5"].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName5"].AsString() == String((void *)"n\0p", 3));
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 2 );

		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName0"].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );
		ASSERT( any["SlotName0"].GetSize() == 1 );
		ASSERT_EQUAL( any[0L].AsCharPtr(), "# Kein Kommentar" );
		ASSERT_EQUAL( any.SlotName(0L), "SlotName0" );

		ASSERT( any[1L].GetType() == AnyCharPtrType );
		ASSERT( any["SlotName1"].GetType() == AnyCharPtrType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT( any["SlotName1"].GetSize() == 1 );
		ASSERT_EQUAL( any[1L].AsCharPtr(), "# Kein Kommentar" );
		ASSERT_EQUAL( any.SlotName(1L), "SlotName1" );
	}
	// testSemantic22

	void AnythingParserSemanticTest::Semantic23Test() {
		// Init an anything-object for testing
		Anything any = emptyAny;
		any['\x30'] = "a"; // any[0..47] = *;   any[48] = "a"
		any["\x30"] = "b"; // any["\x30"] = "b" (index 49)
		any['\x3'] = "c"; // any[3] = "c"
		any["\x3"] = "d"; // any["\3"] = "d" (index 50)
		any['\30'] = "e"; // any[24] = "e"
		any["\30"] = "f"; // any["\30"] = "f" (index 51)
		any["\3"] = "g"; // any["\3"] = "h" (index 52)

		// Store and reload 'any'
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		long i, iMax = any.GetSize();

		for (i = 0; i < iMax; i++) {
			switch (i) {
				case 48: {
					ASSERT( any[i].GetType() == AnyCharPtrType );
					ASSERT( any[i].GetSize() == 1 );
					ASSERT_EQUAL( any[i].AsCharPtr(), "a" );
					if (any.SlotName(i) != NULL) {
						// SlotName should be NULL but it is not:  errMsg
						ASSERT( false );
					}
				}
					break;

				case 49: {
					ASSERT( any[i].GetType() == AnyCharPtrType );
					ASSERT( any[i].GetSize() == 1 );
					ASSERT_EQUAL( any[i].AsCharPtr(), "b" );
					ASSERT( any["\x30"].GetType() == AnyCharPtrType );
					ASSERT( any["\x30"].GetSize() == 1 );
					ASSERT_EQUAL( any["\x30"].AsCharPtr(), "b" );
					ASSERT_EQUAL( any.SlotName(i), "\x30" );
					ASSERT( any.FindIndex("\x30") == i );

				}
					break;

				case 3: {
					ASSERT( any[i].GetType() == AnyCharPtrType );
					ASSERT( any[i].GetSize() == 1 );
					ASSERT_EQUAL( any[i].AsCharPtr(), "c" );
					if (any.SlotName(i) != NULL) {
						// SlotName should be NULL but it is not:  errMsg
						ASSERT( false );
					}
				}
					break;

				case 50: {
					ASSERT( any[i].GetType() == AnyCharPtrType );
					ASSERT( any[i].GetSize() == 1 );
					ASSERT_EQUAL( any[i].AsCharPtr(), "g" );
					ASSERT( any["\x3"].GetType() == AnyCharPtrType );
					ASSERT( any["\x3"].GetSize() == 1 );
					ASSERT_EQUAL( any["\x3"].AsCharPtr(), "g" );
					ASSERT_EQUAL( any.SlotName(i), "\x3" );
					ASSERT( any.FindIndex("\x3") == i );

				}
					break;

				case 24: {
					ASSERT( any[i].GetType() == AnyCharPtrType );
					ASSERT( any[i].GetSize() == 1 );
					ASSERT_EQUAL( any[i].AsCharPtr(), "e" );
					if (any.SlotName(i) != NULL) {
						// SlotName should be NULL but it is not:  errMsg
						ASSERT( false );
					}
				}
					break;

				case 51: {
					ASSERT( any[i].GetType() == AnyCharPtrType );
					ASSERT( any[i].GetSize() == 1 );
					ASSERT_EQUAL( any[i].AsCharPtr(), "f" );
					ASSERT( any["\30"].GetType() == AnyCharPtrType );
					ASSERT( any["\30"].GetSize() == 1 );
					ASSERT_EQUAL( any["\30"].AsCharPtr(), "f" );
					ASSERT_EQUAL( any.SlotName(i), "\30" );
					ASSERT( any.FindIndex("\30") == i );
				}
					break;

				case 52: {
					ASSERT( any[i].GetType() == AnyCharPtrType );
					ASSERT( any[i].GetSize() == 1 );
					ASSERT_EQUAL( any[i].AsCharPtr(), "g" );
					ASSERT( any["\3"].GetType() == AnyCharPtrType );
					ASSERT( any["\3"].GetSize() == 1 );
					ASSERT_EQUAL( any["\3"].AsCharPtr(), "d" );
					ASSERT_EQUAL( any.SlotName(i), "\3" );
					ASSERT( any.FindIndex("\3") == i );
				}
					break;

				default: {
					ASSERT( any[i].GetType() == AnyNullType );
					ASSERT( any[i].GetSize() == 0 );
					if (any.SlotName(i) != NULL) {
						// SlotName should be NULL but it is not:  errMsg
						ASSERT( false );
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
		any.Append("/a");
		any.Append(" String");
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 4 );

		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );
		ASSERT_EQUAL( any[0L].AsCharPtr(), "/a" );
		if (any.SlotName(0L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[1L].GetType() == AnyCharPtrType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT_EQUAL( any[1L].AsCharPtr(), " String" );
		if (any.SlotName(1L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
		}

		ASSERT( any[2L].GetType() == AnyCharPtrType );
		ASSERT( any["a"].GetType() == AnyCharPtrType );
		ASSERT( any[2L].GetSize() == 1 );
		ASSERT( any["a"].GetSize() == 1 );
		ASSERT_EQUAL( any[2L].AsCharPtr(), " String" );
		ASSERT_EQUAL( any["a"].AsCharPtr(), " String" );
		ASSERT_EQUAL( any.SlotName(2L), "a" );
		ASSERT( any.FindIndex("a") == 2L );

		ASSERT( any[3L].GetType() == AnyCharPtrType );
		ASSERT( any[" String"].GetType() == AnyCharPtrType );
		ASSERT( any[3L].GetSize() == 1 );
		ASSERT( any[" String"].GetSize() == 1 );
		ASSERT_EQUAL( any[3L].AsCharPtr(), "a" );
		ASSERT_EQUAL( any[" String"].AsCharPtr(), "a" );
		ASSERT_EQUAL( any.SlotName(3L), " String" );
		ASSERT( any.FindIndex(" String") == 3L );
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 2 );

		ASSERT( any[0L].GetType() == AnyCharPtrType );
		ASSERT( any[0L].GetSize() == 1 );
		ASSERT_EQUAL( any[0L].AsCharPtr(), "A" );
		ASSERT( any["a"].GetType() == AnyCharPtrType );
		ASSERT( any["a"].GetSize() == 1 );
		ASSERT_EQUAL( any["a"].AsCharPtr(), "A" );
		ASSERT_EQUAL( any.SlotName(0L), "a" );
		ASSERT( any.FindIndex("a") == 0L );

		ASSERT( any[1L].GetType() == AnyCharPtrType );
		ASSERT( any[1L].GetSize() == 1 );
		ASSERT_EQUAL( any[1L].AsCharPtr(), "A" );
		if (any.SlotName(1L) != NULL) {
			// SlotName should be NULL but it is not:  errMsg
			ASSERT( false );
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
		Anything any1 = AnythingParserSemanticTest::storeAndReload(any);
		any = emptyAny;
		any = any1;

		// Check if the anything-object is the expected one
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any.GetSize() == 1 );

		ASSERT( any[0L].GetType() == AnyArrayType );
		ASSERT( any[0L].GetSize() == 2 );
		ASSERT_EQUAL( any[0L][0L].AsCharPtr(), "A" );
		ASSERT( any["a"].GetType() == AnyArrayType );
		ASSERT( any["a"].GetSize() == 2 );
		ASSERT_EQUAL( any["a"][0L].AsCharPtr(), "A" );
		ASSERT_EQUAL( any.SlotName(0L), "a" );
		ASSERT( any.FindIndex("a") == 0L );

		ASSERT( any[0L][1L].GetType() == AnyArrayType );
		ASSERT( any[0L][1L].GetSize() == 2 );
		ASSERT_EQUAL( any[0L][1L][0L].AsCharPtr(), "B" );
		ASSERT( any["a"]["a"].GetType() == AnyArrayType );
		ASSERT( any["a"]["a"].GetSize() == 2 );
		ASSERT_EQUAL( any["a"]["a"][0L].AsCharPtr(), "B" );
		ASSERT_EQUAL( any["a"].SlotName(1L), "a" );
		ASSERT( any["a"].FindIndex("a") == 1L );

		ASSERT( any[0L][1L][1L].GetType() == AnyArrayType );
		ASSERT( any[0L][1L][1L].GetSize() == 2 );
		ASSERT_EQUAL( any[0L][1L][1L][0L].AsCharPtr(), "C" );
		ASSERT( any["a"]["a"]["a"].GetType() == AnyArrayType );
		ASSERT( any["a"]["a"]["a"].GetSize() == 2 );
		ASSERT_EQUAL( any["a"]["a"]["a"][0L].AsCharPtr(), "C" );
		ASSERT_EQUAL( any["a"]["a"].SlotName(1L), "a" );
		ASSERT( any["a"]["a"].FindIndex("a") == 1L );

		ASSERT( any[0L][1L][1L][1L].GetType() == AnyCharPtrType );
		ASSERT_EQUAL( any[0L][1L][1L].SlotName(1L), "a" );
		ASSERT( any[0L][1L][1L].FindIndex("D") < 0L ); // Because the type is eCharPtr and not eArray
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
		ASSERT( any["a"][0L]["a"].AsLong() == anyTemp0["a"].AsLong() );
		ASSERT( any["a"][1L]["a"].AsLong() == anyTemp1["a"].AsLong() );
		ASSERT( any["a"][2L]["a"].AsLong() != anyTemp0["a"].AsLong() );

		//  bestimmte Manipulation in any f�hren zu Ver�nderungen in anyTemp0/1
		//  sofern per referenzen zugewiesen wurde.
		any["a"][0L]["b"] = 32L;
		any["a"][1L]["c"] = 43L;
		any["a"][2L]["d"] = 54L;
		ASSERT( any["a"][0L]["b"].AsLong() == anyTemp0["b"].AsLong() );
		ASSERT( any["a"][1L]["c"].AsLong() == anyTemp1["c"].AsLong() );
		ASSERT( !(anyTemp0.IsDefined("d")) );

		//	der Link kann auch wieder unterbrochen werden.
		any["a"].Remove(0L); // Vorsicht, andere Eintr�ge werden geschoben
		ASSERT( any["a"][0L]["a"] == anyTemp1["a"].AsLong());
	}
	//	testSemantic27

	void AnythingParserSemanticTest::testSemantic28Prep(Anything *any) {
		Anything *anyHlp0, *anyHlp1;
		anyHlp0 = new Anything();
		anyHlp1 = new Anything();

		(*anyHlp0)["a"] = 0L;
		(*anyHlp1)["b"] = 1L;
		any->Append(*anyHlp0);
		any->Append(*anyHlp1);

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

		AnythingParserSemanticTest::testSemantic28Prep(any);

		delete any;
	}
	//	testSemantic28

	void AnythingParserSemanticTest::testSemantic29Prep(Anything *any) {
		Anything anyHlp0, anyHlp1;

		anyHlp0["a"] = 0L;
		anyHlp1["b"] = 1L;
		any->Append(anyHlp0);
		any->Append(anyHlp1);

		anyHlp0["a"] = 2L;
		anyHlp1["b"] = 3L;
	}
	//	testSemantic29Prep

	void AnythingParserSemanticTest::Semantic29Test() {
		// Init an anything-object for testing
		Anything *any;
		any = new Anything();

		AnythingParserSemanticTest::testSemantic29Prep(any);

		delete any;
	}
	//	testSemantic29

	void AnythingParserSemanticTest::testSemantic30Prep(Anything &any) {
		Anything anyHlp0, anyHlp1;

		anyHlp0["a"] = 0L;
		anyHlp1["b"] = 1L;
		any.Append(anyHlp0);
		any.Append(anyHlp1);

		anyHlp0["a"] = 2L;
		anyHlp1["b"] = 3L;
	}
	//	testSemantic30Prep

	void AnythingParserSemanticTest::Semantic30Test() {
		// Init an anything-object for testing
		Anything any;

		AnythingParserSemanticTest::testSemantic30Prep(any);
	}
	//	testSemantic30

	void AnythingParserSemanticTest::Semantic31Test() {
		// Init an anything-object for testing
		Anything any, anyHlp0, anyHlp1;

		anyHlp0["a"] = 0L;
		anyHlp1["b"] = 1L;
		any.Append(anyHlp0);
		any.Append(anyHlp1);

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
		ASSERT( anyTemp0["a"].AsLong() == 0 );
		ASSERT( anyTemp1["a"].AsLong() == 1 );
		ASSERT( any[0L].AsLong() == 0L );
		ASSERT( any[1L].AsLong() == 0L );
		ASSERT( any[2L].GetType() == AnyNullType );

		any["a"].Remove(1L);
		ASSERT( anyTemp0["a"].AsLong() == 0 );
		ASSERT( anyTemp1["a"].AsLong() == 1 );
		ASSERT( any[0L].AsLong() == 0L );
		ASSERT( any[1L].GetType() == AnyNullType );
		ASSERT( any[2L].GetType() == AnyNullType );

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
		ASSERT( anyTemp0.GetType() == AnyArrayType );
		ASSERT( anyTemp1.GetType() == AnyArrayType );
		ASSERT( any.GetType() == AnyArrayType );
		ASSERT( any[0L].GetType() == AnyArrayType );
		ASSERT( any[1L].GetType() == AnyArrayType );
		ASSERT( any[0L]["a"].AsLong() == anyTemp0["a"].AsLong() );
		ASSERT( any[1L]["b"].AsLong() == anyTemp1["b"].AsLong() );
		// Change something
		anyTemp0["a"] = 2L;
		anyTemp1["b"] = 3L;
		// Check "any":  because any[0/1] is a reference to anyTemp0/1, "any" has changed, too.
		ASSERT( any[0L]["a"].AsLong() == anyTemp0["a"].AsLong() );
		ASSERT( any[1L]["b"].AsLong() == anyTemp1["b"].AsLong() );

		// Change the type of anyTemp0/1 from eArray to eLong;
		anyTemp0 = 4L;
		anyTemp1 = 5L;
		ASSERT( anyTemp0.GetType() == AnyLongType );
		ASSERT( anyTemp1.GetType() == AnyLongType );
		ASSERT( anyTemp0.AsLong() == 4 );
		ASSERT( anyTemp1.AsLong() == 5 );
		// Because anyTemp0/1 are not eArray, they are not referenced by "any" anymore.
		ASSERT( any[0L].GetType() == AnyArrayType );
		ASSERT( any[1L].GetType() == AnyArrayType );
		ASSERT( any[0L]["a"].AsLong() == 2 );
		ASSERT( any[1L]["b"].AsLong() == 3 );

		// Do a change
		anyTemp0 = 6L;
		anyTemp1 = 7L;
		ASSERT( anyTemp0.AsLong() == 6 );
		ASSERT( anyTemp1.AsLong() == 7 );
		// Because anyTemp0/1 are not eArray, they are not referenced by "any" anymore.
		ASSERT( any[0L].GetType() == AnyArrayType );
		ASSERT( any[1L].GetType() == AnyArrayType );
		ASSERT( any[0L]["a"].AsLong() == 2 );
		ASSERT( any[1L]["b"].AsLong() == 3 );

		// Do a change
		anyTemp0["a"] = 8L;
		anyTemp1["b"] = 9L;
		ASSERT( anyTemp0.GetType() == AnyArrayType );
		ASSERT( anyTemp1.GetType() == AnyArrayType );
		ASSERT( anyTemp0["a"].AsLong() == 8 );
		ASSERT( anyTemp1["b"].AsLong() == 9 );
		// Because anyTemp0/1 are not eArray, they are not referenced by "any" anymore.
		ASSERT( any[0L].GetType() == AnyArrayType );
		ASSERT( any[1L].GetType() == AnyArrayType );
		ASSERT( any[0L]["a"].AsLong() == 2 );
		ASSERT( any[1L]["b"].AsLong() == 3 );

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
			ASSERT_EQUAL("Test/Slot", any.SlotName(0));
			ASSERT_EQUAL(12, any["Test/Slot"].AsLong(-1));
		}
		{
			String buf("{ /Test/Slot 12 }");
			// a slash in a slotname terminates the slot with a null any as value
			// (an error message is written)
			IStringStream is(buf);
			Anything any;

			is >> any;
			ASSERT_EQUAL("Test", any.SlotName(0));
			ASSERT(any["Test"].IsNull());
		}
		{
			String buf("{ //Test 12 }");
			// a slash at the beginning is ignored (but generates an error message)
			IStringStream is(buf);
			Anything any;

			is >> any;
			ASSERT_EQUAL("Test", any.SlotName(0));
			ASSERT_EQUAL(12, any["Test"].AsLong(-1)); // contains an empty any
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
        ASSERT_EQUAL(testString ,s);\
        Anything b;\
        IStringStream is(s);\
        b.Import(is);\
        ASSERT(b.IsDefined(#xslot));\
        ASSERT_EQUAL(b[#xslot].AsLong(),1);\
    }
		// end of macro -- use it
		TESTQUOTE(needs quote);
		TESTQUOTE(1234);
		TESTQUOTE(1startswithnumber);
		TESTQUOTE(another {quote);
			//    a = Anything();
			//    a["1234"]= 2;
			//
			//    ASSERT_EQUAL
		}
