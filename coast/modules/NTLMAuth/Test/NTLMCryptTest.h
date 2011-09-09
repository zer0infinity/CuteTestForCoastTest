/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NTLMCryptTest_H
#define _NTLMCryptTest_H

#include "TestCase.h"

class NTLMCryptTest: public TestFramework::TestCase {
public:
	NTLMCryptTest(TString tstrName) :
			TestCaseType(tstrName) {
	}

	static Test *suite();

	void DecodeClientMsg();
	void EncodeClientMsg();
	void EncodeServerNonce();
	void DecodeServerNonce();
	void DecodeResponse();
	void EncodeResponse();
	void EncodeLMPassword();
	void EncodeNTPassword();
	void MakeUtf16();
	void ClientMsgFactory();
};

#endif
