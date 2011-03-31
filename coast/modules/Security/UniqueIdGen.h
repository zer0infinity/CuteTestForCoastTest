/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UniqueIdGen_H
#define _UniqueIdGen_H

class MD5Signer;

//---- other includes      -----------------------------------------------
#include "ITOString.h"

//---- UniqueIdGen ----------------------------------------------------------
//! Generate a Unique ID
/*!
Returns a String containing a unique id.
*/
class UniqueIdGen
{
public:
	//--- constructors
	UniqueIdGen();
	~UniqueIdGen();
	//! Generate a UniqueId. You can pass an additional token, e.g. the calling
	//! thread's id.
	static String GetUniqueId(const String &additionalToken = "NoAdditionaTokenWasSet");

protected:
	static String HexHash(MD5Signer &md5, const char *s);

private:
	//! inhibit compiler supplied bitwise default copy ctor
	UniqueIdGen(const UniqueIdGen &);
	//! inhibit compiler supplied bitwise default assignment operator
	UniqueIdGen &operator=(const UniqueIdGen &);
};

#endif
