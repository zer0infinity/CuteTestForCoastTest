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
class RequestProcessorTest : public TestFramework::TestCase
{
public:
	//--- constructors

	//!TestCase constructor
	//! \param name name of the test
	RequestProcessorTest(TString tstrName);

	//!destroys the test case
	~RequestProcessorTest();

	//!builds up a suite of testcases for this test
	static Test *suite ();

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

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) LoopbackProcessor(fName);
	}

protected:
	//!read the input arguments from the stream and generate an anything
	virtual void DoReadInput(std::iostream &Ios, Context &ctx);

	//!process the arguments and generate a reply
	virtual void DoProcessRequest(std::ostream &reply, Context &ctx);

private:
	LoopbackProcessor(const LoopbackProcessor &);
	LoopbackProcessor &operator=(const LoopbackProcessor &);
};

#endif
