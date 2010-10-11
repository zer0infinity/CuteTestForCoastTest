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
#include "MD5.h"

//--- interface include --------------------------------------------------------
#include "MD5Test.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"

//--- c-library modules used ---------------------------------------------------
#include <iomanip>

MD5Test::MD5Test(TString tname) : TestCaseType(tname)
{};

MD5Test::~MD5Test() {}

Test *MD5Test::suite ()
/* what: return the whole suite of tests for renderers, we could add the suites
		 of the derived classes here, but we would need to include all the headers
		 of the derived classes which is not nice.
		 Add cases for the generic Renderer here (if you find any that is);
*/
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, MD5Test, hashCheck);
	ADD_CASE(testSuite, MD5Test, signCheck);
	return testSuite;
}

String hexhash(MD5Signer &md5, const char *s)
{
	String hash;
	md5.DoHash(s, hash);
	OStringStream os;
	for (long i = 0 ; i < hash.Length(); i++) {
		os << std::hex <<  std::setw(2) << std::setfill('0') << (int)(unsigned char)hash[i];
	}
	return os.str();
}

void MD5Test::hashCheck()
{
	MD5Signer md5("HashTester");

	assertEqual("d41d8cd98f00b204e9800998ecf8427e", hexhash(md5, ""));
	assertEqual("0cc175b9c0f1b6a831c399e269772661", hexhash(md5, "a"));
	assertEqual("900150983cd24fb0d6963f7d28e17f72", hexhash(md5, "abc"));
	assertEqual("f96b697d7cb7938d525a2f31aaf161d0", hexhash(md5, "message digest"));
	assertEqual("c3fcd3d76192e4007dfb496cca67e13b", hexhash(md5, "abcdefghijklmnopqrstuvwxyz"));
	assertEqual("d174ab98d277d9f5a5611c2c9f419d9f", hexhash(md5, "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789"));
	assertEqual("57edf4a22be3c955ac49da2e2107b67a", hexhash(md5, "12345678901234567890123456789012345678901234567890123456789012345678901234567890"));

	// ifs Tests:
	assertEqual("a90bc35ba003927a6dcceb9ca29de0c4", hexhash(md5, "ABCDEXYZabcdexyz"));
	assertEqual("e807f1fcf82d132f9bb018ca6738a19f", hexhash(md5, "1234567890"));
	assertEqual("5c2b6f5cd9864fba02f6c0581c31404a", hexhash(md5, "-,.*="));
	assertEqual("ff848a8525e4859a924b7108811052bf", hexhash(md5, "CH10601-tktestx1,172.016.003.077,20000126141759,systempw"));

// official MD5 testcases
//MD5 ("") = d41d8cd98f00b204e9800998ecf8427e
//MD5 ("a") = 0cc175b9c0f1b6a831c399e269772661
//MD5 ("abc") = 900150983cd24fb0d6963f7d28e17f72
//MD5 ("message digest") = f96b697d7cb7938d525a2f31aaf161d0
//MD5 ("abcdefghijklmnopqrstuvwxyz") = c3fcd3d76192e4007dfb496cca67e13b
//MD5 ("ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789") =
//d174ab98d277d9f5a5611c2c9f419d9f
//MD5 ("12345678901234567890123456789012345678901234567890123456789012345678901234567890") = 57edf4a22be3c955ac49da2e2107b67a
}

void MD5Test::signCheck()
// what: check a few sign check cycles for correctness
{
	MD5Signer md5Signer("TestSigner");
	String clearText("This is a simple test String");
	String key("A very simple test key");
	String digestedText, transcript;

	md5Signer.DoSign(key, digestedText, clearText);
	assertEqual(clearText, digestedText.SubString(16, clearText.Length()));
	// the first 16 byte are the digest value
	t_assert(md5Signer.DoCheck(key, transcript, digestedText));
	assertEqual(clearText, transcript);

	MD5Signer md5Signer2("Signer2");	// second signer must produce the same string
	String digestedText2, transcript2;
	md5Signer2.DoSign(key, digestedText2, clearText);
	t_assert(memcmp(digestedText, digestedText2, digestedText.Length()) == 0);
	t_assert(md5Signer.DoCheck(key, transcript2, digestedText2));
	assertEqual(clearText, transcript2);

	// try to modify the digestedText, Check must fail
	long max = digestedText.Length();
	assertEqual(45, max);
	bool ret;
	String transcript3;

	for (long i = 0; i < max; i++) {
		digestedText2 = digestedText;
		digestedText2.PutAt(i, digestedText2[i] - 1);
		ret = md5Signer.DoCheck(key, transcript3, digestedText2);
		assertEqual("", transcript3);		// must not change transcript
		t_assert(!ret);
		if (ret) {
			assertEqual(-1, i);
		}
	} // for

	// try a transposition
	digestedText2 = digestedText;
	char c = digestedText2[10L];

	digestedText2.PutAt(10, digestedText2[11L]);
	digestedText2.PutAt(11, c);
	t_assert(!md5Signer.DoCheck(key, transcript3, digestedText2));
	assertEqual("", transcript3);		// must not change transcript

	// reverse transposition
	c = digestedText2[10L];
	digestedText2.PutAt(10, digestedText2[11L]);
	digestedText2.PutAt(11, c);
	t_assert(md5Signer.DoCheck(key, transcript3, digestedText2));
	assertEqual(transcript, transcript3);		// must the same as original
} // signCheck
