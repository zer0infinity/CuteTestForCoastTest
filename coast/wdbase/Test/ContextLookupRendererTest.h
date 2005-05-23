/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ContextLookupRendererTest_h_
#define _ContextLookupRendererTest_h_

#include "TestCase.h"
#include "Context.h"
#include "StringStream.h"

//---- ContextLookupRendererTest -----------------------------------------------------------

class ContextLookupRendererTest : public TestCase
{
public:
	ContextLookupRendererTest(TString tstrName);
	virtual ~ContextLookupRendererTest();

	static Test *suite ();
	void setUp ();
protected:
	void ContextCharPtr();
	void ContextLong();
	void ContextDouble();
	void ContextNull();
	void ContextCharPtrConfigArray();
	void ContextCharPtrDef();
	void ContextLongDef();
	void ContextDoubleDef();
	void ContextNullDef();
	void NestedLookup();
	void NestedLookupWithoutSlotnames();

	Context fContext;
	OStringStream fReply;
	Anything fConfig;
};

#endif
