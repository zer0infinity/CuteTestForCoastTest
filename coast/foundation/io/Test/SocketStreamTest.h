/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SocketStreamTest_H
#define _SocketStreamTest_H

#include "FoundationTestTypes.h"//lint !e537

class Connector;

//---- SocketStreamTest -----------------------------------------------------------
//!testcases for the SocketStream class
class SocketStreamTest : public TestFramework::TestCaseWithConfig
{
public:
	SocketStreamTest(TString tstrName);

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
};

#endif
