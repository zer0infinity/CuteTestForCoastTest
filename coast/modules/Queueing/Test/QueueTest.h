/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _QueueTest_H
#define _QueueTest_H

#include "TestCase.h"

//---- QueueTest ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class QueueTest : public testframework::TestCase
{
public:
	//--- constructors

	/*! \param name name of the test */
	QueueTest(TString tstrName);

	//! destroys the test case
	~QueueTest();

	//--- public api

	//! builds up a suite of testcases for this test
	static Test *suite ();

	//! describe this testcase
	void BlockingSideTest();
	void PutGetStatusTest();
	void SimplePutGetTest();
	void MultiProducerSingleConsumerTest();
	void SingleProducerMultiConsumerTest();
	void ConsumerTerminationTest();
	void SimpleTypeAnyStorageQueueTest();
	void SimpleTypeListStorageQueueTest();
	void QueueWithAllocatorTest();
	void QueueTypePerfTest();
	void SingleProducerMultiConsumerQTypeTest();

private:
	void DoMultiProducerSingleConsumerTest(long lQueueSize);
	void DoSingleProducerMultiConsumerTest(long lQueueSize);
};

#endif
