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
#include "LocalizedRenderers.h"

//--- interface include --------------------------------------------------------
#include "StringRendererTest.h"

StringRendererTest::StringRendererTest (TString tname) : RendererTest(tname),
	fStringRenderer("StringRendererTest") {};

StringRendererTest::~StringRendererTest() {};

/*=================================================================================*/
/*      Suite init                                                                 */
/*=================================================================================*/
Test *StringRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, StringRendererTest, langGE);
	ADD_CASE(testSuite, StringRendererTest, langGEWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langGEDefaultWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langGEalone);

	ADD_CASE(testSuite, StringRendererTest, langFR);
	ADD_CASE(testSuite, StringRendererTest, langFRWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langFRDefaultWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langFRalone);

	ADD_CASE(testSuite, StringRendererTest, langEN);
	ADD_CASE(testSuite, StringRendererTest, langENWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langENDefaultWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langENalone);

	ADD_CASE(testSuite, StringRendererTest, langIT);
	ADD_CASE(testSuite, StringRendererTest, langITWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langITDefaultWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langITalone);

	ADD_CASE(testSuite, StringRendererTest, langDefault);
	ADD_CASE(testSuite, StringRendererTest, langDefaultWithoutText);
	ADD_CASE(testSuite, StringRendererTest, langDefaultalone);

	ADD_CASE(testSuite, StringRendererTest, noText);

	return testSuite;

}

void StringRendererTest::setUp ()
{
	RendererTest::setUp();
	fConfig["D"] = "Deutsch";
	fConfig["F"] = "Francais";
	fConfig["E"] = "English";
	fConfig["I"] = "Italiano";
	fConfig["Default"] = "Default language";
}

/*=================================================================================*/
/*      Tests for language == German                                               */
/*=================================================================================*/
void StringRendererTest::langGE()
{
	// set up the configuration
	fContext.SetLanguage("D");

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["D"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langGEWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("D");
	fConfig.Remove("D");
	t_assert( fConfig.IsDefined("D") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langGEDefaultWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("D");
	fConfig.Remove("D");
	t_assert( fConfig.IsDefined("D") == 0 );
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void StringRendererTest::langGEalone()
{
	// set up the configuration
	fContext.SetLanguage("D");
	fConfig.Remove("F");
	fConfig.Remove("I");
	fConfig.Remove("E");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("F") == 0 );
	t_assert( fConfig.IsDefined("I") == 0 );
	t_assert( fConfig.IsDefined("E") == 0 );
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["D"].AsCharPtr(), fReply.str());
}

/*=================================================================================*/
/*      Tests for language == French                                               */
/*=================================================================================*/
void StringRendererTest::langFR()
{
	// set up the configuration
	fContext.SetLanguage("F");

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["F"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langFRWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("F");
	fConfig.Remove("F");
	t_assert( fConfig.IsDefined("F") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langFRDefaultWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("F");
	fConfig.Remove("F");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("F") == 0 );
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertEqual(0, fReply.str().Length() );
	assertCharPtrEqual( "", (const char *)fReply.str());
}

void StringRendererTest::langFRalone()
{
	// set up the configuration
	fContext.SetLanguage("F");
	fConfig.Remove("D");
	fConfig.Remove("I");
	fConfig.Remove("E");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("D") == 0 );
	t_assert( fConfig.IsDefined("I") == 0 );
	t_assert( fConfig.IsDefined("E") == 0 );
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["F"].AsCharPtr(), fReply.str());
}

/*=================================================================================*/
/*      Tests for language == English                                              */
/*=================================================================================*/
void StringRendererTest::langEN()
{
	// set up the configuration
	fContext.SetLanguage("E");

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["E"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langENWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("E");
	fConfig.Remove("E");
	t_assert( fConfig.IsDefined("E") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langENDefaultWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("E");
	fConfig.Remove("E");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("E") == 0 );
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void StringRendererTest::langENalone()
{
	// set up the configuration
	fContext.SetLanguage("E");
	fConfig.Remove("D");
	fConfig.Remove("I");
	fConfig.Remove("F");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("D") == 0 );
	t_assert( fConfig.IsDefined("I") == 0 );
	t_assert( fConfig.IsDefined("F") == 0 );
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["E"].AsCharPtr(), fReply.str());
}

/*=================================================================================*/
/*      Tests for language == Italian                                              */
/*=================================================================================*/
void StringRendererTest::langIT()
{
	// set up the configuration
	fContext.SetLanguage("I");

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["I"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langITWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("I");
	fConfig.Remove("I");
	t_assert( fConfig.IsDefined("I") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langITDefaultWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("I");
	fConfig.Remove("I");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("Default") == 0 );
	t_assert( fConfig.IsDefined("I") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void StringRendererTest::langITalone()
{
	// set up the configuration
	fContext.SetLanguage("I");
	fConfig.Remove("D");
	fConfig.Remove("F");
	fConfig.Remove("E");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("D") == 0 );
	t_assert( fConfig.IsDefined("F") == 0 );
	t_assert( fConfig.IsDefined("E") == 0 );
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["I"].AsCharPtr(), fReply.str());
}

/*=================================================================================*/
/*      Tests for language == Default                                              */
/*=================================================================================*/
void StringRendererTest::langDefault()
{
	// set up the configuration
	fContext.SetLanguage("*");

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

void StringRendererTest::langDefaultWithoutText()
{
	// set up the configuration
	fContext.SetLanguage("Default");
	fConfig.Remove("Default");
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void StringRendererTest::langDefaultalone()
{
	// set up the configuration
	fContext.SetLanguage("Default");
	fConfig.Remove("D");
	fConfig.Remove("I");
	fConfig.Remove("E");
	fConfig.Remove("F");
	t_assert( fConfig.IsDefined("F") == 0 );
	t_assert( fConfig.IsDefined("I") == 0 );
	t_assert( fConfig.IsDefined("E") == 0 );
	t_assert( fConfig.IsDefined("D") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Default"].AsCharPtr(), fReply.str());
}

/*=================================================================================*/
/*     Miscellaneous                                                               */
/*=================================================================================*/
void StringRendererTest::noText()
{
	// set up the configuration
	fContext.SetLanguage("Default");
	fConfig.Remove("Default");
	fConfig.Remove("D");
	fConfig.Remove("I");
	fConfig.Remove("E");
	fConfig.Remove("F");
	t_assert( fConfig.IsDefined("F") == 0 );
	t_assert( fConfig.IsDefined("I") == 0 );
	t_assert( fConfig.IsDefined("E") == 0 );
	t_assert( fConfig.IsDefined("D") == 0 );
	t_assert( fConfig.IsDefined("Default") == 0 );

	// render the configuration
	ROAnything roConfig = fConfig;
	fStringRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

