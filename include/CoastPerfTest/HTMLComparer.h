/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLComparer_H
#define _HTMLComparer_H

#include "Anything.h"

//---- HTMLComparer ----------------------------------------------------------
//!compares two HTML any trees
//! to be defined
//!
class HTMLComparer {
	HTMLComparer();
	HTMLComparer(const HTMLComparer &);
	HTMLComparer &operator=(const HTMLComparer &);
public:
	HTMLComparer(Anything master, Anything slave) :
		fMaster(master), fSlave(slave), fPathStack() {
	}
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

	Anything fMaster, fSlave, fPathStack;
};

#endif
