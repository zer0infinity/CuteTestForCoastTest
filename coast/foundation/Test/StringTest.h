/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringTest_h_
#define _StringTest_h_

#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOString.h"
//---- StringTest -----------------------------------------------------------
//!testcases for String
class StringTest : public TestCase
{
protected:
	String fShort;
	String fLong;

public:
	StringTest (TString name); // : TestCase (name) {}
	virtual ~StringTest();

	virtual void			setUp ();
	static Test				*worksuite ();
	static Test				*suite ();

	void			constructors ();
	void			appendsChar ();
	void			appendsCharChain ();
	void			appendsNullPtr ();
	void			appendsString ();
	void			appendsLong ();
	void			appendsDouble ();
	void			appendsAsHex ();
	void            appendsTwoHexAsChar ();
	void			appendsFile ();
	void 			appendsWithDelimiter();

	void			methods ();

	void 			toLowers();
	void toLower0();

	void			toUppers();
	void toUpper0();

	void			dumps();
	void dump0();
	void 			DumpAsHexTest();

	void asLong();
	void asDouble();
	void asLongLong();

	void startsWith();

	void			caselessCompares();
	void caselessCompare0();
	void caselessCompare1();
	void caselessCompare2();
	void caselessCompare3();
	void caselessCompareWithNullPtr();

	void			intPrintOns();
	void intPrintOn0();
	void intPrintOn1();

//	void			intReadFroms();
//	void intReadFrom0();

	void			isEquals ();
	void isEqual0 ();
	void isEqual1 ();
	void isEqual2 ();

	void			compares();
	void			compareWithNullBytes();
	void compare0 ();

	void			compareNs();
	void compareN0 ();
	void compareN1 ();

	void 			copyTos ();
	void copyTo0 ();
	void copyTo1 ();
	void copyTo2 ();
	void copyTo3 ();
	void copyTo4 ();

	void 			ats();
	void at0 ();

	void			putAts();
	void putAt0 ();

	void			replaceAts();
	void replaceAt0 ();
	void replaceAt1 ();
	void replaceAt2 ();
	void replaceAtBeyondLength ();
	void replaceAt5 ();
	void replaceAt6 ();
	void replaceAt7 ();
	void replaceAt8 ();
	void replaceAt9 ();

	void			strChrs ();
	void	strChr0 ();
	void	strChr1 ();

	void			strRChrs ();
	void	strRChr0 ();

	void			contains ();
	void contain0 ();
	void TestFirstCharOf();
	void TestLastCharOf();
	void TestContainsCharAbove();

	void			subStrings ();
	void subString0 ();
	void subString1 ();
	void subString2 ();
	void subString3 ();
	void subString4 ();
	void subString5 ();
	void subStringWithNull ();

	void			trimFronts ();
	void trimFront0 ();
	void trimFront1 ();
	void trimFront2 ();
	void trimFront3 ();

	void trimFrontXXX ();
	void trimFrontEmpty ();
	void trimFrontOneByte ();
	void			trims ();
	void trim0 ();
	void trim1 ();
	void trim2 ();
	void trim3 ();
	void trim4 ();

	void			reserve ();

	void TestCapacity ();
	void GetLine ();
	void OptimizedConstructorOrAssignment();

	void EmptyAllocatorTest();
	void ReplaceTest();
	void IntReplaceTest(String orig, const String &prefix, const String &nStr, bool exp, const String expStr);
};

#endif
