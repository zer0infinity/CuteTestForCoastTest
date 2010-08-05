/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef IT_TESTFW_TString_H
#define IT_TESTFW_TString_H

#if defined(__370__) || defined(_ARCH_COM) || defined(WIN32) || defined(__SUNPRO_CC) || __GNUC__ >= 3
#include <string.h>
#endif

#if defined(__370__)
typedef long l_long;
#endif
#if defined(__370__) || defined(_ARCH_COM) || (defined(__SUNPRO_CC) && __SUNPRO_CC < 0x500)
typedef int bool;
#ifndef true
#define true (bool)1
#endif
#ifndef false
#define false (bool)0
#endif
#endif

#if defined(ONLY_STD_IOSTREAM)
#include <iosfwd>
using std::istream;
using std::ostream;
#else
class istream;
class ostream;
#endif
typedef int Unicode;

#define NotNull(s) ((s) ? (s) : "null")
#define NotNullStr(s) ((s.Length() > 0) ? (const char *)(s) : "null")
//---- TString --------------------------------------------------------------
//! trivial implementation of a string class
class TString
{
	void alloc(long capacity); // factor out mem alloc for ctors
public:
	TString();
	TString(long capacity);
	TString(const char *s, long length = -1);
	TString(const TString &s);
	~TString();

	TString &operator= (const char *s);
	TString &operator= (const TString &s);

	bool IsEqual(const char *other) const;
	int  Compare(const char *other) const;

	void ToUpper();

	//! output routine to dump the string content as hexadecimal numbers in the form:
	//!
	void DumpAsHex(TString &str, long dumpwidth = 16L) const;

	//! compare two strings and return a string containing the diffs
	//! \return long, position of first difference, -1 if no difference
	long DiffDumpAsHex(TString &outbuf, const TString &strRight) const;

	// return value for convenient multi-appends
	TString &Append(char);
	TString &Append(const char *s, long length = -1);
	TString &Append(const TString &s);

	// Abfrage-Methode
	inline long Length() const				{
		return fLength;
	}

	// the following three may allow us to get rid of
	// sprintfs completely
	TString &Append(int number);
	TString &Append(long number);
	TString &Append(unsigned long number);
	TString &Append(long long number);
	TString &Append(unsigned long long number);
	TString &Append(double number);
	TString &AppendAsHex(unsigned char cc);
	TString &AppendTwoHexAsChar(const char *cc);

	// stream operators
	TString &operator << (char c)				{
		return Append(c);
	}
	TString &operator << (long l)				{
		return Append(l);
	}
	TString &operator << (double d)				{
		return Append(d);
	}
	TString &operator << (bool b)				{
		return Append((long)b);
	}
	TString &operator << (const char *s)		{
		return Append(s);
	}
	TString &operator << (const TString &s)		{
		return Append(s);
	}

	friend inline bool operator==(const TString &s1, const TString &s2);
	friend inline bool operator==(const char *s1, const TString &s2);
	friend inline bool operator==(const TString &s1, const char *s2);
	friend inline bool operator!=(const TString &s1, const TString &s2);
	friend inline bool operator!=(const char *s1, const TString &s2);
	friend inline bool operator!=(const TString &s1, const char *s2);

	// convenient operator for easier use
	// when GetLength is not needed one can use const char * params
	operator const char *() const;

protected:
	friend istream &operator>>(istream &is, TString &s);
	friend ostream  &operator<<(ostream &os, const TString &s);

	friend istream  &getline(istream &is, TString &s, char c);

	void Set(long start, const char *s, long l);

protected:
	char *fCont;
	long fCapacity;
	long fLength;
};

// inlines
inline TString::operator const char *() const
{
	return (fCont) ? fCont : "";
}

inline int TString::Compare(const char *other) const
{
	if (fCont && other) {
		return strcmp(fCont, other);
	} else if ((fCont && *fCont) && ! other ) {
		return 1;
	} else if (! fCont && (other && *other) ) {
		return -1;
	} else {
		return 0;    // both are empty
	}
}

inline bool TString::IsEqual(const char *other) const
{
	return ! this->Compare(other);
}

bool operator==(const TString &s1, const TString &s2)
{
	return s1.IsEqual(s2);
}
bool operator==(const char *s1, const TString &s2)
{
	return s2.IsEqual(s1);
}
bool operator==(const TString &s1, const char *s2)
{
	return s1.IsEqual(s2);
}
bool operator!=(const TString &s1, const TString &s2)
{
	return !s1.IsEqual(s2);
}
bool operator!=(const char *s1, const TString &s2)
{
	return !s2.IsEqual(s1);
}
bool operator!=(const TString &s1, const char *s2)
{
	return !s1.IsEqual(s2);
}

//! small hack to replace missing toupper system function
//!  (could be done faster with a table)
inline void TString::ToUpper()
{
	for (long i = 0, sz = Length(); i < sz; ++i) {
		if (('a' <= fCont[i]) && (fCont[i] <= 'z')) {
			fCont[i] = fCont[i] - 'a' + 'A';
		}
	}
}

#endif
