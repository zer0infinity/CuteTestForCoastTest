/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _IT_TESTFW_TESTSLIST_H
#define _IT_TESTFW_TESTSLIST_H

class Test;

//---- TestElt ------------------------------------------------------
class TestElt {
	TestElt();
protected:
	Test *fValue;
	TestElt *fNext;
	TestElt(Test *aValue) :
			fValue(aValue), fNext(0) {
	}
	~TestElt();
	friend class TestList;
};

//---- TestList ------------------------------------------------------
//! simple list class for tests to avoid the use of STL
class TestList
{
protected:
	TestElt *fList;
	TestElt *fIn;
	TestElt *fIt;
	long fSize;

public:
	TestList(): fList(0), fIn(0), fIt(0), fSize(0) {};
	~TestList();

	Test *first();
	Test *next();
	void push_back( Test *aValue );
	long size() {
		return fSize;
	};
};

#endif
