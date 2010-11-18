/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyComparers_H
#define _AnyComparers_H

//---- baseclass include -------------------------------------------------
#include "Anything.h"

//---- AnyComparers ----------------------------------------------------------
//! convenience classes implementing the AnyCompare interface
//class AnyComparer {
//public:
//	virtual int Compare(const Anything &left,const Anything &right)const=0;
//};

class AnyStringValueComparer: public AnyComparer
{
public:
	int Compare(const Anything &left, const Anything &right)const {
		return left.AsString().Compare(right.AsString());
	}
};
class AnyReverseComparer: public AnyComparer
{
	const AnyComparer &ac;
public:
	AnyReverseComparer(const AnyComparer &theComparer)
		: ac(theComparer) {}
	int Compare(const Anything &left, const Anything &right)const {
		return - ac.Compare(left, right);
	}
};

class AnyLongValueComparer: public AnyComparer
{
public:
	int Compare(const Anything &left, const Anything &right)const {
		long l = left.AsLong();
		long r = right.AsLong();
		if (l < r) {
			return -1;
		}
		if (l > r) {
			return 1;
		}
		return 0;
	}
};
class AnyDoubleValueComparer: public AnyComparer
{
public:
	int Compare(const Anything &left, const Anything &right)const {
		double l = left.AsDouble();
		double r = right.AsDouble();
		if (l < r) {
			return -1;
		}
		if (l > r) {
			return 1;
		}
		return 0;
	}
};

class AnyLookupValueComparer: public AnyComparer
{
protected:
	String lookup;
	const AnyComparer &ac;
public:
	AnyLookupValueComparer(const char *lookuppath, const AnyComparer &theValueComparer)
		: lookup(lookuppath), ac(theValueComparer) {}
	int Compare(const Anything &left, const Anything &right)const {
		// would require more efficient LookupPath version to avoid copies (potential deepclones)
		Anything intleft, intright;
		left.LookupPath(intleft, lookup);
		right.LookupPath(intright, lookup);
		return ac.Compare(intleft, intright);
	}
};

#endif
