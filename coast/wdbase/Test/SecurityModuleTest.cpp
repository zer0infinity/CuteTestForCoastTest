/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SecurityModuleTest.h"
#include "TestSuite.h"
#include "SecurityModule.h"
#include "SystemFile.h"

using namespace coast;

Test *SecurityModuleTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, SecurityModuleTest, EncodeDecodeTest);
	ADD_CASE(testSuite, SecurityModuleTest, InitWithConfigTest);
	return testSuite;
}

SecurityModuleTest::SecurityModuleTest(TString tname) :
		TestCaseType(tname) {
}

SecurityModuleTest::~SecurityModuleTest() {
}

class TestScrambler: public Scrambler {
public:
	TestScrambler(const char *name) :
			Scrambler(name), fKey("") {
	}
	virtual ~TestScrambler() {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) TestScrambler(fName);
	}
	virtual void InitKey(const String &key) {
		fKey = key;
	}
	String fKey;

};
RegisterScrambler(TestScrambler);
void SecurityModuleTest::InitWithConfigTest() {
	StartTrace(SecurityModuleTest.InitWithConfigTest);
	SecurityModule smtest("smtest");
	Anything theConfig;
	theConfig["smtest"]["SecurityItems"]["TestScrambler"] = "MyTestScrambler";
	TraceAny(theConfig, "config to use");
	t_assert(system::GetFilePath("SecurityItems", "any") != "");
	Anything keyFromFile;
	t_assert(system::LoadConfigFile(keyFromFile, "aTestKey.txt", ""));
	t_assert(keyFromFile.AsString("oops") != "oops");
	TestScrambler *ts = SafeCast(Scrambler::FindScrambler("MyTestScrambler"), TestScrambler);
	t_assert(!ts);
	smtest.Init(theConfig);
	ts = SafeCast(Scrambler::FindScrambler("MyTestScrambler"), TestScrambler);
	if (t_assert(ts)) {
		assertEqual(keyFromFile.AsString("oops"), ts->fKey);
	}
}

void SecurityModuleTest::tearDown() {
	StartTrace(SecurityModuleTest.tearDown);
	WDModule::Terminate(ROAnything());
}

void SecurityModuleTest::EncodeDecodeTest() {
	Encoder encoder("encoder");

	int i;

	String EmptySt;
	String EncodedString;
	String DecodedString;
	String OriginalString;

	// Test 1:
	// checkif all HEX-values can be en-/decoded
	// check for identity of original and decoded string
	// check the lengths

	// setup original
	for (i = 0; i < 256; i++) {
		OriginalString.Append((char) i);
	}

	// encode decode with standard encoder (cleartext)
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	// assert strings are equal
	t_assert( memcmp( (const char *)OriginalString, (const char *)DecodedString, OriginalString.Length() ) == 0);
	t_assert( OriginalString.Length() == DecodedString.Length());

	assertEqual( OriginalString, DecodedString);
	assertEqual( OriginalString.Length(), DecodedString.Length());

	// Test 2
	// reset strings
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	// encode decode with standard encoder (cleartext)
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	// check whether the Strings are identical
	assertCharPtrEqual( OriginalString, DecodedString);
	assertEqual(1, (long)( OriginalString == DecodedString ));

	// Test 3
	// reset strings
	OriginalString = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	// encode decode with standard encoder (cleartext)
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	// check whether the Strings are identical
	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);
	assertCharPtrEqual( "ABCDEFGHIJKLMNOPQRSTUVWXYZ", EncodedString);

	//Test 4
	// Only Unprintable Chars, unscrambled
	OriginalString = EmptySt;
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	for (i = 0; i < 10; i++) {
		OriginalString.Append((char) i);
	}

	// encode decode with standard encoder (cleartext)
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	// check whether the Strings are identical
	t_assert( (memcmp( (const char *)OriginalString, (const char *)DecodedString, OriginalString.Length() ) == 0));
	t_assert( OriginalString.Length() == DecodedString.Length());

	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);

	assertCharPtrEqual( "\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09", EncodedString);

	// One Printable Char, unscrambled
	OriginalString = "M";
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	// check whether the Strings are identical
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);
	assertCharPtrEqual( "M", EncodedString);

	// One Unprintable Char, unscrambled
	OriginalString = EmptySt;
	OriginalString.Append((char) 5);

	OriginalString = OriginalString;
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);

	// String::IntPrintOn:  05     -> \x05
	// coast::urlutils::urlEncode: \x05   -> %5Cx05
	// coast::urlutils::urlEncode: %5Cx05 -> %255Cx05
	assertCharPtrEqual( "\x05", EncodedString);

	// Only Printable Chars, scrambled
	OriginalString = "ABC";
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);
	// w\"ABC\"
	assertCharPtrEqual( "ABC", EncodedString);

	// a particularly nasty sequence of non-printables and printables
	OriginalString = "Anfang%252B+%2BEnde";
	EncodedString = EmptySt;
	DecodedString = EmptySt;

	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	assertEqual( OriginalString, DecodedString);
	assertEqual( OriginalString, EncodedString);

	// Only Unprintable Chars, scrambled
	OriginalString = EmptySt;
	for (i = 0; i < 3; i++) {
		OriginalString.Append((char) i);
	}
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);

	assertCharPtrEqual( "\x00\x01\x02", EncodedString);

	// One Printable Char, scrambled
	OriginalString = "M";
	OriginalString = OriginalString;
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);
	assertCharPtrEqual( "M", EncodedString);

	//           1         2         3         4         5         6
	// 0123456789012345678901234567890123456789012345678901234567890123456789
	// ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789-$

	// One Unprintable Char, scrambled
	OriginalString = EmptySt;
	OriginalString.Append((char) 5);

	OriginalString = OriginalString;
	EncodedString = EmptySt;
	DecodedString = EmptySt;
	encoder.DoEncode(EncodedString, OriginalString);
	encoder.DoDecode(DecodedString, EncodedString);

	assertCharPtrEqual( OriginalString, DecodedString);
	t_assert( OriginalString == DecodedString);

	assertCharPtrEqual( "\x05", EncodedString);
}
