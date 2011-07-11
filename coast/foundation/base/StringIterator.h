/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef StringIterator_H
#define StringIterator_H

#include <iterator>

class String;
typedef std::iterator<std::random_access_iterator_tag, char> StringIteratorBase;

class String_iterator: public StringIteratorBase {
	friend class String;
protected:
	String *a;
	long position;
	String_iterator(String *s, long p = 0) :
		a(s), position(p) {
	}
public:
	bool operator==(const String_iterator &r) const;
	bool operator!=(const String_iterator &r) const {
		return !(this->operator==(r));
	}
	bool operator<(const String_iterator &r) const;
	bool operator>=(const String_iterator &r) const {
		return !(this->operator<(r));
	}
	bool operator>(const String_iterator &r) const {
		return r.operator<(*this);
	}
	bool operator<=(const String_iterator &r) const {
		return !(r.operator<(*this));
	}
	reference operator*() const;
	reference operator[](difference_type index) const;
	difference_type operator-(const String_iterator &r) const;
	String_iterator operator+(difference_type index) const {
		return String_iterator(a, position + index);
	}
	String_iterator operator-(difference_type index) const {
		return String_iterator(a, position - index);
	}
	String_iterator &operator++() {
		++position;
		return *this;
	}
	String_iterator &operator--() {
		--position;
		return *this;
	}
	String_iterator operator++(int) {
		return String_iterator(a, position++);
	}
	String_iterator operator--(int) {
		return String_iterator(a, position--);
	}
	String_iterator &operator+=(difference_type index) {
		position += index;
		return *this;
	}
	String_iterator &operator-=(difference_type index) {
		position -= index;
		return *this;
	}
	String const* base() const {
		return a;
	}
	long pos() const {
		return position;
	}
};

// no direct support for const_iterators, need to spell out std::iterator template parameters
typedef std::iterator<std::random_access_iterator_tag, char const, ptrdiff_t, char const*, char const &> ConstStringIteratorBase;

class String_const_iterator: public ConstStringIteratorBase {
	friend class String;
protected:
	const String * a;
	long position;
	String_const_iterator(const String * s, long pos = 0) :
		a(s), position(pos) {
	}
public:
	String_const_iterator(const String_iterator & r) :
		a(r.base()), position(r.pos()) {
	}

	bool operator ==(const String_const_iterator & r) const;
	bool operator !=(const String_const_iterator & r) const {
		return !(this->operator==(r));
	}

	bool operator <(const String_const_iterator & r) const;
	bool operator >=(const String_const_iterator & r) const {
		return !(this->operator<(r));
	}

	bool operator >(const String_const_iterator & r) const {
		return r.operator<(*this);
	}

	bool operator <=(const String_const_iterator & r) const {
		return !(r.operator<(*this));
	}

	reference operator *() const;

	reference operator [](difference_type index) const;
	difference_type operator-(const String_const_iterator &r) const;
	String_const_iterator operator+(difference_type index) const {
		return String_const_iterator(a, position + index);
	}
	String_const_iterator operator-(difference_type index) const {
		return String_const_iterator(a, position - index);
	}
	String_const_iterator &operator++() {
		++position;
		return *this;
	}
	String_const_iterator &operator--() {
		--position;
		return *this;
	}
	String_const_iterator operator++(int) {
		return String_const_iterator(a, position++);
	}
	String_const_iterator operator--(int) {
		return String_const_iterator(a, position--);
	}
};

#endif
