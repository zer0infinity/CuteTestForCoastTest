/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestProcessorTest_H
#define _RequestProcessorTest_H

//---- baseclass include -------------------------------------------------
#include "TestCase.h"
#include "RequestProcessor.h"

//---- RequestProcessorTest ----------------------------------------------------------
//!test cases for RequestProcessor
class RequestProcessorTest : public TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RequestProcessorTest(TString name);

	//!destroys the test case
	~RequestProcessorTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

	//!sets the environment for this test
	void setUp ();

	//!deletes the environment for this test
	void tearDown ();

	//!test initialization of request processor
	void InitTest();

	//!test processing of request with stub objects
	void ProcessRequestTest();
};

//! policy object shared by all threads to handle a request message;
class LoopbackProcessor : public RequestProcessor
{
public:
	//!named object shared by all requests
	LoopbackProcessor(const char *processorName);
	virtual ~LoopbackProcessor()	{ }

	//! support for prototypes is required
	virtual IFAObject *Clone() const {
		return new LoopbackProcessor(fName);
	}

protected:
	//!read the input arguments from the stream and generate an anything
	virtual void DoReadInput(iostream &Ios, Context &ctx);

	//!process the arguments and generate a reply
	virtual void DoProcessRequest(ostream &reply, Context &ctx);

private:
	LoopbackProcessor(const LoopbackProcessor &);
	LoopbackProcessor &operator=(const LoopbackProcessor &);
};

#endif
