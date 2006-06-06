/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "Context.h"
#include "Renderer.h"

//--- test modules used --------------------------------------------------------
#include "TestSuite.h"

//--- module under test --------------------------------------------------------
#include "ConditionalRenderers.h"

//--- interface include --------------------------------------------------------
#include "SwitchRendererTest.h"

SwitchRendererTest::SwitchRendererTest (TString tname) : RendererTest(tname),
	fSwitchRenderer("SwitchRendererTest") {};
SwitchRendererTest::~SwitchRendererTest() {};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void SwitchRendererTest::setUp ()
{
	RendererTest::setUp();
}

Test *SwitchRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, SwitchRendererTest, TestCaseA);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseB);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseC);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseNotFoundWithDefault);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseNotFoundWithoutDefault);

	ADD_CASE(testSuite, SwitchRendererTest, TestCaseEmptyKeyWithDefault);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseEmptyKeyWithIsEmpty);

	ADD_CASE(testSuite, SwitchRendererTest, TestCaseAWithoutCases);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseBWithoutCases);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseCWithoutCases);

	ADD_CASE(testSuite, SwitchRendererTest, TestCaseAWithoutContextLookupName);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseBWithoutContextLookupName);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseCWithoutContextLookupName);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseNotFoundWithoutContextLookupName);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseLookupEmpty);

	return testSuite;

}

/*===============================================================*/
/*     Check found cases where all is correctly defined          */
/*===============================================================*/
void SwitchRendererTest::ConfigureCases( const char *cases )
{
	fConfig["ContextLookupName"] = "CaseKey";	// name

	fConfig["Case"]["CaseA"] = "That is case A";
	fConfig["Case"]["CaseB"] = "That is case B";
	fConfig["Case"]["CaseC"] = "That is case C";
	fConfig["Default"] = "That is Default";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["CaseKey"] = cases; // = CaseA,...  = s
}

void SwitchRendererTest::TestCaseA()
{
	SwitchRenderer switchRenderer("");

	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseA" );
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseA"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseB()
{
	SwitchRenderer switchRenderer("");

	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseB" );
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseB"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseC()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseC" );
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseC"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseNotFoundWithDefault()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseX" );
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

/*===============================================================*/
/*     Check case where Default is not defined                   */
/*===============================================================*/
void SwitchRendererTest::TestCaseNotFoundWithoutDefault()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseX" );
	// Remove the "Default" action
	fConfig.Remove("Default");
	// Check if the "Default" action is really removed
	t_assert( fConfig.IsDefined("Default") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

/*===============================================================*/
/*     Check case where all is defined and "key" is ""           */
/*===============================================================*/
void SwitchRendererTest::TestCaseEmptyKeyWithDefault()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "" );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseEmptyKeyWithIsEmpty()
{
	// _IsEmpty is programmed for SwitchRenderer and NOT for SwitchRenderer !!!
	// --> "_IsEmpty" is treated as a normal SlotName

	// set up the configuration
	SwitchRendererTest::ConfigureCases( "" );
	fConfig["Case"]["_IsEmpty"] = "That is case Empty";
	// this Slot was used by the old renderer when an empty string was used
	// as a lookup key
	fConfig["Default"] = "This is the default";

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
//	assertCharPtrEqual( fConfig["Case"]["_IsEmpty"].AsCharPtr(), fReply.str());
	// this used to be the correct result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
	// this is the new result
}

/*===============================================================*/
/*     Check when cases are not defined                          */
/*===============================================================*/
void SwitchRendererTest::TestCaseAWithoutCases()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseA" );
	// Remove the "Case" actions
	fConfig.Remove("Case" );
	// Check if the "Case" action is really removed
	t_assert( fConfig.IsDefined("Case") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void SwitchRendererTest::TestCaseBWithoutCases()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseB" );
	// Remove the "Case" actions
	fConfig.Remove("Case" );
	// Check if the "Case" action is really removed
	t_assert( fConfig.IsDefined("Case") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void SwitchRendererTest::TestCaseCWithoutCases()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseC" );
	// Remove the "Case" actions
	fConfig.Remove("Case" );
	// Check if the "Case" action is really removed
	t_assert( fConfig.IsDefined("Case") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

/*===============================================================*/
/*     Check when ContextLookupName is not defined               */
/*===============================================================*/
void SwitchRendererTest::TestCaseAWithoutContextLookupName()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseA" );
	// Remove the "Case" actions
	fConfig.Remove("ContextLookupName" );
	// Check if the "Case" action is really removed
	t_assert( fConfig.IsDefined("ContextLookupName") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void SwitchRendererTest::TestCaseBWithoutContextLookupName()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseB" );
	// Remove the "Case" actions
	fConfig.Remove("ContextLookupName" );
	// Check if the "Case" action is really removed
	t_assert( fConfig.IsDefined("ContextLookupName") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void SwitchRendererTest::TestCaseCWithoutContextLookupName()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseC" );
	// Remove the "Case" actions
	fConfig.Remove("ContextLookupName" );
	// Check if the "Case" action is really removed
	t_assert( fConfig.IsDefined("ContextLookupName") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void SwitchRendererTest::TestCaseNotFoundWithoutContextLookupName()
{
	// set up the configuration
	SwitchRendererTest::ConfigureCases( "CaseX" );
	// Remove the "Case" actions
	fConfig.Remove("ContextLookupName" );
	// Check if the "Case" action is really removed
	t_assert( fConfig.IsDefined("ContextLookupName") == false );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

/*===============================================================*/
/*     Check case where lookup fails                             */
/*===============================================================*/
void SwitchRendererTest::TestCaseLookupEmpty()
{
	// set up the configuration

	fConfig["ContextLookupName"] = "fields.NotExisting.Test";	// name
	t_assert( fConfig.IsDefined("ContextLookupName") == true );
	SwitchRendererTest::ConfigureCases( "CaseY" );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

