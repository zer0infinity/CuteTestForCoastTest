/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "NTLMCryptTest.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "NTLMCoder.h"
#include "NTLMAuthMessage.h"
//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- NTLMCryptTest ----------------------------------------------------------------
NTLMCryptTest::NTLMCryptTest(TString name) : TestCase(name)
{
	StartTrace(NTLMCryptTest.Ctor);
}

NTLMCryptTest::~NTLMCryptTest()
{
	StartTrace(NTLMCryptTest.Dtor);
}

// setup for this TestCase
void NTLMCryptTest::setUp ()
{
	StartTrace(NTLMCryptTest.setUp);
}

void NTLMCryptTest::tearDown ()
{
	StartTrace(NTLMCryptTest.tearDown);
}

void NTLMCryptTest::DecodeClientMsg()
{
	StartTrace(NTLMCryptTest.DecodeClientMsg);
	String msgb64("TlRMTVNTUAABAAAAA7IAAAoACgApAAAACQAJACAAAABMSUdIVENJVFlVUlNBLU1JTk9S");
	String host;
	String domain;
	t_assert(NTLMCoder::DecodeClientMsg(msgb64, domain, host));
	assertEqual("URSA-MINOR", domain);
	assertEqual("LIGHTCITY", host);
}
void NTLMCryptTest::EncodeClientMsg()
{
	StartTrace(NTLMCryptTest.EncodeClientMsg);
	String expected("TlRMTVNTUAABAAAAA7IAAAoACgApAAAACQAJACAAAABMSUdIVENJVFlVUlNBLU1JTk9S");
	String host("LightCity");
	String domain("URSA-minor");
	String msgb64 = NTLMCoder::EncodeClientMsg(domain, host);
	assertEqual(expected, msgb64);
	t_assert(NTLMCoder::DecodeClientMsg(msgb64, domain, host));
	assertEqual("URSA-MINOR", domain);
	assertEqual("LIGHTCITY", host);

	host = "myhost";
	domain = "myDomain";
	msgb64 = NTLMCoder::EncodeClientMsg(domain, host);
	t_assert(NTLMCoder::DecodeClientMsg(msgb64, domain, host));
	assertEqual("MYHOST", host);
	assertEqual("MYDOMAIN", domain);

}
void NTLMCryptTest::EncodeServerNonce()
{
	StartTrace(NTLMCryptTest.EncodeServerNonce);
	String expected("TlRMTVNTUAACAAAAAAAAACgAAAABggAAU3J2Tm9uY2UAAAAAAAAAAA==");

	String msgb64 = NTLMCoder::EncodeServerNonce("SrvNonce");
	assertEqual(expected, msgb64);
}
void NTLMCryptTest::DecodeServerNonce()
{
	StartTrace(NTLMCryptTest.DecodeServerNonce);
	String msgb64("TlRMTVNTUAACAAAAAAAAACgAAAABggAAU3J2Tm9uY2UAAAAAAAAAAA==");
	String nonce;
	t_assert(NTLMCoder::DecodeServerNonce(msgb64, nonce));
	assertEqual("SrvNonce", nonce);
	msgb64 = NTLMCoder::EncodeServerNonce("12345678");
	t_assert(NTLMCoder::DecodeServerNonce(msgb64, nonce));
	assertEqual("12345678", nonce);
}
void NTLMCryptTest::DecodeResponse()
{
	StartTrace(NTLMCryptTest.DecodeResponse);
	String msgb64("TlRMTVNTUAADAAAAGAAYAHIAAAAYABgAigAAA"
				  "BQAFABAAAAADAAMAFQAAAASABIAYAAAAAAAAACiAAAAAYIAAFU"
				  "AUgBTAEEALQBNAEkATgBPAFIAWgBhAHAAaABvAGQAT"
				  "ABJAEcASABUAEMASQBUAFkArYfKbe/jRoW5xDxHeox"
				  "C1gBmfWiS5+iX4OAN4xBKG/IFPwfH3agtPEia6YnhsADT");
	String host;
	String domain;
	String user;
	String password("Beeblebrox");
	String lmhash(NTLMCoder::EncodeLMPassword(password));
	String nthash(NTLMCoder::EncodeNTPassword(password));
	t_assert(NTLMCoder::DecodeResponse(msgb64, "SrvNonce", lmhash, nthash, domain, host, user));
	assertEqual("URSA-MINOR", domain);
	assertEqual("LIGHTCITY", host);
	assertEqual("Zaphod", user);
}
void NTLMCryptTest::EncodeResponse()
{
	StartTrace(NTLMCryptTest.EncodeResponse);
	String expect("TlRMTVNTUAADAAAAGAAYAHIAAAAYABgAigAAA"
				  "BQAFABAAAAADAAMAFQAAAASABIAYAAAAAAAAACiAAAAAYIAAFU"
				  "AUgBTAEEALQBNAEkATgBPAFIAWgBhAHAAaABvAGQAT"
				  "ABJAEcASABUAEMASQBUAFkArYfKbe/jRoW5xDxHeox"
				  "C1gBmfWiS5+iX4OAN4xBKG/IFPwfH3agtPEia6YnhsADT");
	String host("LightCity");
	String domain("URSA-minor");
	String user("Zaphod");
	String password("Beeblebrox");
	String lmhash(NTLMCoder::EncodeLMPassword(password));
	String nthash(NTLMCoder::EncodeNTPassword(password));
	String result = NTLMCoder::EncodeResponse("SrvNonce", lmhash, nthash, domain, host, user);
	assertEqual(expect, result);
	t_assert(NTLMCoder::DecodeResponse(result, "SrvNonce", lmhash, nthash, domain, host, user));
	assertEqual("URSA-MINOR", domain);
	assertEqual("LIGHTCITY", host);
	assertEqual("Zaphod", user);

	host = "myhost";
	domain = "myDomain";
	user = "it's me";//'
	password = "geheim";
	lmhash = NTLMCoder::EncodeLMPassword(password);
	nthash = NTLMCoder::EncodeNTPassword(password);
	result = NTLMCoder::EncodeResponse("12345678", lmhash, nthash, domain, host, user);
	t_assert(NTLMCoder::DecodeResponse(result, "12345678", lmhash, nthash, domain, host, user));
	assertEqual("MYDOMAIN", domain);
	assertEqual("MYHOST", host);
	assertEqual("it's me", user); //'

}

void NTLMCryptTest::EncodeLMPassword()
{
	StartTrace(NTLMCryptTest.EncodeLMPassword);
	unsigned char expected[21] = {
		'\x91', '\x90', '\x16', '\xf6', '\x4e', '\xc7', '\xb0', '\x0b',
		'\xa2', '\x35', '\x02', '\x8c', '\xa5', '\x0c', '\x7a', '\x03',
		'\x00', '\x00', '\x00', '\x00', '\x00'
	};
	String expect((void *)expected, 21L);
	String result = NTLMCoder::EncodeLMPassword("Beeblebrox");
	assertEqualRaw(expect, result);
}

void NTLMCryptTest::EncodeNTPassword()
{
	StartTrace(NTLMCryptTest.EncodeNTPassword);
	unsigned char expected[21] = {
		'\x8c', '\x1b', '\x59', '\xe3', '\x2e', '\x66', '\x6d', '\xad',
		'\xf1', '\x75', '\x74', '\x5f', '\xad', '\x62', '\xc1', '\x33',
		'\x00', '\x00', '\x00', '\x00', '\x00'
	};
	String expect((void *)expected, 21L);
	String result = NTLMCoder::EncodeNTPassword("Beeblebrox");
	assertEqualRaw(expect, result);
}

void NTLMCryptTest::MakeUtf16()
{
	StartTrace(NTLMCryptTest.MakeUtf16);
	unsigned char expected[] = {
		'A', '\0', 'd', '\0', 'u', '\0', 'm', '\0',
		'b', '\0', 'T', '\0', 'h', '\0', 'i', '\0',
		'n', '\0', 'g', '\0'
	};
	String expect((void *)expected, 20L);
	const char *input = "AdumbThing";
	String result = NTLMCoder::MakeUtf16("AdumbThing");
	assertEqualRaw(expect, result);
	assertEqual(2 * strlen(input), result.Length());
}
void NTLMCryptTest::ClientMsgFactory()
{
	String buffer;
	t_assert(NTLMCoder::DecodeBase64(buffer, "TlRMTVNTUAABAAAAA7IAAAoACgApAAAACQAJACAAAABMSUdIVENJVFlVUlNBLU1JTk9S"));
	NTLMAuthMessage *m = NTLMAuthMessage::MakeMessage(buffer);
	assertEqual(1, m->GetMessageType());
	assertEqual(0xb203, m->GetFlags());
	assertEqual("URSA-MINOR", m->GetDomain());
	assertEqual("LIGHTCITY", m->GetHost());
	assertEqual("", m->GetUser());
	assertEqual("", m->GetNonce());
	buffer = "";
	delete m;
	t_assert(NTLMCoder::DecodeBase64(buffer, "TlRMTVNTUAADAAAAGAAYAHIAAAAYABgAigAAA"
									 "BQAFABAAAAADAAMAFQAAAASABIAYAAAAAAAAACiAAAAAYIAAFU"
									 "AUgBTAEEALQBNAEkATgBPAFIAWgBhAHAAaABvAGQAT"
									 "ABJAEcASABUAEMASQBUAFkArYfKbe/jRoW5xDxHeox"
									 "C1gBmfWiS5+iX4OAN4xBKG/IFPwfH3agtPEia6YnhsADT"));
	m = NTLMAuthMessage::MakeMessage(buffer);
	assertEqual(3, m->GetMessageType());
	assertEqual(0x8201, m->GetFlags());
	assertEqual("", m->GetNonce());
	assertEqual("URSA-MINOR", m->GetDomain());
	assertEqual("LIGHTCITY", m->GetHost());
	assertEqual("Zaphod", m->GetUser());

}

// builds up a suite of testcases, add a line for each testmethod
Test *NTLMCryptTest::suite ()
{
	StartTrace(NTLMCryptTest.suite);
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, NTLMCryptTest, DecodeClientMsg);
	ADD_CASE(testSuite, NTLMCryptTest, EncodeClientMsg);
	ADD_CASE(testSuite, NTLMCryptTest, EncodeServerNonce);
	ADD_CASE(testSuite, NTLMCryptTest, DecodeServerNonce);
	ADD_CASE(testSuite, NTLMCryptTest, EncodeLMPassword);
	ADD_CASE(testSuite, NTLMCryptTest, EncodeNTPassword);
	ADD_CASE(testSuite, NTLMCryptTest, MakeUtf16);
	ADD_CASE(testSuite, NTLMCryptTest, DecodeResponse);
	ADD_CASE(testSuite, NTLMCryptTest, EncodeResponse);
	ADD_CASE(testSuite, NTLMCryptTest, ClientMsgFactory);

	return testSuite;
}
