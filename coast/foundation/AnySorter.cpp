/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "AnySorter.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "Anything.h"
#include "AnyComparers.h"

//--- c-library modules used ---------------------------------------------------
//! shuffles anys where lookuppath is not defined towards the end, keeping their original sequence
SpecialLookupComparer::SpecialLookupComparer(const char *lookuppath, const AnyComparer &theValueComparer)
	: AnyLookupValueComparer(lookuppath, theValueComparer)
{
}

int SpecialLookupComparer::Compare(const Anything &left, const Anything &right) const
{
	// would require more efficient LookupPath version to avoid copies (potential deepclones)
	Anything intleft, intright;
	bool leftfound = left.LookupPath(intleft, lookup);
	bool rightfound = right.LookupPath(intright, lookup);
	if (!leftfound && rightfound) {
		return 1;
	}
	if (!rightfound) {
		return -1;
	}
	return ac.Compare(intleft, intright);
}

void AnySorter::SortByKeyInArray(const String &sortFieldName, Anything &toSort, EMode mode, bool sortCritIsNumber)
{
	StartTrace(AnySorter.SortByKeyInArray);

//	static AnyStringValueComparer asc;
//	static AnyLongValueComparer asl;
//	AnyComparer &cm = (sortCritIsNumber? (AnyComparer&)asl:(AnyComparer&)asc);
	// this ugly code is for brain dead legacy gcc compiler...
	// it doesn't seem to correctly initialize static instances, resp. their vtables
	if (mode == AnySorter::asc) {
		if (sortCritIsNumber) {
			toSort.SortByAnyComparer(SpecialLookupComparer(sortFieldName, AnyLongValueComparer()));
		} else {
			toSort.SortByAnyComparer(SpecialLookupComparer(sortFieldName, AnyStringValueComparer()));
		}
	} else {
		if (sortCritIsNumber) {
			toSort.SortByAnyComparer(SpecialLookupComparer(sortFieldName, AnyReverseComparer(AnyLongValueComparer())));
		} else {
			toSort.SortByAnyComparer(SpecialLookupComparer(sortFieldName, AnyReverseComparer(AnyStringValueComparer())));
		}

	}
//	toSort.SortByAnyComparer(SpecialLookupComparer(sortFieldName,(const AnyComparer&)(() ?
//	(sortCritIsNumber? asl:asc) : AnyReverseComparer(sortCritIsNumber? asl:asc))));
	TraceAny(toSort, "toSort");
}
