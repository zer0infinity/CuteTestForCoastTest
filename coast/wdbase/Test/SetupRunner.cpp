/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TestRunner.h"
#include "TestSuite.h"

//--- standard modules used ----------------------------------------------------

//--- test cases ---------------------------------------------------------------
#include "RegistryTest.h"
#include "ContextTest.h"
#include "SessionTest.h"
#include "RoleTest.h"
#include "PageTest.h"
#include "MTStorageTest.h"
#include "URLFilterTest.h"
#include "ServerTest.h"
#include "MasterServerTest.h"
#include "TransitionTests.h"
#include "WDModuleTest.h"
#include "ActionCoreTest.h"
#include "SimpleListenerPoolTest.h"
#include "InterruptHandlerTest.h"
#include "AnythingUtilsTest.h"
#include "RequestProcessorTest.h"
#include "SessionListManagerTest.h"
#include "PeriodicActionTest.h"
#include "ServiceDispatcherTest.h"
#include "RendererDispatcherTest.h"
#include "AppBooterTest.h"
#include "RegistryIteratorTest.h"
#include "LFListenerPoolTest.h"
#include "LocalizationUtilsTest.h"
#include "LogTimerTest.h"
#include "RequestBlockerTest.h"
#include "URLEncoderTest.h"
#include "SecurityModuleTest.h"
#include "LocalizedStringsTest.h"
#include "BasicRendererTest.h"
#include "ROAnyLookupAdapterTest.h"
#include "ROAnyConfNamedObjectLookupAdapterTest.h"
#include "HTTPChunkedOStreamTest.h"
#include "HTTPStreamStackTest.h"
#include "ContextLookupRendererTest.h"
#include "PageNameRendererTest.h"
#include "WebAppServiceTest.h"
#include "ThreadedTimeStampTest.h"
#include "ConfiguredLookupAdapterTest.h"

#if defined(WIN32)
#include "config_bases.h"
void bases()
{
	RegisterBasesObjs();
}
#endif

void setupRunner(TestRunner &runner)
{
	ADD_SUITE(runner, ThreadedTimeStampTest);
	ADD_SUITE(runner, ActionCoreTest);
	ADD_SUITE(runner, InterruptHandlerTest);
	ADD_SUITE(runner, LocalizedStringsTest);
	ADD_SUITE(runner, LFListenerPoolTest);
	ADD_SUITE(runner, ServerTest);
	ADD_SUITE(runner, MasterServerTest);
	ADD_SUITE(runner, RoleTest);
	ADD_SUITE(runner, SessionTest);
	ADD_SUITE(runner, TransitionTests);
	ADD_SUITE(runner, ContextTest);
	ADD_SUITE(runner, URLFilterTest);
	ADD_SUITE(runner, AnythingUtilsTest);
	ADD_SUITE(runner, ServiceDispatcherTest);
	ADD_SUITE(runner, WebAppServiceTest);
	ADD_SUITE(runner, SimpleListenerPoolTest);
	ADD_SUITE(runner, AppBooterTest);
	ADD_SUITE(runner, BasicRendererTest);
	ADD_SUITE(runner, ContextLookupRendererTest);
	ADD_SUITE(runner, HTTPChunkedOStreamTest);
	ADD_SUITE(runner, HTTPStreamStackTest);
	ADD_SUITE(runner, LocalizationUtilsTest);
	ADD_SUITE(runner, LogTimerTest);
	ADD_SUITE(runner, MTStorageTest);
	ADD_SUITE(runner, PageNameRendererTest);
	ADD_SUITE(runner, PageTest);
	ADD_SUITE(runner, PeriodicActionTest);
	ADD_SUITE(runner, RegistryIteratorTest);
	ADD_SUITE(runner, RegistryTest);
	ADD_SUITE(runner, RendererDispatcherTest);
	ADD_SUITE(runner, RequestBlockerTest);
	ADD_SUITE(runner, RequestProcessorTest);
	ADD_SUITE(runner, ROAnyLookupAdapterTest);
	ADD_SUITE(runner, ROAnyConfNamedObjectLookupAdapterTest);
	ADD_SUITE(runner, ConfiguredLookupAdapterTest);
	ADD_SUITE(runner, SecurityModuleTest);
	ADD_SUITE(runner, URLEncoderTest);
	ADD_SUITE(runner, SessionListManagerTest);
	ADD_SUITE(runner, WDModuleTest);
} // setupRunner
