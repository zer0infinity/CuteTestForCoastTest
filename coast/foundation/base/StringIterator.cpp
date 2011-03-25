/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringIterator.h"
#include "ITOString.h"
#include <limits>

bool String_iterator::operator==(const String_iterator &r) const {
	return a == r.a && position == r.position;
}

bool String_iterator::operator<(const String_iterator &r) const {
	if (a == r.a) {
		return position < r.position;
	}
	return false;
}

String_iterator::reference String_iterator::operator*() const {
	return a[position];
}

String_iterator::reference String_iterator::operator[](difference_type index) const {
	return a[position + index];
}

String_iterator::difference_type String_iterator::operator-(const String_iterator &r) const {
	if (a == r.a) {
		return position - r.position;
	} else {
		return std::numeric_limits<String_iterator::difference_type>::max();
	}
}

String_const_iterator& String_const_iterator::operator=(String_const_iterator const &r) {
	 const_cast<String*>(&a)->operator=(r.a);
	 position = r.position;
	 return *this;
}//lint !e1529

bool String_const_iterator::operator==(const String_const_iterator &r) const {
	return a == r.a && position == r.position;
}

bool String_const_iterator::operator<(const String_const_iterator &r) const {
	if (a == r.a) {
		return position < r.position;
	}
	return false;
}

String_const_iterator::value_type String_const_iterator::operator*() const {
	return a[position];
}

String_const_iterator::value_type String_const_iterator::operator[](difference_type index) const {
	return a[position + index];
}

String_const_iterator::difference_type String_const_iterator::operator-(const String_const_iterator &r) const {
	if (a == r.a) {
		return position - r.position;
	} else {
		return std::numeric_limits<String_const_iterator::difference_type>::max();
	}
}
