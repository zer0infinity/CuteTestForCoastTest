/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ActionCoreTest_H
#define _ActionCoreTest_H

#include "TestCase.h"

class ActionCoreTest: public TestFramework::TestCase {
public:
	ActionCoreTest(TString tstrName);
	~ActionCoreTest();
	static Test *suite();
	void ConsistentTransitionHandling();
	void EmptyConfigurationTransitionHandling();
	void ConfiguredTransitionHandling();
	void ActionSequence();
	void AbortedActionSequence();
};

#endif
