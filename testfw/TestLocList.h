/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TESTLOCLIST_H
#define IT_TESTFW_TESTLOCLIST_H

class TestLocation;

//---- TestLocElt ------------------------------------------------------
class TestLocElt
{
protected:
	TestLocation	*fValue;
	TestLocElt		*fNext;

	TestLocElt( TestLocation *aValue ): fValue(aValue),  fNext(0) { };
	~TestLocElt();

	friend class TestLocList;
};

//---- TestLocList ------------------------------------------------------
//! <B>simple list of test locations to avoid the use of STL</B>
class TestLocList
{
protected:
	TestLocElt *fList;
	TestLocElt *fIn;
	TestLocElt *fIt;
	long		fSize;

public:
	TestLocList(): fList(0), fIn(0), fIt(0), fSize(0) {};
	~TestLocList();
	TestLocation *first();
	TestLocation *next();
	void push_back( TestLocation *aValue );
	long size() {
		return fSize;
	};
};

#endif
