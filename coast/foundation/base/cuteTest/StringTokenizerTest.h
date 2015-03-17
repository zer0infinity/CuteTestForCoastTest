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

#ifndef _StringTokenizerTest_H
#define _StringTokenizerTest_H

#include "cute.h"
#include "ide_listener.h"
#include "cute_runner.h"
#include "ITOString.h"//lint !e537

class StringTokenizerTest {
protected:
	String fShort;
	String fLong;

public:
	StringTokenizerTest();
	static void runAllTests(cute::suite &s);

	void constrMethodsAll();
	void constrMethods0();
	void constrMethods1();
	void constrMethods2();
	void constrMethods3();
	void constrMethods4();
	void constrMethods5();
	void constrMethods6();
	void constrMethods7();
	void constrMethods8();
	void constrMethods9();
	void constrMethods10();
	void constrMethods11();
	void constrMethods12();
	void constructors();
	void nextTokenNormal();
	void nextTokenEmpty();
	void resetTest();
	void getRemainder();
};

#endif

