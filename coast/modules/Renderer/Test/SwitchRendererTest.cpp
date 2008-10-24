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
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseCtxLookupNameSpecChars);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseCtxLookupNameSpecCharsWithDefDelims);
	ADD_CASE(testSuite, SwitchRendererTest, TestCaseCtxLookupNameSpecCharsWithRendering);

	return testSuite;

}

/*===============================================================*/
/*     Check found cases where all is correctly defined          */
/*===============================================================*/
void SwitchRendererTest::ConfigureCases( const String &caseKey, const char *cases )
{
	StartTrace(SwitchRendererTest.ConfigureCases);
	fConfig = Anything();
	fConfig["ContextLookupName"] = caseKey;	// name

	fConfig["Case"]["CaseA"] = "That is case A";
	fConfig["Case"]["CaseB"] = "That is case B";
	fConfig["Case"]["CaseC"] = "That is case C";
	fConfig["Default"] = "That is Default";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore[caseKey] = cases; // = CaseA,...  = s
	TraceAny(fConfig, "fConfig");
	TraceAny(tmpStore, "tmpStore")
}

void SwitchRendererTest::TestCaseA()
{
	StartTrace(SwitchRendererTest.TestCaseA);
	SwitchRenderer switchRenderer("");

	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseA" );
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseA"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseB()
{
	StartTrace(SwitchRendererTest.TestCaseB);
	SwitchRenderer switchRenderer("");

	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseB" );
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseB"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseC()
{
	StartTrace(SwitchRendererTest.TestCaseC);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseC" );
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseC"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseNotFoundWithDefault()
{
	StartTrace(SwitchRendererTest.TestCaseNotFoundWithDefault);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseX" );
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
	StartTrace(SwitchRendererTest.TestCaseNotFoundWithoutDefault);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseX" );
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
	StartTrace(SwitchRendererTest.TestCaseEmptyKeyWithDefault);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "" );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseEmptyKeyWithIsEmpty()
{
	StartTrace(SwitchRendererTest.TestCaseEmptyKeyWithIsEmpty);
	// _IsEmpty is programmed for SwitchRenderer and NOT for SwitchRenderer !!!
	// --> "_IsEmpty" is treated as a normal SlotName

	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "" );
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
	StartTrace(SwitchRendererTest.TestCaseAWithoutCases);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseA" );
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
	StartTrace(SwitchRendererTest.TestCaseBWithoutCases);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseB" );
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
	StartTrace(SwitchRendererTest.TestCaseCWithoutCases);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseC" );
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
	StartTrace(SwitchRendererTest.TestCaseAWithoutContextLookupName);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseA" );
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
	StartTrace(SwitchRendererTest.TestCaseBWithoutContextLookupName);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseB" );
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
	StartTrace(SwitchRendererTest.TestCaseCWithoutContextLookupName);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseC" );
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
	StartTrace(SwitchRendererTest.TestCaseNotFoundWithoutContextLookupName);
	// set up the configuration
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseX" );
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
	StartTrace(SwitchRendererTest.TestCaseLookupEmpty);
	// set up the configuration

	fConfig["ContextLookupName"] = "fields.NotExisting.Test";	// name
	t_assert( fConfig.IsDefined("ContextLookupName") == true );
	SwitchRendererTest::ConfigureCases("CaseKey", "CaseY" );

	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseCtxLookupNameSpecChars()
{
	StartTrace(SwitchRendererTest.TestCaseCtxLookupNameSpecChars);
	SwitchRenderer switchRenderer("");

	// set up the configuration
	// This path would normally indicate a path (/slot and index[] !
	SwitchRendererTest::ConfigureCases("/gugus.dada:txt", "CaseA" );
	fConfig["PathDelim"]  = "Ignore";
	fConfig["IndexDelim"] = "Ignore";
	// render the configuration
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);
	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseA"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseCtxLookupNameSpecCharsWithRendering()
{
	StartTrace(SwitchRendererTest.TestCaseCtxLookupNameSpecChars);
	SwitchRenderer switchRenderer("");

	// set up the configuration
	// This path would normally indicate a path (/slot and index[] !
	SwitchRendererTest::ConfigureCases("/gugus.dada:txt", "CaseA" );
	fConfig["PathDelim"]["ContextLookupRenderer"]	= "ThePathDelim";
	fConfig["IndexDelim"]["ContextLookupRenderer"]	= "TheIndexDelim";
	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ThePathDelim"]	= "!";
	tmpStore["TheIndexDelim"]	= "?";
	TraceAny(tmpStore, "tmpStore")
	// render the configuration
	ROAnything roConfig = fConfig;

	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);
	// assert the result
	assertCharPtrEqual( fConfig["Case"]["CaseA"].AsCharPtr(), fReply.str());
}

void SwitchRendererTest::TestCaseCtxLookupNameSpecCharsWithDefDelims()
{
	StartTrace(SwitchRendererTest.TestCaseCtxLookupNameSpecCharsWithDefDelims);
	SwitchRenderer switchRenderer("");

	// This test uses a ContextLookup name containing the standard path (.)
	// and index (:) delimiters.
	// It returns the default value
	SwitchRendererTest::ConfigureCases("/gugus.dada:txt", "CaseA" );
	fConfig["PathDelim"]  = ".";
	fConfig["IndexDelim"] = ":";
	ROAnything roConfig = fConfig;
	fSwitchRenderer.RenderAll(fReply, fContext, roConfig);
	// Since the context condition is not found, not even the default value
	// is rendered.
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}
