/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AnythingIterator.h"
#include "Anything.h"
#include <limits>

bool Anything_iterator::operator==(const Anything_iterator &r) const
{
	return a == r.a && position == r.position;
}

bool Anything_iterator::operator<(const Anything_iterator &r) const
{
	if (a == r.a) {
		return position < r.position;
	}
	return false;
}

Anything_iterator::reference Anything_iterator::operator*() const
{
	return a[position];
}

Anything_iterator::reference
Anything_iterator::operator[](Anything_iterator::difference_type offset) const
{
	return a[position+offset];
}

Anything_iterator::difference_type
Anything_iterator::operator-(const Anything_iterator &r) const
{
	if (a == r.a) {
		return position - r.position;
	} else { // this is an error... TODO: determine a useful default e.g. numeric_limits<difference_type>::max()
		return std::numeric_limits<Anything_iterator::difference_type>::max();
	}
}

bool Anything_const_iterator::operator==(const Anything_const_iterator &r)const
{
	return a == r.a && position == r.position;
}

bool Anything_const_iterator::operator<(const Anything_const_iterator &r) const
{
	if (a == r.a) {
		return position < r.position;
	}
	return false;
}

const Anything &Anything_const_iterator::operator*() const
{
	return a[position];
}

Anything_const_iterator::reference
Anything_const_iterator::operator[](Anything_const_iterator::difference_type offset) const
{
	return a[position+offset];
}

Anything_const_iterator::difference_type
Anything_const_iterator::operator-(const Anything_const_iterator &r) const
{
	if (a == r.a) {
		return position - r.position;
	} else { // this is an error... TODO: determine a useful default e.g. numeric_limits<difference_type>::max()
		return std::numeric_limits<Anything_const_iterator::difference_type>::max();
	}
}
