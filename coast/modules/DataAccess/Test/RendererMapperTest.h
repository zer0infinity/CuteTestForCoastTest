/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RendererMapperTest_H
#define _RendererMapperTest_H

//---- TestCase include -------------------------------------------------
#include "TestCase.h"
#include "Anything.h"

//---- RendererMapperTest ----------------------------------------------------------
//!test mapper moved to renderer use via RendererMapper
//! further explanation of the purpose of the class
//! this may contain <B>HTML-Tags</B>
//! ...
class RendererMapperTest : public TestCase
{
public:
	//--- constructors
	RendererMapperTest(TString name);
	~RendererMapperTest();

	//--- public api
	//!single line description of newmethod
	//! further explanation of the purpose of the method
	//! this may contain <B>HTML-Tags</B>
	//! ...
	//! \param aParam explanation of aParam (important : paramname - explanation )
	//! \return explanation of return value
	//! \pre explanation of precondition for the method call
	//! \post explanation of postcondition for the method call
	static Test *suite ();
	void setUp();
	void StdGetTest();
	void GetOnAnyTest();

protected:
	//--- subclass api

	//--- member variables declaration
	long fMember;
	static long fgMember;

	//--- member variables declaration
	Anything fStdContextAny;
};

#endif
