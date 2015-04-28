/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * Copyright (c) 2015, David Tran, Faculty of Computer Science,
 * University of Applied Sciences Rapperswil (HSR),
 * 8640 Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ROSimpleAnythingTest_H
#define _ROSimpleAnythingTest_H

#include "cute.h"
#include "string"

class ROSimpleAnythingTest {
public:
	static void runAllTests(cute::suite &s);
	void ConstructorsAndConversions();
	void AssignmentsAndConversions();

	void EmptyConstructor();
	void AnyConstructor();
	void AnyIntConstructor();
	void AnyBoolConstructor();
	void AnyLongConstructor();
	void AnyFloatConstructor();
	void AnyDoubleConstructor();
	void AnyIFAObjConstructor();
};

#endif		//not defined _ROSimpleAnythingTest_H
