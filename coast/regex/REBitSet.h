/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _REBitSet_H
#define _REBitSet_H

#include "StringStream.h"

//!implement bitsets suitable for 256 bits corresponding to a set of 8 bit unsigned chars
class REBitSet
{
	enum eREBitSetConstants {
		//!internal constants to optionally adjust what is going on later....
		//!however 16 bit characters will make the bitsets unnecessarily large
		//!with the existing implementation style (=8kbytes per bitset)
		fgcSubSetBits = (sizeof(unsigned long) * 8),
		fgcSubSetMask = fgcSubSetBits - 1,
		fgcNofBits = 256,
		fgcSize = fgcNofBits / fgcSubSetBits, //number of longs used here
	};
	//!map of powers of 2 to access the individual bits, fgcSingletons[i] == 2**i
	static const unsigned long fgcSingletons[fgcSubSetBits];
	//!here is the set
	unsigned long fSet[fgcSize];
	//!access the right element in fSet
	unsigned long &Bucket(unsigned char index) {
		return fSet[index/fgcSubSetBits];
	}
	const unsigned long &Bucket(unsigned char index)const {
		return fSet[index/fgcSubSetBits];
	}
public:
	//!for calculating set contents dynamically based on a Predicate
	//!i.e. ctype.h's isalpha() isdigit() etc.
	typedef int (*Predicate)(int);

	//!make an empty or universal bitset
	REBitSet(bool setall = false);
	//!copy ctor to attain value semantics
	REBitSet(const REBitSet &b);
	//!special ctor for statcally initialized bitsets, because
	//!compiler didn't like REBitSet initialization with = { 1,2,3,4} syntax
	REBitSet(const unsigned long s[fgcSize]);
	//!typical set operators
	bool IsEqual(const REBitSet &b)const;
	bool IsSubSet(const REBitSet &b)const;
	bool operator==(const REBitSet &b) const {
		return IsEqual(b);
	}
	bool operator<=(const REBitSet &b) const {
		return IsSubSet(b);
	}
	REBitSet operator~() const;
	REBitSet &operator=(const REBitSet &b);
	REBitSet &operator|=(const REBitSet &b);
	REBitSet &operator&=(const REBitSet &b);
	REBitSet &operator^=(const REBitSet &b);
	//!set difference operator
	REBitSet &operator-=(const REBitSet &b); // &= ~
	//!in place complement *this = ~(*this)
	REBitSet &Complement();
	//!test the bit
	bool IsMember(unsigned char bit)const {
		return ((Bucket(bit)&Slot(bit)) != 0UL);
	}
	//!raise the bit to true/1
	REBitSet &Set(unsigned char bit) {
		Bucket(bit) |= Slot(bit);
		return *this;
	}
	//!raise a number of bits in a row
	REBitSet &Set(unsigned char from, unsigned char to);
	//!set all bits where the predicate evaluates to true
	REBitSet &Set(Predicate p); // set all bits where p evaluates to true
	//!set bit to false/0
	REBitSet &Unset(unsigned char bit) {
		Bucket(bit) &= ~Slot(bit);
		return *this;
	}
	//!toggle the bit
	REBitSet &Toggle(unsigned char bit) {
		Bucket(bit) ^= Slot(bit);
		return *this;
	}
	//!auxiliaries for bitsets made from a simple unsigned long
	static unsigned long Bit(unsigned char bit) {
		return Slot(bit);
	}
	static bool IsMember(unsigned long bits, unsigned char bit) {
		return ((bits & Slot(bit)) != 0UL);
	}
	static unsigned long Slot(unsigned char index) {
		return fgcSingletons[index&fgcSubSetMask];
	}
	//!auxiliary to generate ascii/hex representation useful for inclusion in programs
	friend std::ostream &operator<<(std::ostream & , const REBitSet &s);
};

#endif  /* REBitSet */
