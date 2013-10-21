/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RequestProcessorTest_H
#define _RequestProcessorTest_H

#include "TestCase.h"
#include "RequestProcessor.h"

class RequestProcessorTest: public testframework::TestCase {
public:
	RequestProcessorTest(TString tstrName) :
		TestCaseType(tstrName) {
	}
	static Test *suite();
	void InitTest();

	//!test processing of request with stub objects
	void ProcessRequestTest();
	void tearDown();
};

//! policy object shared by all threads to handle a request message;
class LoopbackProcessor: public RequestProcessor {
public:
	//!named object shared by all requests
	LoopbackProcessor(const char *processorName);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) LoopbackProcessor(fName);
	}

protected:
	//!read the input arguments from the stream and generate an anything
	virtual bool DoReadInput(std::iostream &Ios, Context &ctx);

	//!process the arguments and generate a reply
	virtual bool DoProcessRequest(std::ostream &reply, Context &ctx);

private:
	LoopbackProcessor(const LoopbackProcessor &);
	LoopbackProcessor &operator=(const LoopbackProcessor &);
};

#endif
