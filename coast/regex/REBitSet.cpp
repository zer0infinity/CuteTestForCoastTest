/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "REBitSet.h"

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

REBitSet::REBitSet(bool setall)
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] = setall ? ~0L : 0L;
	}
}
REBitSet::REBitSet(const REBitSet &b)
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] = b.fSet[i];
	}
}
REBitSet::REBitSet(const unsigned long s[fgcSize])
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] = s[i];
	}
}

bool REBitSet::IsEqual(const REBitSet &b)const
{
	bool res = true;
	for (unsigned i = 0; res && i < fgcSize; ++i) {
		res = (fSet[i] == b.fSet[i]) && res;
	}
	return res;
}
bool REBitSet::IsSubSet(const REBitSet &b)const
{
	bool res = true;
	for (unsigned i = 0; res && i < fgcSize; ++i) {
		res = ((fSet[i] & b.fSet[i]) == fSet[i]) && res;
	}
	return res;

}
REBitSet REBitSet::operator~() const
{
	REBitSet res;
	for (unsigned i = 0; i < fgcSize; ++i) {
		res.fSet[i] = ~fSet[i];
	}
	return res;
}

REBitSet &REBitSet::Complement()
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] = ~fSet[i];
	}
	return *this;
}
REBitSet &REBitSet::operator=(const REBitSet &b)
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] = b.fSet[i];
	}
	return *this;
}
REBitSet &REBitSet::operator|=(const REBitSet &b)
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] |= b.fSet[i];
	}
	return *this;
}
REBitSet &REBitSet::operator&=(const REBitSet &b)
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] &= b.fSet[i];
	}
	return *this;
}

REBitSet &REBitSet::operator^=(const REBitSet &b)
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] ^= b.fSet[i];
	}
	return *this;
}
REBitSet &REBitSet::operator-=(const REBitSet &b)
{
	for (unsigned i = 0; i < fgcSize; ++i) {
		fSet[i] &= ~b.fSet[i];
	}
	return *this;
}

REBitSet &REBitSet::Set(unsigned char from, unsigned char to)
{
	unsigned toidx = to;
	for (unsigned c = from; c <= toidx; ++c) {
		this->Set((unsigned char)c);
	}

	return *this;
}

REBitSet &REBitSet::Set(Predicate p)
{
	for (unsigned c = 0; c < fgcNofBits; ++c) {
		if ((*p)((unsigned char)c)) {
			this->Set((unsigned char)c);
		} else {
			this->Unset((unsigned char)c);
		}
	}

	return *this;
}

ostream &operator<<(ostream &os, const REBitSet &s)
{
	for (unsigned long i = 0; i < REBitSet::fgcSize; ++i) {
		os << hex << "0x" << s.fSet[i] << "UL ,";
	}
	return os;
}

const unsigned long REBitSet::fgcSingletons[fgcSubSetBits] = {
	0x1,
	0x2,
	0x4,
	0x8,
	0x10,
	0x20,
	0x40,
	0x80,
	0x100,
	0x200,
	0x400,
	0x800,
	0x1000,
	0x2000,
	0x4000,
	0x8000,
	0x10000,
	0x20000,
	0x40000,
	0x80000,
	0x100000,
	0x200000,
	0x400000,
	0x800000,
	0x1000000,
	0x2000000,
	0x4000000,
	0x8000000,
	0x10000000,
	0x20000000,
	0x40000000,
	0x80000000
};

