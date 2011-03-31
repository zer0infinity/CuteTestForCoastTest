/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include <stdio.h>
#include "BlowfishTest.h"
#include "Blowfish.h"
#include "TestSuite.h"
#include "Dbg.h"
#include "Base64.h"
#include "SystemFile.h"
#include "StringStream.h"

/* Lets use the DES test vectors :-) */
#define NUM_TESTS 34
static unsigned char ecb_data[NUM_TESTS][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x30, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10},
	{0x7C, 0xA1, 0x10, 0x45, 0x4A, 0x1A, 0x6E, 0x57},
	{0x01, 0x31, 0xD9, 0x61, 0x9D, 0xC1, 0x37, 0x6E},
	{0x07, 0xA1, 0x13, 0x3E, 0x4A, 0x0B, 0x26, 0x86},
	{0x38, 0x49, 0x67, 0x4C, 0x26, 0x02, 0x31, 0x9E},
	{0x04, 0xB9, 0x15, 0xBA, 0x43, 0xFE, 0xB5, 0xB6},
	{0x01, 0x13, 0xB9, 0x70, 0xFD, 0x34, 0xF2, 0xCE},
	{0x01, 0x70, 0xF1, 0x75, 0x46, 0x8F, 0xB5, 0xE6},
	{0x43, 0x29, 0x7F, 0xAD, 0x38, 0xE3, 0x73, 0xFE},
	{0x07, 0xA7, 0x13, 0x70, 0x45, 0xDA, 0x2A, 0x16},
	{0x04, 0x68, 0x91, 0x04, 0xC2, 0xFD, 0x3B, 0x2F},
	{0x37, 0xD0, 0x6B, 0xB5, 0x16, 0xCB, 0x75, 0x46},
	{0x1F, 0x08, 0x26, 0x0D, 0x1A, 0xC2, 0x46, 0x5E},
	{0x58, 0x40, 0x23, 0x64, 0x1A, 0xBA, 0x61, 0x76},
	{0x02, 0x58, 0x16, 0x16, 0x46, 0x29, 0xB0, 0x07},
	{0x49, 0x79, 0x3E, 0xBC, 0x79, 0xB3, 0x25, 0x8F},
	{0x4F, 0xB0, 0x5E, 0x15, 0x15, 0xAB, 0x73, 0xA7},
	{0x49, 0xE9, 0x5D, 0x6D, 0x4C, 0xA2, 0x29, 0xBF},
	{0x01, 0x83, 0x10, 0xDC, 0x40, 0x9B, 0x26, 0xD6},
	{0x1C, 0x58, 0x7F, 0x1C, 0x13, 0x92, 0x4F, 0xEF},
	{0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x01},
	{0x1F, 0x1F, 0x1F, 0x1F, 0x0E, 0x0E, 0x0E, 0x0E},
	{0xE0, 0xFE, 0xE0, 0xFE, 0xF1, 0xFE, 0xF1, 0xFE},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10}
};

static unsigned char plain_data[NUM_TESTS][8] = {
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x10, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x01},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11, 0x11},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x01, 0xA1, 0xD6, 0xD0, 0x39, 0x77, 0x67, 0x42},
	{0x5C, 0xD5, 0x4C, 0xA8, 0x3D, 0xEF, 0x57, 0xDA},
	{0x02, 0x48, 0xD4, 0x38, 0x06, 0xF6, 0x71, 0x72},
	{0x51, 0x45, 0x4B, 0x58, 0x2D, 0xDF, 0x44, 0x0A},
	{0x42, 0xFD, 0x44, 0x30, 0x59, 0x57, 0x7F, 0xA2},
	{0x05, 0x9B, 0x5E, 0x08, 0x51, 0xCF, 0x14, 0x3A},
	{0x07, 0x56, 0xD8, 0xE0, 0x77, 0x47, 0x61, 0xD2},
	{0x76, 0x25, 0x14, 0xB8, 0x29, 0xBF, 0x48, 0x6A},
	{0x3B, 0xDD, 0x11, 0x90, 0x49, 0x37, 0x28, 0x02},
	{0x26, 0x95, 0x5F, 0x68, 0x35, 0xAF, 0x60, 0x9A},
	{0x16, 0x4D, 0x5E, 0x40, 0x4F, 0x27, 0x52, 0x32},
	{0x6B, 0x05, 0x6E, 0x18, 0x75, 0x9F, 0x5C, 0xCA},
	{0x00, 0x4B, 0xD6, 0xEF, 0x09, 0x17, 0x60, 0x62},
	{0x48, 0x0D, 0x39, 0x00, 0x6E, 0xE7, 0x62, 0xF2},
	{0x43, 0x75, 0x40, 0xC8, 0x69, 0x8F, 0x3C, 0xFA},
	{0x07, 0x2D, 0x43, 0xA0, 0x77, 0x07, 0x52, 0x92},
	{0x02, 0xFE, 0x55, 0x77, 0x81, 0x17, 0xF1, 0x2A},
	{0x1D, 0x9D, 0x5C, 0x50, 0x18, 0xF7, 0x28, 0xC2},
	{0x30, 0x55, 0x32, 0x28, 0x6D, 0x6F, 0x29, 0x5A},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0x01, 0x23, 0x45, 0x67, 0x89, 0xAB, 0xCD, 0xEF},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00},
	{0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF}
};

static unsigned char cipher_data[NUM_TESTS][8] = {
	{0x4E, 0xF9, 0x97, 0x45, 0x61, 0x98, 0xDD, 0x78},
	{0x51, 0x86, 0x6F, 0xD5, 0xB8, 0x5E, 0xCB, 0x8A},
	{0x7D, 0x85, 0x6F, 0x9A, 0x61, 0x30, 0x63, 0xF2},
	{0x24, 0x66, 0xDD, 0x87, 0x8B, 0x96, 0x3C, 0x9D},
	{0x61, 0xF9, 0xC3, 0x80, 0x22, 0x81, 0xB0, 0x96},
	{0x7D, 0x0C, 0xC6, 0x30, 0xAF, 0xDA, 0x1E, 0xC7},
	{0x4E, 0xF9, 0x97, 0x45, 0x61, 0x98, 0xDD, 0x78},
	{0x0A, 0xCE, 0xAB, 0x0F, 0xC6, 0xA0, 0xA2, 0x8D},
	{0x59, 0xC6, 0x82, 0x45, 0xEB, 0x05, 0x28, 0x2B},
	{0xB1, 0xB8, 0xCC, 0x0B, 0x25, 0x0F, 0x09, 0xA0},
	{0x17, 0x30, 0xE5, 0x77, 0x8B, 0xEA, 0x1D, 0xA4},
	{0xA2, 0x5E, 0x78, 0x56, 0xCF, 0x26, 0x51, 0xEB},
	{0x35, 0x38, 0x82, 0xB1, 0x09, 0xCE, 0x8F, 0x1A},
	{0x48, 0xF4, 0xD0, 0x88, 0x4C, 0x37, 0x99, 0x18},
	{0x43, 0x21, 0x93, 0xB7, 0x89, 0x51, 0xFC, 0x98},
	{0x13, 0xF0, 0x41, 0x54, 0xD6, 0x9D, 0x1A, 0xE5},
	{0x2E, 0xED, 0xDA, 0x93, 0xFF, 0xD3, 0x9C, 0x79},
	{0xD8, 0x87, 0xE0, 0x39, 0x3C, 0x2D, 0xA6, 0xE3},
	{0x5F, 0x99, 0xD0, 0x4F, 0x5B, 0x16, 0x39, 0x69},
	{0x4A, 0x05, 0x7A, 0x3B, 0x24, 0xD3, 0x97, 0x7B},
	{0x45, 0x20, 0x31, 0xC1, 0xE4, 0xFA, 0xDA, 0x8E},
	{0x75, 0x55, 0xAE, 0x39, 0xF5, 0x9B, 0x87, 0xBD},
	{0x53, 0xC5, 0x5F, 0x9C, 0xB4, 0x9F, 0xC0, 0x19},
	{0x7A, 0x8E, 0x7B, 0xFA, 0x93, 0x7E, 0x89, 0xA3},
	{0xCF, 0x9C, 0x5D, 0x7A, 0x49, 0x86, 0xAD, 0xB5},
	{0xD1, 0xAB, 0xB2, 0x90, 0x65, 0x8B, 0xC7, 0x78},
	{0x55, 0xCB, 0x37, 0x74, 0xD1, 0x3E, 0xF2, 0x01},
	{0xFA, 0x34, 0xEC, 0x48, 0x47, 0xB2, 0x68, 0xB2},
	{0xA7, 0x90, 0x79, 0x51, 0x08, 0xEA, 0x3C, 0xAE},
	{0xC3, 0x9E, 0x07, 0x2D, 0x9F, 0xAC, 0x63, 0x1D},
	{0x01, 0x49, 0x33, 0xE0, 0xCD, 0xAF, 0xF6, 0xE4},
	{0xF2, 0x1E, 0x9A, 0x77, 0xB7, 0x1C, 0x49, 0xBC},
	{0x24, 0x59, 0x46, 0x88, 0x57, 0x54, 0x36, 0x9A},
	{0x6B, 0x5C, 0x5A, 0x9C, 0x5D, 0x9E, 0x0A, 0x5A},
};

#define KEY_TEST_NUM    25
unsigned char key_test[KEY_TEST_NUM] = {
	0xf0, 0xe1, 0xd2, 0xc3, 0xb4, 0xa5, 0x96, 0x87,
	0x78, 0x69, 0x5a, 0x4b, 0x3c, 0x2d, 0x1e, 0x0f,
	0x00, 0x11, 0x22, 0x33, 0x44, 0x55, 0x66, 0x77,
	0x88
};

unsigned char key_data[8] =
{0xFE, 0xDC, 0xBA, 0x98, 0x76, 0x54, 0x32, 0x10};

unsigned char key_out[KEY_TEST_NUM][8] = {
	{0xF9, 0xAD, 0x59, 0x7C, 0x49, 0xDB, 0x00, 0x5E},
	{0xE9, 0x1D, 0x21, 0xC1, 0xD9, 0x61, 0xA6, 0xD6},
	{0xE9, 0xC2, 0xB7, 0x0A, 0x1B, 0xC6, 0x5C, 0xF3},
	{0xBE, 0x1E, 0x63, 0x94, 0x08, 0x64, 0x0F, 0x05},
	{0xB3, 0x9E, 0x44, 0x48, 0x1B, 0xDB, 0x1E, 0x6E},
	{0x94, 0x57, 0xAA, 0x83, 0xB1, 0x92, 0x8C, 0x0D},
	{0x8B, 0xB7, 0x70, 0x32, 0xF9, 0x60, 0x62, 0x9D},
	{0xE8, 0x7A, 0x24, 0x4E, 0x2C, 0xC8, 0x5E, 0x82},
	{0x15, 0x75, 0x0E, 0x7A, 0x4F, 0x4E, 0xC5, 0x77},
	{0x12, 0x2B, 0xA7, 0x0B, 0x3A, 0xB6, 0x4A, 0xE0},
	{0x3A, 0x83, 0x3C, 0x9A, 0xFF, 0xC5, 0x37, 0xF6},
	{0x94, 0x09, 0xDA, 0x87, 0xA9, 0x0F, 0x6B, 0xF2},
	{0x88, 0x4F, 0x80, 0x62, 0x50, 0x60, 0xB8, 0xB4},
	{0x1F, 0x85, 0x03, 0x1C, 0x19, 0xE1, 0x19, 0x68},
	{0x79, 0xD9, 0x37, 0x3A, 0x71, 0x4C, 0xA3, 0x4F},
	{0x93, 0x14, 0x28, 0x87, 0xEE, 0x3B, 0xE1, 0x5C},
	{0x03, 0x42, 0x9E, 0x83, 0x8C, 0xE2, 0xD1, 0x4B},
	{0xA4, 0x29, 0x9E, 0x27, 0x46, 0x9F, 0xF6, 0x7B},
	{0xAF, 0xD5, 0xAE, 0xD1, 0xC1, 0xBC, 0x96, 0xA8},
	{0x10, 0x85, 0x1C, 0x0E, 0x38, 0x58, 0xDA, 0x9F},
	{0xE6, 0xF5, 0x1E, 0xD7, 0x9B, 0x9D, 0xB2, 0x1F},
	{0x64, 0xA6, 0xE1, 0x4A, 0xFD, 0x36, 0xB4, 0x6F},
	{0x80, 0xC7, 0xD7, 0xD4, 0x5A, 0x54, 0x79, 0xAD},
	{0x05, 0x04, 0x4B, 0x62, 0xFA, 0x52, 0xD0, 0x80},
};

BlowfishTest::BlowfishTest(TString tstrName) : TestCaseType(tstrName)
{};

BlowfishTest::~BlowfishTest() {}

Test *BlowfishTest::suite ()
/* what: return the whole suite of tests for renderers, we could add the suites
                 of the derived classes here, but we would need to include all the headers
                 of the derived classes which is not nice.
                 Add cases for the generic Renderer here (if you find any that is);
*/
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, BlowfishTest, paddingTest);
	ADD_CASE(testSuite, BlowfishTest, paddingFrontTest);
	ADD_CASE(testSuite, BlowfishTest, paddingIsRandomTest);
	ADD_CASE(testSuite, BlowfishTest, scrambleUnscramble);
	ADD_CASE(testSuite, BlowfishTest, differentKey);
	ADD_CASE(testSuite, BlowfishTest, differentLength);
	ADD_CASE(testSuite, BlowfishTest, textWithNull);
	ADD_CASE(testSuite, BlowfishTest, modifyScramble);
	ADD_CASE(testSuite, BlowfishTest, rawECBCyphers);
	ADD_CASE(testSuite, BlowfishTest, scrambleLongerStringTheSame);
	ADD_CASE(testSuite, BlowfishTest, scrambleLongerStringDifferent);
	ADD_CASE(testSuite, BlowfishTest, scrambleCarefullyPaddedString);
	ADD_CASE(testSuite, BlowfishTest, scrambleSameKeyButDifferentIvec);
	ADD_CASE(testSuite, BlowfishTest, cbcCrossPlatform);

	return testSuite;
}
void BlowfishTest::paddingTest()
{
	StartTrace(BlowfishTest.paddingTest);

	BlowfishScrambler bf("TestPad");
	for (long l = 0; l < 20; l++) {
		String topad;
		topad.Append("12345678901234567890", l);
		assertEqual(l, topad.Length());
		String padded;
		bf.PadToBlockSize(padded, topad);
		t_assert(padded.Length() > topad.Length());
		assertEqual(0L, padded.Length() % BF_BLOCK);
		t_assert(bf.UnPad(padded));
		assertEqual(topad, padded);
	}
	// test invalid padded strings
	String shortblock("01234");
	t_assert(!bf.UnPad(shortblock));
	String oneblockOK("01234567");
	t_assert(bf.UnPad(oneblockOK));
	String twoblockOK((void *)"012345670123456\002", 16);
	t_assert(bf.UnPad(twoblockOK));
	assertEqual(14, twoblockOK.Length());
	String twoblockWrong((void *)"012345670123456\011", 16);
	t_assert(!bf.UnPad(twoblockWrong));
}

void BlowfishTest::paddingFrontTest()
{
	StartTrace(BlowfishTest.paddingFrontTest);

	BlowfishCBCScrambler bf("TestPadFront");
	for (long l = 0; l < 20; l++) {
		String topad;
		topad.Append("12345678901234567890", l);
		assertEqual(l, topad.Length());
		String padded;
		bf.PadToBlockSize(padded, topad);
		t_assert(padded.Length() > topad.Length());
		assertEqual(0L, padded.Length() % BF_BLOCK);
		t_assert(bf.UnPad(padded));
		assertEqual(topad, padded);
	}
	// test invalid padded strings
	String oneblock("01234567");
	t_assert(!bf.UnPad(oneblock));
	String twoblockOK((void *)"\002\0a1234567012345678", 16);
	t_assert(bf.UnPad(twoblockOK));
	assertEqual(14, twoblockOK.Length());
	assertEqual("a1234567012345", twoblockOK);
	String twoblockWrong((void *)"012345670123456\011", 16);
	t_assert(!bf.UnPad(twoblockWrong));
}
void BlowfishTest::paddingIsRandomTest()
{
	StartTrace(BlowfishTest.paddingIsRandomTest);

	BlowfishCBCScrambler bf("PadFront");
	for (long l = 0; l < 20; l++) {
		String topad;
		topad.Append("12345678901234567890", l);
		assertEqual(l, topad.Length());
		String padded1, padded2;
		bf.PadToBlockSize(padded1, topad);
		bf.PadToBlockSize(padded2, topad);
		t_assert(padded1.Length() > topad.Length());
		assertEqual(padded1.Length(), padded2.Length());
		assertEqual(0L, padded2.Length() % BF_BLOCK);
		assertEqual(padded1[0L] & 0xf, padded2[0L] & 0xf); // same padding length
		if (padded1[0L] != padded2[0L]) { // care for rare conditon that padding of length 1 had same upper 4 bits
			t_assertm(padded1 != padded2, "expect random padding");
		}
		t_assert(bf.UnPad(padded1));
		t_assert(bf.UnPad(padded2));
		assertEqual(topad, padded1);
		assertEqual(topad, padded2);
	}
}

void BlowfishTest::scrambleCarefullyPaddedString()
{
	StartTrace(BlowfishTest.scrambleCarefullyPaddedString);

	BlowfishCBCScrambler bfcbc("PadFront");
	BlowfishScrambler bf("PadTail");
	String aKey("a simple Key with some Bytes");
	bf.InitKey(aKey);
	bfcbc.InitKey(aKey);

	String padded((void *)"\00112345670123456\001", 16);
	String sfront = padded;
	String stail = padded;
	bfcbc.UnPad(sfront);
	bf.UnPad(stail);
	assertEqual(sfront.SubString(0, sfront.Length() - 1), stail.SubString(1));
	assertEqual(sfront.Length(), stail.Length());

	String scrambled;
	String cbcscrambled;
	bfcbc.DoEncode(cbcscrambled, padded.SubString(1));
	bf.DoEncode(scrambled, padded.SubString(0, 15));
	// with random padding the following is no longer true:
	//assertEqual(scrambled.SubString(0,8),cbcscrambled.SubString(0,8));
	t_assert(scrambled != cbcscrambled);
}
void BlowfishTest::scrambleSameKeyButDifferentIvec()
{
	StartTrace(BlowfishTest.scrambleSameKeyButDifferentIvec);
	Anything config;
	config["Key"] = "A simple Key";
	BlowfishCBCScrambler bcs0("zero");
	t_assert(bcs0.Init(config));
	config["InitVec0"] = 4711L;
	config["InitVec1"] = (long)0xabcdef12;
	BlowfishCBCScrambler bcsn("nonzero");
	t_assert(bcsn.Init(config));
	String text("This is a simple test Text to Scramble");
	String scr0, scrn;
	bcs0.DoEncode(scr0, text);
	bcsn.DoEncode(scrn, text);
	t_assert(scr0 != scrn); // doesn't test that scrambling is really different, because of random padding
	// but cross decode tests it:
	// cross decode should fail:
	String dec0n, decn0;
	bcs0.DoDecode(decn0, scrn); // might fail or succeed, but should be wrong
	bcsn.DoDecode(dec0n, scr0); // might fail or succeed, but should be wrong
	t_assert(dec0n != text);
	t_assert(decn0 != text);
	// own decode should succeed:
	String dec0, decn;
	t_assert(bcs0.DoDecode(dec0, scr0));
	t_assert(bcsn.DoDecode(decn, scrn));
	assertEqual(text, dec0);
	assertEqual(text, decn);
}

void BlowfishTest::cbcCrossPlatform()
{
	StartTrace(BlowfishTest.cbcCrossPlatform);
	Anything config;
	config["Key"] = "A simple Key";
	config["InitVec0"] = 4711L;
	config["InitVec1"] = (long)0xabcdef12;
	BlowfishCBCScrambler bcsn("nonzero");
	t_assert(bcsn.Init(config));
	long index;
	long lowbound =  0L;
	long highbound = 255L;
	String reference;
	Anything result;

	// Start cbcResult generation code
//	for ( index = highbound; index >= lowbound; index-- )
//	{
//		reference.Append((char) index);
//		String bfcbcEncoded;
//		bcsn.DoEncode(bfcbcEncoded,reference);
//		Base64 base64("base64");
//		String bfcbcBase64Encoded;
//		base64.DoEncode( bfcbcBase64Encoded, bfcbcEncoded );
//		result[index] = bfcbcBase64Encoded;
//	}
//	TraceAny(result,"result");
//	String resolvedFileName =  Coast::System::GetFilePath("CBCResult.any", (const char *)0);
//	Trace("resolved FileName: " << resolvedFileName);
//	ostream *os0 = Coast::System::OpenOStream(resolvedFileName,"", ios::out);
//	if ( os0 )
//	{
//		result.Export( *os0, 0 );
//		delete os0;
//	}
//	else
//	{
//		assertEqualm(true,false,"Problem writing reference file CBCResult.any.");
//	}
//	reference = "";
	// End cbcResult generation code

	Anything cbcResult;
	t_assert(Coast::System::LoadConfigFile(cbcResult, "CBCResult", "any"));
	t_assert(!cbcResult.IsNull());
	for ( index = highbound; index >= lowbound; index-- ) {
		reference.Append((char) index);
		Base64 base64("base64");
		String bfcbcBase64Encoded = cbcResult[index].AsString();
		String bfcbcEncoded, decodedString;
		base64.DoDecode( bfcbcEncoded, bfcbcBase64Encoded );
		bcsn.DoDecode(decodedString, bfcbcEncoded); // might fail or succeed, but should be wrong
		assertEqual(reference, decodedString);
		String strBuf1;
		OStringStream stream1(strBuf1);
		reference.DumpAsHex(stream1);
		Trace("Reference string: " << reference);
		String strBuf2;
		OStringStream stream2(strBuf2);
		decodedString.DumpAsHex(stream2);
		Trace("Decoded string  : " << decodedString);
	}
}

void BlowfishTest::scrambleLongerStringTheSame()
{
	StartTrace(BlowfishTest.scrambleLongerStringTheSame);

	String clearText("This is a simple test String");
	String differentClearText(clearText);
	differentClearText.PutAt(2L, 'u'); // "Thus is a simple test String"
	String key("A very simple test key");

	// regular ECB mode scrambling should produce the same encoding after the second block
	BlowfishScrambler bfscrambler("TestScrambler");
	bfscrambler.InitKey(key);
	String scrambledText, transcript;
	String scrambledText2, transcript2;
	bfscrambler.DoEncode(scrambledText, clearText);
	bfscrambler.DoEncode(scrambledText2, differentClearText);
	t_assert(scrambledText != scrambledText2);
	assertEqual(scrambledText2.Length(), scrambledText.Length());
	//now cut the first block and check if the rest is equal
	scrambledText.TrimFront(BF_BLOCK);
	scrambledText2.TrimFront(BF_BLOCK);
	assertEqual(scrambledText2, scrambledText);
	t_assert(bfscrambler.DoDecode(transcript, scrambledText));
	t_assert(bfscrambler.DoDecode(transcript2, scrambledText2));
	assertEqual(transcript, transcript2);
	differentClearText.TrimFront(BF_BLOCK);
	assertEqual(differentClearText, transcript);
}
void BlowfishTest::scrambleLongerStringDifferent()
{
	StartTrace(BlowfishTest.scrambleLongerStringDifferent);

	String clearText("This is a simple test String");
	String differentClearText(clearText);
	differentClearText.PutAt(2L, 'u'); // "Thus is a simple test String"
	String key("A very simple test key");

	// regular ECB mode scrambling should produce the same encoding after the second block
	BlowfishCBCScrambler bfcbc("TestCBCScrambler");
	bfcbc.InitKey(key);
	String scrambledText, transcript;
	String scrambledText2, transcript2;
	bfcbc.DoEncode(scrambledText, clearText);
	bfcbc.DoEncode(scrambledText2, differentClearText);
	assertEqual(clearText.Length(), differentClearText.Length());
	assertEqual(scrambledText.Length(), scrambledText2.Length());
	t_assert(scrambledText != scrambledText2);
	t_assert(bfcbc.DoDecode(transcript, scrambledText));
	t_assert(bfcbc.DoDecode(transcript2, scrambledText2));
	assertEqual(clearText, transcript);
	assertEqual(differentClearText, transcript2);
}

void BlowfishTest::scrambleUnscramble()
// what: check a few properties of a simple DoEncode-DoDecode cycle
{
	BlowfishScrambler bfscrambler("TestScrambler");
	BlowfishScrambler bfscrambler2("TestScrambler2");
	DoScrambleUnscramble(bfscrambler, bfscrambler2);
	BlowfishCBCScrambler bfcbc("TestCBCScrambler");
	BlowfishCBCScrambler bfcbc2("TestCBCScrambler2");
	DoScrambleUnscramble(bfcbc, bfcbc2);
} // scrambleUnscramble

void BlowfishTest::DoScrambleUnscramble(Scrambler &bfscrambler, Scrambler &bfscrambler2)
{

	String clearText("This is a simple test String");
	String key("A very simple test key");
	String otherKey("This key is really different");

	// BF one
	bfscrambler.InitKey(key);
	String scrambledText, transcript;

	// BF two
	bfscrambler2.InitKey(key);
	String scrambledText2, transcript2;

	bfscrambler.DoEncode(scrambledText, clearText);
	bfscrambler2.DoEncode(scrambledText2, clearText);

	// assertEqual("", scrambledText); // very binary, cannot assert
	t_assert(bfscrambler.DoDecode(transcript, scrambledText));
	assertEqual(clearText, transcript);

	t_assert(bfscrambler2.DoDecode(transcript2, scrambledText2));
	assertEqual(clearText, transcript2);

//        t_assert(memcmp(scrambledText, scrambledText2, scrambledText.Length()) == 0);
	// is be the same code for ECB Blowfischscrambler but shouldn't
	// CBC mode uses random padding, so it should never be the same

	// try it cross-wise
	// first scrambler must be able to unscramble second scramble
	String transcript3;
	t_assert(bfscrambler.DoDecode(transcript3, scrambledText2));
	assertEqual(clearText, transcript3);
	t_assert(clearText == transcript3);             // string comparison is more exact

	// and the other way round
	String transcript4;
	t_assert(bfscrambler2.DoDecode(transcript4, scrambledText));
	assertEqual(clearText, transcript4);

	// finally the second scrambler must also be able to unscramble
	String transcript5;
	t_assert(bfscrambler2.DoDecode(transcript5, scrambledText2));
	assertEqual(clearText, transcript5);
	t_assert(clearText == transcript5);             // string comparison is more exact
}

void BlowfishTest::differentKey()
// what: check a few properties of a simple DoEncode-DoDecode cycle
{
	BlowfishScrambler bfscrambler("TestScrambler");
	BlowfishScrambler bfscrambler2("TestScrambler2");
	DoDifferentKey(bfscrambler, bfscrambler2);
	BlowfishCBCScrambler bfcbc("TestScrambler");
	BlowfishCBCScrambler bfcbc2("TestScrambler2");
	DoDifferentKey(bfcbc, bfcbc2);
} // differentKey

void BlowfishTest::DoDifferentKey(Scrambler &bfscrambler, Scrambler &bfscrambler2)
{
	String clearText("This is a simple test String");
	String key("A very simple test key");
	// BF one
	bfscrambler.InitKey(key);
	String scrambledText, transcript;

	bfscrambler.DoEncode(scrambledText, clearText);

	long max = key.Length();

	for (long i = 0; i < max; i++) {
		String otherKey(key);
		otherKey.PutAt(i, 'X');         // modify one character in the key

		bfscrambler2.InitKey(otherKey);
		String scrambledText2, transcript2;

		bfscrambler2.DoEncode(scrambledText2, clearText);
		t_assert(scrambledText != scrambledText2);              // scramble must be different

		bfscrambler2.DoDecode(transcript2, scrambledText2);
		assertEqual(transcript2, clearText);
	}
}

void BlowfishTest::differentLength()
// what: DoEncode-DoDecode with different string sizes
{
	String key("A very simple test key");
	String otherKey("This key is really different");

	BlowfishScrambler bfscrambler("TestScrambler");
	bfscrambler.InitKey(key);
	String scrambledText, transcript;

	String sample("This is a sample text with 35 chars");
	long max = sample.Length();

	for (long i = 0; i <= max; i++) {
		String clearText = sample.SubString(0, i);      // strings with different sizes
		scrambledText = "";
		transcript = "";

		bfscrambler.DoEncode(scrambledText, clearText);
		t_assert(bfscrambler.DoDecode(transcript, scrambledText));
		assertEqual(clearText, transcript);
	} // for
	// during this test the emtpy string is tested
} // differentLength

void BlowfishTest::textWithNull()
// what: tests a string which contains null characters
{
	String key("A very simple test key");

	BlowfishScrambler bfscrambler("TestScrambler");
	bfscrambler.InitKey(key);
	String scrambledText, transcript;

	String sample("This is a sample text without null chars");
	// do not believe him, I will put in some later (the nasty programmer)
	long max = sample.Length();

	for (long i = 0; i < max; i++) {
		String clearText = sample;
		scrambledText = "";
		transcript = "";

		clearText.PutAt(i, '\0');               // try a 0 at every position

		bfscrambler.DoEncode(scrambledText, clearText);
		t_assert(bfscrambler.DoDecode(transcript, scrambledText));
		t_assert(clearText == transcript);      // must use String comparison
	} // for
} // testWithNull

void BlowfishTest::modifyScramble()
// what: modify a scramble and make sure it does not decode correctly
{
	String key("A very simple test key");

	BlowfishScrambler bfscrambler("TestScrambler");
	bfscrambler.InitKey(key);
	String scrambledText, transcript;

	String sample("This is a sample text without null chars");

	String clearText = sample;

	bfscrambler.DoEncode(scrambledText, clearText);

	long max = scrambledText.Length();
	long i;
	for (i = 0; i < max - 8; i++)
		// regular case, modification in encription part
	{
		scrambledText.PutAt(i, (char) i);               // modify the scramble

		t_assert(bfscrambler.DoDecode(transcript, scrambledText));
		t_assert(clearText != transcript);      // must not be the same
		assertEqual(clearText.Length(), transcript.Length());
	} // for

	for ( i = max - 8; i < max; i++)

		// excetional case, modification in the last block
	{
		// this more sensitive modification is tested more intensively
		for (long j = 0; j < 256; j++) {
			if (scrambledText[i] != (char)j) {              // real modification
				scrambledText.PutAt(i, (char) j);               // modify the scramble
				// we added a sanity check, so DoDecode might fail
				// we no longer assert that it is true
				bfscrambler.DoDecode(transcript, scrambledText);
				t_assert(clearText != transcript);      // must not be the same
				// assertEqual(clearText.Length(), transcript.Length());
				// it is pretty sure, that the length *will* change because
				// of the modification. Uncomment the line and see for yourself
				// the actual behavior remains unspecified, but it will *not* crash.
			} // if
		} // for
	} // for
} // modifyScramble

void BlowfishTest::rawECBCyphers()
{
// The input for this test comes from the SSLeavy implementation.
// It exercises the scrambling/unscrambling of 8 byte blocks
// in ECB mode.
	int i, n;
	BlowfishScrambler bfscrambler("TestScrambler");
	for (n = 0; n < NUM_TESTS; n++) {
		String key;
		key.Append(ecb_data[n], 8);
		bfscrambler.InitKey(key);

		unsigned char out[8];
		memcpy(out, &(plain_data[n][0]), 8);
		bfscrambler.DoECB_multipleBlockEncrypt(out, 8);

		t_assert(memcmp(&(cipher_data[n][0]), out, 8) == 0);
		if (memcmp(&(cipher_data[n][0]), out, 8) != 0) {
			assertEqualm(true, false, "Expected BF_ecb_encrypt blowfish error encrypting test to pass");
			printf("BF_ecb_encrypt blowfish error encrypting\n");
			printf("got     :");
			for (i = 0; i < 8; i++) {
				printf("%02X ", out[i]);
			}
			printf("\n");
			printf("expected:");
			for (i = 0; i < 8; i++) {
				printf("%02X ", cipher_data[n][i]);
			}
			printf("\n");
		}
		bfscrambler.DoECB_multipleBlockDecrypt(out, 8);
		t_assert(memcmp(&(plain_data[n][0]), &out, 8) == 0);
		if (memcmp(&(plain_data[n][0]), &out, 8) != 0) {
			assertEqualm(true, false, "Expected BF_ecb_encrypt error decrypting test to pass");
			printf("BF_ecb_encrypt error decrypting\n");
			printf("got     :");
			for (i = 0; i < 8; i++) {
				printf("%02X ", out[i]);
			}
			printf("\n");
			printf("expected:");
			for (i = 0; i < 8; i++) {
				printf("%02X ", plain_data[n][i]);
			}
			printf("\n");
		}
	}

//  Test the setting of the blowfish key. Again the
//  results to verify come from SSLeavy.

	for (n = 1; n < KEY_TEST_NUM; n++) {
		String key;
		key.Append(key_test, n);
		bfscrambler.InitKey(key);
		unsigned char out[8];
		memcpy(out, key_data, 8);
		bfscrambler.DoECB_multipleBlockEncrypt(out, 8);
		t_assert(memcmp(out, &(key_out[n-1][0]), 8) == 0);
		if (memcmp(out, &(key_out[n-1][0]), 8) != 0) {
			printf("Blowfish set key test error:\n");
			printf("got     :");
			for (i = 0; i < 8; i++) {
				printf("%02X ", out[i]);
			}
			printf("\n");
			printf("expected:");
			for (i = 0; i < 8; i++) {
				printf("%02X ", key_out[n-1][i]);
			}
			printf("\n");
		}
	}
}
