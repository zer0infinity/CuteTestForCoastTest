/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StatementDescriptionTest.h"
#include "OracleStatement.h"
#include "TestSuite.h"
#include "AnyIterators.h"
#include "Tracer.h"

StatementDescriptionTest::StatementDescriptionTest(TString tname)
	: TestCaseType(tname)
{
}

StatementDescriptionTest::~StatementDescriptionTest()
{
}

void StatementDescriptionTest::DescriptionElementConstructionTest()
{
	StartTrace(StatementDescriptionTest.DescriptionElementConstructionTest);
	OracleStatement::Description::Element aElt;
	t_assert( aElt.fRBuf.IsNull() );
	t_assert( aElt.fWBuf.IsNull() );

	ROAnything roaEmpty;
	Anything anyVals;
	anyVals["Name"] = "gugus";
	anyVals["Idx"] = 12L;
	OracleStatement::Description::Element aFilledElt(roaEmpty, anyVals);
	t_assert( !aFilledElt.fWBuf.IsNull() );
	assertAnyEqual(anyVals, aFilledElt.fWBuf);
	assertEqual("gugus", aFilledElt.AsString("Name", "gaga") );
	assertEqual(12L, aFilledElt.AsLong("Idx", -1L) );

	aElt = aFilledElt;
	assertAnyEqual(anyVals, aElt.fWBuf);
	aElt["Name"] = "gaga";
	assertEqual("gaga", aElt.AsString("Name", "gugus") );
	aElt["NewSlot"] = "Juppieh";
	assertEqual("Juppieh", aElt.AsString("NewSlot", "gugus") );
}

void StatementDescriptionTest::DescriptionElementShadowTest()
{
	StartTrace(StatementDescriptionTest.DescriptionElementShadowTest);
	Anything anyDefaultVals;
	anyDefaultVals["Name"] = "NameDefault";
	anyDefaultVals["Idx"] = 11L;
	Anything anyVals = Anything(Anything::ArrayMarker());
	ROAnything roaDefaults(anyDefaultVals);
	OracleStatement::Description::Element aFilledElt(roaDefaults, anyVals);
	t_assert( !aFilledElt.fWBuf.IsNull() );

	assertEqual("NameDefault", aFilledElt.AsString("Name", "gaga") );
	assertEqual(11L, aFilledElt.AsLong("Idx", -1L) );

	aFilledElt["Name"] = "Overridden";
	assertEqual("Overridden", aFilledElt.AsString("Name", "gugus") );
	aFilledElt["NewSlot"] = "Juppieh";
	assertEqual("Juppieh", aFilledElt.AsString("NewSlot", "gugus") );
	aFilledElt["Idx"] = 2L;
	assertEqual(2L, aFilledElt.AsLong("Idx", -1L) );
}

void StatementDescriptionTest::DescriptionSimpleTest()
{
	StartTrace(StatementDescriptionTest.DescriptionSimpleTest);
	OracleStatement::Description aDesc;
	Anything anyDefaultVals;
	anyDefaultVals[0L]["Name"] = "NameDefault0";
	anyDefaultVals[0L]["Idx"] = 0L;
	anyDefaultVals[1L]["Name"] = "NameDefault1";
	anyDefaultVals[1L]["Idx"] = 1L;
	anyDefaultVals[2L]["Name"] = "NameDefault1";
	anyDefaultVals[2L]["Idx"] = 2L;
	ROAnything roaDefaults(anyDefaultVals);
	assertEqual(0L, aDesc.GetSize());
	aDesc = roaDefaults;
	assertEqual(roaDefaults.GetSize(), aDesc.GetSize());
	{
		OracleStatement::Description::Element aElt0;
		aElt0 = aDesc[0L];
		assertEqual("NameDefault0", aElt0.AsString("Name", "gaga"));
		aElt0["Name"] = "ReturnName";
		assertEqual("ReturnName", aElt0.AsString("Name", "gaga"));
	}
	ROAnything roaValue;
	roaValue = ROAnything(aDesc.fWBuf)[0L];
	assertEqual("ReturnName", roaValue["Name"].AsString("gaga"));
	assertEqual("NameDefault0", aDesc.fRBuf[0L]["Name"].AsString("gaga"));
	TraceAny(aDesc.fRBuf, "RBuf");
	TraceAny(aDesc.fWBuf, "WBuf");
}

void StatementDescriptionTest::DescriptionIteratorTest()
{
	StartTrace(StatementDescriptionTest.DescriptionIteratorTest);
	AnyExtensions::Iterator<ROAnything> aIterator(GetTestCaseConfig());
	ROAnything roaConfig;
	while ( aIterator(roaConfig) ) {
		ROAnything roaR ( roaConfig["RO"] );
		OracleStatement::Description aDesc;
		aDesc = roaR;
		AnyExtensions::Iterator<ROAnything> aWRIterator(roaConfig["WR"]);
		ROAnything roaWR;
		while ( aWRIterator(roaWR) ) {
			Anything anyDesc = roaWR.DeepClone();
			aDesc.Append(anyDesc);
		}
		AnyExtensions::Iterator<OracleStatement::Description, OracleStatement::Description::Element> aDescIterator(aDesc);
		OracleStatement::Description::Element aElt;
		while ( aDescIterator.Next(aElt) ) {
			ROAnything roaExpected = roaConfig["Expected"][aDescIterator.Index()];
			AnyExtensions::Iterator<ROAnything> aSlotIterator(roaExpected);
			ROAnything roaSlot;
			String strName;
			while ( aSlotIterator.Next(roaSlot) ) {
				aSlotIterator.SlotName(strName);
				assertEqual(roaSlot.AsString("gugus"), aElt.AsString(strName, "gaga"));
			}
		}
	}
}

Test *StatementDescriptionTest::suite ()
{
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, StatementDescriptionTest, DescriptionElementConstructionTest);
	ADD_CASE(testSuite, StatementDescriptionTest, DescriptionElementShadowTest);
	ADD_CASE(testSuite, StatementDescriptionTest, DescriptionSimpleTest);
	ADD_CASE(testSuite, StatementDescriptionTest, DescriptionIteratorTest);
	return testSuite;
}
