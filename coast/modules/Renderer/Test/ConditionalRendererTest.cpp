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
#include "ConditionalRendererTest.h"

ConditionalRendererTest::ConditionalRendererTest (TString tname) : RendererTest(tname) {};
ConditionalRendererTest::~ConditionalRendererTest() {};

/*===============================================================*/
/*     Init                                                      */
/*===============================================================*/
void ConditionalRendererTest::setUp ()
// setup config for all the renderers in this TestCase
{
	RendererTest::setUp();
} // setUp

Test *ConditionalRendererTest::suite ()
// collect all test cases for the ConditionalRenderer
{
	TestSuite *testSuite = new TestSuite;

	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondTrue));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondFalse));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondDefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondUndefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondError));

	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondOnlyTrue));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondOnlyFalse));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondOnlyDefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondOnlyUndefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondOnlyError));

	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondMissingTrue));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondMissingFalse));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondMissingDefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondMissingUndefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondMissingError));

	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondTrueWithoutTrueDefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondFalseWithoutFalseDefined));

	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondTrueConfUndefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondFalseConfUndefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondDefinedConfUndefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondUndefinedConfUndefined));
	testSuite->addTest (NEW_CASE(ConditionalRendererTest, CondErrorConfUndefined));

	testSuite->addTest (NEW_CASE(ConditionalRendererTest, TestTrueFalseCase));

	return testSuite;

} // suite

/*===============================================================*/
/*     Check the single cases where all is correctly defined     */
/*===============================================================*/
void ConditionalRendererTest::ConfigureTrue()
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 1L;
}

void ConditionalRendererTest::ConfigureFalse()
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 0L;
}

void ConditionalRendererTest::ConfigureDefined()
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureUndefined()
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

//	tmpStore["ContextConditionKey"] = undef
//
//	Anything tmpStore(fContext.GetTmpStore());
//	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureError()
{
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
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureTrue();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["True"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondFalse()
// test the Conditional renderer with simple formatting strings
{
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureFalse();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["False"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondDefined()
// test the Conditional renderer with simple formatting strings
{
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureDefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Defined"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondUndefined()
// test the Conditional renderer with simple formatting strings
{
	ConditionalRenderer conditionalRenderer("");

	// set up the configuration
	ConditionalRendererTest::ConfigureUndefined();
	// render the configuration
	ROAnything roConfig = fConfig;
	conditionalRenderer.RenderAll(fReply, fContext, roConfig);

	// assert the result
	assertCharPtrEqual( fConfig["Undefined"].AsCharPtr(), fReply.str());
}

void ConditionalRendererTest::CondError()
// test the Conditional renderer with simple formatting strings
{
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
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 1L;
}

void ConditionalRendererTest::ConfigureOnlyFalse()
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["False"] = "That is False";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = 0L;
}

void ConditionalRendererTest::ConfigureOnlyDefined()
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["Defined"] = "That is Defined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureOnlyUndefined()
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["Undefined"] = "That is Undefined";

//	tmpStore["ContextConditionKey"] = undef
//
//	Anything tmpStore(fContext.GetTmpStore());
//	tmpStore["ContextConditionKey"] = "string";
}

void ConditionalRendererTest::ConfigureOnlyError()
{
//	fConfig["ContextCondition"] = undef
//
//	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["Error"] = "That is Error";	// normally this slot is missing
}

void ConditionalRendererTest::CondOnlyTrue()
// test the Conditional renderer with simple formatting strings
{
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
void ConditionalRendererTest::SetTrueFalseCase(long condition)
{
	fConfig["ContextCondition"] = "ContextConditionKey";
	fConfig["True"] = "That is True";
	fConfig["False"] = "That is False";
	fConfig["Defined"] = "That is Defined";
	fConfig["Undefined"] = "That is Undefined";

	Anything tmpStore(fContext.GetTmpStore());
	tmpStore["ContextConditionKey"] = condition;
}

void ConditionalRendererTest::TestTrueFalseCase()
{
	long condition;
	for ( condition = -5; condition < 6; condition++ ) {
		OStringStream reply;
		ConditionalRenderer conditionalRenderer("");
		// set up the configuration
		ConditionalRendererTest::SetTrueFalseCase( condition );
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

