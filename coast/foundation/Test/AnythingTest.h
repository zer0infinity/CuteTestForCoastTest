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
#include "Anything.h"

//---- AnythingTest -----------------------------------------------------------
//!testcases for Anything
class AnythingTest : public TestFramework::TestCase
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
	static Test	*suite ();
	void		Constructors ();
	void		DefaultConstrTest();
	void		IntConstrTest();
	void		LongConstrTest();
	void		FloatConstrTest();
	void		DoubleConstr0Test();
	void		DoubleConstr1Test();
	void		CharStarConstrTest();
	void		CharStarLongConstr0Test();
	void		CharStarLongConstr1Test();
	void		CharStarLongConstr2Test();
	void		CharStarLongConstr3Test();
	void		StringConstrTest();
	void		VoidStarLenConstrTest();
	void		EmptyVoidStarLenConstrTest();
	void		IFAObjectStarConstrTest();
	void		AnythingConstrTest();
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

	void		ReadFailsTest();
	void		WriteReadTest();
	void		WriteRead0Test();
	void		WriteRead1Test();
	void		WriteRead5Test();
	void		WriteRead7Test();
	void		WriteRead8Test();
	void		AnyIncludeTest();
	void		DeepClone0Test();
	void		DeepClone1Test();
	void		DeepClone2Test();
	void		DeepClone3Test();
	void		DeepClone4Test();
	void		DeepClone5Test();

	void		LookUpTest();
	void		EmptyLookup ();
	void		invPathLookup ();
	void		LookUp0Test();
	void		LookUp1Test();
//	void		LookUp2Test();
//	void		LookUp3Test();
//	void		LookUp4Test();
	void 		LookUpWithSpecialCharsTest();
	void		LookupCaseSensitiveTest();
	void 		LookupPathByIndex ();
	void        TypeTest();
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
