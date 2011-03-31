/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConnectorAcceptorTest_h_
#define _ConnectorAcceptorTest_h_

#include "FoundationTestTypes.h"

//---- ConnectorAcceptorTest ---------------------------------------------------
class EBCDICEchoMsgCallBack;
class AcceptorThread;

class ConnectorAcceptorTest: public TestFramework::TestCaseWithConfig {
public:
	ConnectorAcceptorTest(TString tstrName);
	virtual ~ConnectorAcceptorTest();

	virtual void setUp();
	virtual void tearDown();
	static Test *suite();
	void basicOperation();
	void basicOperationWithAllocator();
	void differentReply();

protected:
	EBCDICEchoMsgCallBack *fCallBack;
	AcceptorThread *fAcceptorThread;
};
#endif
