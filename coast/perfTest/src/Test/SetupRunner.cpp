/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "TestRunner.h"
#include "RemoteStresserTest.h"
#include "HTTPFlowControllerPrepareTest.h"
#include "FlowControllerTest.h"
#include "HTMLComparerTest.h"
#include "NewRendererTest.h"
#include "HTMLParserTest.h"

void setupRunner(TestRunner &runner) {
	// Use this work suite to debug a failing TestCase
	ADD_SUITE(runner, StressAppTest);
	ADD_SUITE(runner, RemoteStresserTest);
	ADD_SUITE(runner, FlowControllerTest);
	ADD_SUITE(runner, HTTPFlowControllerTest);
	ADD_SUITE(runner, HTTPFlowControllerPrepareTest);
	ADD_SUITE(runner, ConfiguredActionTest);
	ADD_SUITE(runner, HTMLComparerTest);
	ADD_SUITE(runner, NewRendererTest);
	ADD_SUITE(runner, HTMLParserTest);
} // setupRunner
