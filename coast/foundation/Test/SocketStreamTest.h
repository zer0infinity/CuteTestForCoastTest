/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SocketStreamTest_H
#define _SocketStreamTest_H

#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
class EXPORTDECL_FOUNDATION Connector;
class EXPORTDECL_FOUNDATION String;

//---- SocketStreamTest -----------------------------------------------------------
//!testcases for the SocketStream class
class SocketStreamTest : public TestCase
{
public:
	SocketStreamTest(TString name);
	virtual ~SocketStreamTest();

	static Test *suite ();
	void setUp ();
	void tearDown ();

	// TestCases
	void simpleRead();
	void simpleWrite();
	void timeoutTest();
	void parseHTTPReplyTest();
	void opLeftShiftTest();

protected:

	void parseParams(String &line, Anything &request);

private:
	Connector	*fConnector;
	Anything	fConfig;
};

#endif
