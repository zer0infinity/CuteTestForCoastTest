/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingUtilsTest.h"
#include "AnythingUtils.h"
#include "TestSuite.h"

void AnythingUtilsTest::setUp() {
	StartTrace(AnythingUtilsTest.setUp);
	fQuery = GetConfig()["Queries"][name()].DeepClone();
}

void AnythingUtilsTest::StoreCopierTest() {
	StartTrace(AnythingUtilsTest.StoreCopierTest);
	// Set up
	for (long l = 0; l < fQuery.GetSize(); l++) {
		ROAnything config = fQuery[l];
		ROAnything copyList = config["CopyList"];
		Anything source = config["Source"].DeepClone();
		Anything result;
		Context c;
		coast::testframework::PutInStore(source, c.GetTmpStore());

		StoreCopier::Operate(c, result, copyList, config["Delim"].AsCharPtr(".")[0L], config["IndexDelim"].AsCharPtr(":")[0L]);

		ROAnything expectedStore(config["Result"]);
		assertAnyEqualm(expectedStore, result, TString(name()) << "." << fQuery.SlotName(l));
	}
}

void AnythingUtilsTest::StorePutterTest() {
	StartTrace(AnythingUtilsTest.StorePutterTest);
	// Set up
	Context c;
	Anything toStore = fQuery["ToStore"];
	Anything destinations = fQuery["Destinations"];
	long sz = destinations.GetSize();
	for (long i = 0; i < sz; i++) {
		StorePutter::Operate(toStore, c, destinations[i]);
	}
	TraceAny(c.GetTmpStore(), "TempStore after");
	TraceAny(c.GetRoleStoreGlobal(), "RoleStore after");
	TraceAny(c.GetSessionStore(), "SessionStore after");

	ROAnything expectedStore(GetConfig()["Results"][name()]["Store"]);
	ROAnything expectedSessionStore(GetConfig()["Results"][name()]["SessionStore"]);
	assertAnyEqual(expectedStore, c.GetTmpStore());
	assertAnyEqual(expectedStore, c.GetRoleStoreGlobal());
	assertAnyEqual(expectedSessionStore, c.GetSessionStore());
}

void AnythingUtilsTest::StorePutterReplaceTest() {
	StartTrace(AnythingUtilsTest.StorePutterReplaceTest);
	// Set up
	Context c;
	Anything toStore = fQuery["ToStore"];
	Anything config = fQuery["Destination"];
	Anything rStore = GetConfig()["RoleStore1"].DeepClone();
	coast::testframework::PutInStore(rStore, c.GetRoleStoreGlobal());

	StorePutter::Operate(toStore, c, config);

	TraceAny(c.GetRoleStoreGlobal(), "RoleStore after");

	ROAnything expectedStore(GetConfig()["Results"]["StorePutterTest"]["Store"]);
	assertAnyEqual(expectedStore, c.GetRoleStoreGlobal());
}

void AnythingUtilsTest::StorePutterReplaceRenderedTest() {
	StartTrace(AnythingUtilsTest.StorePutterReplaceRenderedTest);
	// Set up
	Context c;
	Anything toStore = fQuery["ToStore"];
	Anything config = fQuery["Destination"];
	Anything rStore = GetConfig()["RoleStore1"].DeepClone();
	coast::testframework::PutInStore(rStore, c.GetRoleStoreGlobal());

	TraceAny(c.GetRoleStoreGlobal(), "RoleStore before");
	assertAnyEqual(rStore, c.GetRoleStoreGlobal());
	StorePutter::Operate(toStore, c, config);

	TraceAny(c.GetRoleStoreGlobal(), "RoleStore after");

	ROAnything expectedStore(GetConfig()["Results"]["StorePutterTest"]["Store"]);
	assertAnyEqual(expectedStore, c.GetRoleStoreGlobal());
}

void AnythingUtilsTest::StorePutterEmptySlotTest() {
	StartTrace(AnythingUtilsTest.StorePutterEmptySlotTest);
	// Set up
	Context c;
	Anything config = fQuery["Destination"];
	Anything toStore = fQuery["ToStore"];
	StorePutter::Operate(toStore, c, config);

	TraceAny(c.GetTmpStore(), "TempStore after");
	assertEqual(0, c.GetTmpStore().GetSize());
}

void AnythingUtilsTest::StoreFinderTest() {
	StartTrace(AnythingUtilsTest.StoreFinderTest);
	// Set up
	Context c;
	Anything config = fQuery["ToFind"];
	Anything rStore = GetConfig()["RoleStore1"].DeepClone();
	coast::testframework::PutInStore(rStore, c.GetRoleStoreGlobal());

	Anything foundResult;
	StoreFinder::Operate(c, foundResult, config);

	ROAnything expectedStore(GetConfig()["Results"]["StoreFinderTest"]);
	assertAnyEqual(expectedStore, foundResult);

	config = fQuery["SessionToFind"];
	rStore = GetConfig()["SessionStore"].DeepClone();
	coast::testframework::PutInStore(rStore, c.GetSessionStore());

	Anything foundResult2;
	StoreFinder::Operate(c, foundResult2, config);
	assertAnyEqual(expectedStore, foundResult2);
}

void AnythingUtilsTest::StoreFinderRenderedTest() {
	StartTrace(AnythingUtilsTest.StoreFinderRenderedTest);
	// Set up
	Context c;
	Anything config = fQuery["ToFind"];
	Anything rStore = GetConfig()["RoleStore1"].DeepClone();
	coast::testframework::PutInStore(rStore, c.GetRoleStoreGlobal());

	Anything foundResult;
	StoreFinder::Operate(c, foundResult, config);

	ROAnything expectedStore(GetConfig()["Results"]["StoreFinderTest"]);
	assertAnyEqual(expectedStore, foundResult);
}

Test *AnythingUtilsTest::suite() {
	TestSuite *testSuite = new TestSuite;
	ADD_CASE(testSuite, AnythingUtilsTest, StoreCopierTest);
	ADD_CASE(testSuite, AnythingUtilsTest, StoreFinderTest);
	ADD_CASE(testSuite, AnythingUtilsTest, StoreFinderRenderedTest);
	ADD_CASE(testSuite, AnythingUtilsTest, StorePutterTest);
	ADD_CASE(testSuite, AnythingUtilsTest, StorePutterReplaceTest);
	ADD_CASE(testSuite, AnythingUtilsTest, StorePutterReplaceRenderedTest);
	ADD_CASE(testSuite, AnythingUtilsTest, StorePutterEmptySlotTest);
	return testSuite;
}
