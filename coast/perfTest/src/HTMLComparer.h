/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLComparer_H
#define _HTMLComparer_H


class Anything;
class String;

//---- HTMLComparer ----------------------------------------------------------
//!compares two HTML any trees
//! to be defined
//!
class HTMLComparer
{
public:
	//--- constructors
	HTMLComparer(Anything master, Anything slave);
	~HTMLComparer();

	//--- public api
	//!compares the two HTML trees
	//! \param the comparison report is written on this string
	//! \return true if they are considered equal, false otherwise
	//! \pre explanation of precondition for the method call
	//! \post explanation of postcondition for the method call
	bool Compare(String &report);

protected:
	//--- subclass api
	bool DoCompareAny(Anything &master, Anything &slave);
	bool DoCompareAnyArray(Anything &master, Anything &slave);
	void AddToPathStack(String path, int index);

	//--- member variables declaration
	Anything fMaster;
	Anything fSlave;
	Anything fPathStack;
private:
	// use careful, you inhibit subclass use
	//--- private class api
	// block the following default elements of this class
	// because they're not allowed to be used
	HTMLComparer();
	HTMLComparer(const HTMLComparer &);
	HTMLComparer &operator=(const HTMLComparer &);

	//--- private member variables
};

#endif
