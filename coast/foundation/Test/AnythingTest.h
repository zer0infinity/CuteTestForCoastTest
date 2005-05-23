/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingTest_H
#define _AnythingTest_H

#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "Anything.h"

//---- AnythingTest -----------------------------------------------------------
//!testcases for Anything
class AnythingTest : public TestCase
{
protected:
	Anything fString;
	Anything fLong;
	Anything fBool;
	Anything fFloat;
	Anything fDouble;
	Anything fDouble2;
	Anything fNull;
	Anything fArray;
	Anything fSequence;
	Anything fSecondLevel;
	Anything fThirdLevel;
	Anything fNullSlot;
	Anything fSecondLevelNull;
	Anything fQuery;
	Anything fConfig;

public:
	AnythingTest (TString tstrName);

	virtual void	setUp ();
	static Test	*worksuite ();
	static Test	*suite ();
	void		Constructors ();
	void		testDefaultConstr ();
	void		testIntConstr ();
	void		testLongConstr ();
	void		testFloatConstr ();
	void		testDoubleConstr0 ();
	void		testDoubleConstr1 ();
	void		testCharStarConstr ();
	void		testCharStarLongConstr0 ();
	void		testCharStarLongConstr1 ();
	void		testCharStarLongConstr2 ();
	void		testCharStarLongConstr3 ();
	void		testStringConstr ();
	void		testVoidStarLenConstr ();
	void		testEmptyVoidStarLenConstr ();
	void		testIFAObjectStarConstr ();
	void		testAnythingConstr ();
	void		ImportTest ();

	void		KeyDeletion ();
	void 		SimpleRemove ();
	void 		RemoveInvKeys ();
	void		KeyDeletion0 ();
	void		KeyDeletion1 ();
	void		KeyDeletion2 ();
	void 		KeyDeletion3 ();

	void		IndexAccess ();

	void		KeyAccess ();
	void		KeyAccess0 ();
	void		KeyAccess1 ();
	void		KeyAccess2 ();
	void		MixKeysAndArray ();
	void		Recursive();

	void		EmptyAccess ();
	void		EmptyAccess0 ();
	void		EmptyAccess1 ();

	void		testReadFails();
	void		testWriteRead ();
	void		testWriteRead0 ();
	void		testWriteRead1 ();
	void		testWriteRead5 ();
	void		testWriteRead7 ();
	void		testWriteRead8 ();
	void		AnyIncludeTest();
	void		testDeepClone0();
	void		testDeepClone1();
	void		testDeepClone2();
	void		testDeepClone3();
	void		testDeepClone4();
	void		testDeepClone5();

	void		testLookUp();
	void		EmptyLookup ();
	void		invPathLookup ();
	void		testLookUp0 ();
	void		testLookUp1 ();
//	void		testLookUp2 ();
//	void		testLookUp3 ();
//	void		testLookUp4 ();
	void 		testLookUpWithSpecialChars ();
	void		testLookupCaseSensitive();
	void 		LookupPathByIndex ();
	void        testType ();
	void        RefSlotTest ();
	void		SuccessiveAssignments();

	void		operatorAssignemnt();
	void		appendTest();
	void		boolOperatorAssign();
	void		intOperatorAssign();
	void		ifaObjectOperatorAssign();

	void		roConversion();
	void		boolROConversion();
	void		intROConversion();
	void		ifaObjectROConversion();
	void		BinaryBufOutput();
	void		String2LongConversion();

	void		AsCharPtrBufLen();
	void		RefCount();

	//-- new test check for ref integrity with deepclone
	void		DeepCloneWithRef();

	void 		SlotFinderTest();
	void 		SlotCopierTest();
	void 		SlotPutterTest();
	void 		SlotPutterAppendTest();
	void 		AnythingLeafIteratorTest();
	void 		SlotnameSorterTest();
	void 		SlotCleanerTest();

protected:
	Anything	init5DimArray(long);
	bool		check5DimArray( Anything &, Anything &, long );
	void 		intLookupPathCheck(Anything &test, const char *path);
};

#endif		//ifndef _AnythingTest_H
