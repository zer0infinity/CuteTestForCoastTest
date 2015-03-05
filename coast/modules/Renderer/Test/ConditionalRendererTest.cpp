/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "Anything.h"
#include "StringStream.h"
#include "Context.h"
#include "Renderer.h"
#include "TestSuite.h"
#include "ConditionalRenderers.h"
#include "ConditionalRendererTest.h"

ConditionalRendererTest::ConditionalRendererTest (TString tname) : RendererTest(tname) {};
ConditionalRendererTest::~ConditionalRendererTest() {};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ConditionalRendererTest::setUp ()
{
	RendererTest::setUp();
}

Test *ConditionalRendererTest::suite ()
{
	TestSuite *testSuite = new TestSuite;

	ADD_CASE(testSuite, ConditionalRendererTest, CondTrue);
	ADD_CASE(testSuite, ConditionalRendererTest, CondFalse);
	ADD_CASE(testSuite, ConditionalRendererTest, CondDefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondUndefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondError);

	ADD_CASE(testSuite, ConditionalRendererTest, CondOnlyTrue);
	ADD_CASE(testSuite, ConditionalRendererTest, CondOnlyFalse);
	ADD_CASE(testSuite, ConditionalRendererTest, CondOnlyDefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondOnlyUndefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondOnlyError);

	ADD_CASE(testSuite, ConditionalRendererTest, CondMissingTrue);
	ADD_CASE(testSuite, ConditionalRendererTest, CondMissingFalse);
	ADD_CASE(testSuite, ConditionalRendererTest, CondMissingDefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondMissingUndefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondMissingError);

	ADD_CASE(testSuite, ConditionalRendererTest, CondTrueWithoutTrueDefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondFalseWithoutFalseDefined);

	ADD_CASE(testSuite, ConditionalRendererTest, CondTrueConfUndefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondFalseConfUndefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondDefinedConfUndefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondUndefinedConfUndefined);
	ADD_CASE(testSuite, ConditionalRendererTest, CondErrorConfUndefined);

	ADD_CASE(testSuite, ConditionalRendererTest, TestTrueFalseCase);
	ADD_CASE(testSuite, ConditionalRendererTest, CondTrueSpecChars);
	ADD_CASE(testSuite, ConditionalRendererTest, SpecCharsWithDefDelims);
	ADD_CASE(testSuite, ConditionalRendererTest, CondTrueSpecCharsWithRendering);

	return testSuite;

}

/*===============================================================*/
/*     Check the single cases where all is correctly defined     */
/*===============================================================*/
void ConditionalRendererTest::ConfigureTrue(const String &key)
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
	fConfig["ContextCondition"] = key;
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore[key] = 1L;
	TraceAny(fConfig, "fConfig");
	TraceAny(tmpStore, "tmpStore");
}

void ConditionalRendererTest::ConfigureFalse(const String &key)
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
	fConfig["ContextCondition"] = key;
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore[key] = 0L;
	TraceAny(fConfig, "fConfig");
	TraceAny(tmpStore, "tmpStore");
}

void ConditionalRendererTest::ConfigureDefined(const String &key)
{
	StartTrace(ConditionalRendererTest.ConfigureDefined);
	fConfig["ContextCondition"] = key;
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore[key] = "string";
	TraceAny(fConfig, "fConfig");
	TraceAny(tmpStore, "tmpStore");
}

void ConditionalRendererTest::ConfigureUndefined(const String &key)
{
	StartTrace(ConditionalRendererTest.ConfigureUndefined);
	fConfig["ContextCondition"] = key;
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";
	TraceAny(fConfig, "fConfig");

//	tmpStore["ContextConditionKey"] = undef
//
//	Anything tmpStore(fContext.GetTmpStore());
//	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureError()
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
//	fConfig["ContextCondition"] = undef
//
//	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";
	fConfig["Error"] = "That is Error";	// normally this slot is missing
}

void ConditionalRendererTest::CondTrue()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureTrue("ContextConditionKey");
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["True"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondTrueSpecChars()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondTrueSpecChars);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureTrue("/gugus.dada:txt");
	fConfig["PathDelim"]  = "Ignore";
	fConfig["IndexDelim"] = "Ignore";

	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["True"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondTrueSpecCharsWithRendering()
{
	StartTrace(SwitchRendererTest.CondTrueSpecCharsWithRendering);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureTrue("/gugus.dada:txt");
	fConfig["PathDelim"]["ContextLookupRenderer"]	= "ThePathDelim";
	fConfig["IndexDelim"]["ContextLookupRenderer"]	= "TheIndexDelim";
	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ThePathDelim"]	= "!";
	tmpStore["TheIndexDelim"]	= "?";
	TraceAny(tmpStore, "tmpStore")

	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["True"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::SpecCharsWithDefDelims()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondTrueSpecChars);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureTrue("/gugus.dada:txt");
	fConfig["PathDelim"]  = ".";
	fConfig["IndexDelim"] = ":";

	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Undefined"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondFalse()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureFalse("ContextConditionKey");
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["False"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondDefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureDefined("ContextConditionKey");
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Defined"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureUndefined("ContextConditionKey");
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Undefined"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondError()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.ConfigureTrue);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureError();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Error"].AsCharPtr(), fReply.str());
}

/*===============================================================*/
/*     Check the single cases where only the the                 */
/*     right fConfig is defined                                  */
/*===============================================================*/
void ConditionalRendererTest::ConfigureOnlyTrue()
{
	StartTrace(ConditionalRendererTest.ConfigureOnlyTrue);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 1L;
}

void ConditionalRendererTest::ConfigureOnlyFalse()
{
	StartTrace(ConditionalRendererTest.ConfigureOnlyFalse);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["False"] = "That is False";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 0L;
}

void ConditionalRendererTest::ConfigureOnlyDefined()
{
	StartTrace(ConditionalRendererTest.ConfigureOnlyDefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["Defined"] = "That is Defined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureOnlyUndefined()
{
	StartTrace(ConditionalRendererTest.ConfigureOnlyUndefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["Undefined"] = "That is Undefined";

//	tmpStore["ContextConditionKey"] = undef
//
//	Anything tmpStore(fContext.GetTmpStore());
//	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureOnlyError()
{
	StartTrace(ConditionalRendererTest.ConfigureOnlyError);
//	fConfig["ContextCondition"] = undef
//
//	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["Error"] = "That is Error";	// normally this slot is missing
}

void ConditionalRendererTest::CondOnlyTrue()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondOnlyTrue);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureOnlyTrue();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["True"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondOnlyFalse()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondOnlyFalse);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureOnlyFalse();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["False"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondOnlyDefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondOnlyDefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureOnlyDefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Defined"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondOnlyUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondOnlyUndefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureOnlyUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Undefined"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondOnlyError()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondOnlyError);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureOnlyError();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Error"].AsCharPtr(), fReply.str());
}

/*===============================================================*/
/*     Check the single cases where the right fConfig            */
/*     is not defined                                            */
/*===============================================================*/
void ConditionalRendererTest::ConfigureMissingTrue()
{
	StartTrace(ConditionalRendererTest.ConfigureMissingTrue);
	fConfig["ContextCondition"] = "ContextConditionKey";
//	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 1L;
}

void ConditionalRendererTest::ConfigureMissingFalse()
{
	StartTrace(ConditionalRendererTest.ConfigureMissingFalse);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
//	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 0L;
}

void ConditionalRendererTest::ConfigureMissingDefined()
{
	StartTrace(ConditionalRendererTest.ConfigureMissingDefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureMissingUndefined()
{
	StartTrace(ConditionalRendererTest.ConfigureMissingUndefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
//	fConfig["Undefined"] = "That is Undefined";

//	tmpStore["ContextConditionKey"] = undef
//
//	Anything tmpStore(fContext.GetTmpStore());
//	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureMissingError()
{
	StartTrace(ConditionalRendererTest.ConfigureMissingError);
//	fConfig["ContextCondition"] = undef
//
//	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";
//	fConfig["Error"] = "That is Error";
}

void ConditionalRendererTest::CondMissingTrue()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondMissingTrue);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureMissingTrue();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Defined"].AsCharPtr(), fReply.str());
	t_assert( fConfig.IsDefined("True") == false );
}

void ConditionalRendererTest::CondMissingFalse()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondMissingFalse);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureMissingFalse();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Defined"].AsCharPtr(), fReply.str());
	t_assert( fConfig.IsDefined("False") == false );
}

void ConditionalRendererTest::CondMissingDefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondMissingDefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureMissingDefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
	t_assert( fConfig.IsDefined("Defined") == false );
}

void ConditionalRendererTest::CondMissingUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondMissingUndefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureMissingUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
	t_assert( fConfig.IsDefined("Undefined") == false );
}

void ConditionalRendererTest::CondMissingError()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondMissingError);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureMissingError();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
	t_assert( fConfig.IsDefined("Error") == false );

}

/*===============================================================*/
/*     Check the "True/False" cases where                        */
/*    fConfig.True/False/Defined are not defined                 */
/*===============================================================*/
void ConditionalRendererTest::ConfigureTrueWithoutTrueDefined()
{
	StartTrace(ConditionalRendererTest.ConfigureTrueWithoutTrueDefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
//	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 1L;
}

void ConditionalRendererTest::ConfigureFalseWithoutFalseDefined()
{
	StartTrace(ConditionalRendererTest.ConfigureFalseWithoutFalseDefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
//	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 0L;
}

void ConditionalRendererTest::CondTrueWithoutTrueDefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondTrueWithoutTrueDefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureTrueWithoutTrueDefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
	t_assert( fConfig.IsDefined("True") == false );
	t_assert( fConfig.IsDefined("Defined") == false );
}

void ConditionalRendererTest::CondFalseWithoutFalseDefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondFalseWithoutFalseDefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureFalseWithoutFalseDefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
	t_assert( fConfig.IsDefined("False") == false );
	t_assert( fConfig.IsDefined("Defined") == false );
}

/*===============================================================*/
/*     Check the single cases where fConfig is undefined         */
/*===============================================================*/
void ConditionalRendererTest::ConfigureTrueConfUndefined()
{
	StartTrace(ConditionalRendererTest.ConfigureTrueConfUndefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
//	fConfig["True"] = "That is True";
//	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
//	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 1L;
}

void ConditionalRendererTest::ConfigureFalseConfUndefined()
{
	StartTrace(ConditionalRendererTest.ConfigureFalseConfUndefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
//	fConfig["True"] = "That is True";
//	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
//	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 0L;
}

void ConditionalRendererTest::ConfigureDefinedConfUndefined()
{
	StartTrace(ConditionalRendererTest.ConfigureDefinedConfUndefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
//	fConfig["True"] = "That is True";
//	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
//	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureUndefinedConfUndefined()
{
	StartTrace(ConditionalRendererTest.ConfigureUndefinedConfUndefined);
	fConfig["ContextCondition"] = "ContextConditionKey";
//	fConfig["True"] = "That is True";
//	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
//	fConfig["Undefined"] = "That is Undefined";

//	tmpStore["ContextConditionKey"] = undef
//
//	Anything tmpStore(fContext.GetTmpStore());
//	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureErrorConfUndefined()
{
	StartTrace(ConditionalRendererTest.ConfigureErrorConfUndefined);
//	fConfig["ContextCondition"] = undef
//
//	fConfig["ContextCondition"] = "ContextConditionKey";
//	fConfig["True"] = "That is True";
//	fConfig["False"] = "That is False";
//	fConfig["Defined"] = "That is Defined";
//	fConfig["Undefined"] = "That is Undefined";
//	fConfig["Error"] = "That is Error";
}

void ConditionalRendererTest::CondTrueConfUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondTrueConfUndefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureTrueConfUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void ConditionalRendererTest::CondFalseConfUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondFalseConfUndefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureFalseConfUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void ConditionalRendererTest::CondDefinedConfUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondDefinedConfUndefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureDefinedConfUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void ConditionalRendererTest::CondUndefinedConfUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondUndefinedConfUndefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureUndefinedConfUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

void ConditionalRendererTest::CondErrorConfUndefined()
// test the Conditional renderer with simple formatting strings
{
	StartTrace(ConditionalRendererTest.CondErrorConfUndefined);
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureErrorConfUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( "", fReply.str());
}

/*===============================================================*/
/*     Check if "True/False" cases are correctly selected        */
/*===============================================================*/
void ConditionalRendererTest::SetTrueFalseCase(long condition, const String &key)
{
	StartTrace(ConditionalRendererTest.SetTrueFalseCase);
	fConfig["ContextCondition"] = key;
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore[key] = condition;
}

void ConditionalRendererTest::TestTrueFalseCase()
{
	StartTrace(ConditionalRendererTest.TestTrueFalseCase);
	long condition;
	for ( condition = -5; condition < 6; condition++ ) {
		OStringStream reply;
		ConditionalRenderer conditionalRenderer("");
		// set up the configuration
		ConditionalRendererTest::SetTrueFalseCase( condition, "ContextConditionKey");
		// render the configuration
		ROAnything roConfig = fConfig;
		conditionalRenderer.RenderAll(reply, fContext, roConfig);
		if ( condition != 0 ) {
			// true Case
			assertCharPtrEqual( fConfig["True"].AsCharPtr(), reply.str());
		} else {
			// false case
			assertCharPtrEqual( fConfig["False"].AsCharPtr(), reply.str());
		}
	}
}

