/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RendererTest_h_
#define _RendererTest_h_

#include "TestCase.h"

#include "Anything.h"
#include "Context.h"
#include "StringStream.h"

//---- RendererTest -----------------------------------------------------------
class RendererTest : public TestCase
{
public:
	RendererTest(TString name);
	virtual ~RendererTest();

	void setUp ();
	static Test *suite ();

	virtual void printResult( const char *str0, const char *str1 );
	virtual void printReplyBody( const char *MethodeName );

protected:
	Context fContext;
	OStringStream fReply;
	Anything fConfig;
};

#endif
