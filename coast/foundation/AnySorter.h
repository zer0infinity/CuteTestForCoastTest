/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ANYSORTER_H
#define _ANYSORTER_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
//!---- Forward declarations ---------------------------------------------------------
class String;
class Anything;
//---- AnySorter ---------------------------------------------------------------------
//!Sorts operations on Anything
class EXPORTDECL_FOUNDATION AnySorter
{
public:
	enum EMode { asc, desc };
	/*! Sorts array entries having the slot defined by sortFieldName.
		Array entries which don't have such a slot and non-array entries are
		preserved in the sorted result by appending them in the order of their
		appearence in the data to be sorted at the end of the sorted data.
		Args: sortFieldName: slotname of slot containing the date to be used as sort key
			  toSort in: data to be sorted. out: sorted data.
			  mode: sortorder, asc/desc
	*/
	static void SortByKeyInArray(const String &sortFieldName, Anything &toSort, EMode mode = asc, bool sortCritIsNumber = false);
};
#endif
