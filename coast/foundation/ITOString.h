/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ITOString_H
#define _ITOString_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOStorage.h"

#if defined(ONLY_STD_IOSTREAM)
#include <iostream>
#include <iomanip>
using namespace std;
#else
#if !defined(WIN32)
#include <streambuf.h>
#endif
#include <iostream.h>
#include <iomanip.h>
#endif

//---- String --------------------------------------------------------------
//!simple mt-safe string handling class
//!mt-safe string handling class that eases use of strings and its memory management, a notorious source of errors in c and c++
class EXPORTDECL_FOUNDATION String
{
	// factor out mem alloc for ctors and Set()
	//!allocation bottleneck, writes Syslog if memory exhausted
	void alloc(long capacity);
	//!calculate the real capacity to ask for, optimization for
	//!saving calls to alloc on expanding the string
	//!strategy is: start with a minimum size, double in the middle
	//!and  round to the next 1k for large strings (> 4k)
	//!new allocators provide additional support exploited by alloc to
	//!find an optimal size for allocation
	long allocCapacity(long newLength);

public:
	//! default ctor, makes an empty string
	String( Allocator *a = Storage::Current());
	//! ctor, reserves buffer spaces with capacity bytes
	//! \param capacity number of bytes reserved
	String(long capacity, Allocator *a = Storage::Current());
	//! ctor, creates string with a copy of s
	//! \param s initial value of String object, may be 0
	//! \param length take at most length characters from s, if negative use strlen(s)
	String(const char *s, long length = -1,  Allocator *a = Storage::Current());
	//! ctor, creates string with a copy of binary buffer s
	//! \param s initial value of String object
	//! \param length take exactly length bytes from s
	//! \pre length >= 0 AND s != 0, false implies empty string
	String(void *s, long length,  Allocator *a = Storage::Current());
	//! copy ctor
	String(const String &s, Allocator *a = Storage::Current());
	//! dtor, deallocates memory used by string content
	~String();
	//! assignment with character pointers, uses strlen(s)
	//! \param s new value of string, if 0 makes this empty
	String &operator= (const char *s);
	//! assignment with new String value
	//! \param s new value of this, no buffer sharing
	String &operator= (const String &s);

	//! append a character (single byte)
	//! return value for convenient multi-appends
	String &Append(char);
	//! append a char * string with at most length bytes
	//! return value for convenient multi-appends
	//! \param s appended value, may be 0, then nothing is appended
	//! \param length take at most length characters from s, if negative use strlen(s)
	String &Append(const char *s, long length = -1);
	//! append length bytes from binary buffer s
	//! return value for convenient multi-appends
	String &Append(const void *s, long length);
	//! append String s
	//! return value for convenient multi-appends
	String &Append(const String &s);
	//! append length bytes from stream is
	//! read length bytes as is.
	//! return value for convenient multi-appends
	String &Append(istream &is, long length); //
	//! append string from is,  works like istream::get(char *, len, delim)
	//! return value for convenient multi-appends
	String &Append(istream &is, long length, char delim);

	//! append integral number as decimal
	String &Append(long number);
	//! append floating point number as decimal
	String &Append(double number);
	//! append extra long number as decimal
#ifndef __370__
	String &Append(l_long number);
#endif
	//! append extra long unsigned number as decimal
#if defined(WIN32)
	String &Append(u_long number);
#endif
	//! append a single byte in hexadecimal encoding, results in two characters added
	String &AppendAsHex(unsigned char cc);

	//! append len single bytes in hexadecimal encoding, results in two characters added
	//! \param cc  - points a string of bytes
	//! \param len count of bytes to be hex encoded
	//! \param delimiter delimiter used between two encoded byes (default ('\0') -> no delimiter)
	String &AppendAsHex(const unsigned char *cc, long len, char delimiter = '\0');

	//! decode a hexadecimal number with two digits pointed to by cc, append the resulting byte
	//! \param cc points to a hex encoding,
	//! \pre *cc and *(cc+1) are in [0-9A-Fa-f], if not result is undefined
	String &AppendTwoHexAsChar(const char *cc);

	//! decode len hexadecimal numbers with two digits pointed to by cc, append the resulting bytes
	//! \param cc points to a hex encoding
	//! \param len count of two digits hexadecimal numbers to be decoded
	//! \param delimiter if there is a delimiter between the hexadecimal numers use true else false
	//! \pre if delimiter == true : for n=0..len -> *(cc+2*n) and *(cc+2*n+1) are in [0-9A-Fa-f], if not result is undefined
	//!       if delimiter == false: for n=0..len -> *(cc+3*n) and *(cc+3*n+1) are in [0-9A-Fa-f], if not result is undefined
	String &AppendTwoHexAsChar(const char *cc, long len, bool delimiter = false);

	//! returns a long representation of the implementation if possible else the default
	long AsLong(long dflt);

	//! returns a long long representation of the implementation if possible else the default
	l_long AsLongLong(l_long dflt);

	//! returns a double representation of the implementation if possible else the default
	double AsDouble(double dflt);

	// streaming like operators
	String &operator << (char c)				{
		return Append(c);
	}
	String &operator << (int i)					{
		return Append((long)i);
	}
	String &operator << (long l)				{
		return Append(l);
	}
	String &operator << (bool b)				{
		return Append((long)b);
	}
	String &operator << (double d)				{
		return Append(d);
	}
#ifndef __370__
	String &operator << (l_long ll)				{
		return Append(ll);
	}
#endif

#if defined(WIN32)
	String &operator << (u_long ul)				{
		return Append(ul);
	}
#endif

	String &operator << (const char *s)			{
		return Append(s);
	}
	String &operator << (const String &s)		{
		return Append(s);
	}

	//! canonical Add operation with optimal buffer handling, see OOPSLA 98
	String    Add(const String &) const ;

	//! convenient operator for easier use when GetLength is not needed one can use const char * params
	operator const char *() const;
	//! compare this with char *, return true if equal
	bool IsEqual(const char *other) const;
	//! compare this with String, return true if equal
	bool IsEqual(const String &other) const;
	//! compare this with String
	//! \return integer like strcmp for lexicographical order
	int  Compare(const String &other) const;
	//! compare this with char *
	//! \return integer like strcmp for lexicographical order
	int  Compare(const char *other) const;
	//! compare this with at most length bytes from char *
	//! \param other partner for comparison
	//! \param length number of chars to compare
	//! \param start optional offset from the beginning of other
	//! \return integer like strcmp for lexicographical order
	int  CompareN(const char *other, long length, long start = 0) const;
	//! reveal the Length of the String
	long Length() const;
	//! reveal the Capacity of the underlying buffer
	long Capacity() const;
	//! cut (remove) newStart characters from the begining of the String
	void TrimFront(long newStart) ;
	//! cut (remove) characters from the end of the String, make it shorter
	//! no-op if newlen >= Length()
	void Trim(long newlen) ;
	//! ensure internal buffer contains at least minreserve characters
	//! \post Capacity() >= minreserve
	void Reserve(long minreserve) ;
	//! take n bytes starting at pos from this and place them to buf
	//! I am not sure if that operation is still useful
	//! \pre buf AND pos>=0 AND n <= Length()-pos
	long CopyTo(char *buf, long n, long pos) const;
	//! take n characters from buf and place them into this at position pos
	//! I am not sure if that operation is still useful, it has been used by old IFAStringStream
	//! \pre buf AND n > 0 AND pos >= 0
	void ReplaceAt(long pos, const char *buf, long n) ;
	//! place a character at a defined position within String
	//! adjusts string size if required
	//! an char & operator[](long pos) would give us problems.
	//! \pre pos>=0
	void PutAt(long pos, char what);
	//! create a new String containing a part of this
	//! \param start character index to start from >= 0
	//! \param length how many characters come into the new string <0 means to the end
	String SubString(long start, long length = -1) const;
	//! create a new string by finding first occurence of pattern in String
	//! works like strstr
	String SubString(const char *pattern) const;
	//! tests if char * pattern is in this
	long   Contains(const char *pattern) const;
	//! computes the length of the maximum initial segment of the string that consists
	//! entirely of characters that are not in the charSet
	//! -1 indicates "failure" (no char matched), values >= 0 indicate the position of found char in String.
	long FirstCharOf(const String &charSet);
	//! Return position of first char in string beeing > highMark or -1 if no char matching to criterion found.
	//! Chars contained in the exclusion set are excluded from the check.
	long ContainsCharAbove(unsigned highMark = (unsigned) 127, const String excludeSet = "");
	//! computes the length of the maximum initial segment of the string that consists entirely
	//! of characters from the string pointed to by charSet
	//! -1 indicates the string contains no chars from charSet
	long LastCharOf(const String &charSet);
	//! tests if this starts with pattern
	bool StartsWith(const char *pattern) const;
	//! Replace first occurence of pattern with replacement
	//! \return whether a replacement took place
	bool Replace(const char *pattern, const char *replacement);
	//! find index of first occurance of character c after start
	//! \return index of c or -1 if not found
	long   StrChr(char c, long start = 0) const;
	//! find index of last occurance of character c
	//! \return return last index of c or -1 if not contained
	long   StrRChr(char c, long start = -1) const;
	//! return character at position ix, if ix is out of range return 0
	char At(long ix) const;
	//! return character at position ix, if ix is out of range return 0
	char operator[] (long ix) const;
	char operator[] (int ix) const;

	friend EXPORTDECL_FOUNDATION inline bool operator==(const String &s1, const String &s2);
	friend EXPORTDECL_FOUNDATION inline bool operator==(const char *s1, const String &s2);
	friend EXPORTDECL_FOUNDATION inline bool operator==(const String &s1, const char *s2);
	friend EXPORTDECL_FOUNDATION inline bool operator!=(const String &s1, const String &s2);
	friend EXPORTDECL_FOUNDATION inline bool operator!=(const char *s1, const String &s2);
	friend EXPORTDECL_FOUNDATION inline bool operator!=(const String &s1, const char *s2);
	friend EXPORTDECL_FOUNDATION inline bool operator<(const String &s1, const char *s2);
	friend EXPORTDECL_FOUNDATION inline bool operator>(const String &s1, const char *s2);

	//! convert all ascii uppercase characters into corresponding lowercase
	//! this is done inplace using ctype.h - classification isupper()
	//! \return *this after the conversion
	String &ToLower();
	//! convert all ascii lowercase characters into corresponding uppercase
	//! this is done inplace using ctype.h - classification islower()
	//! \return *this after the conversion
	String &ToUpper();
#ifdef __370__
	//! convert all characters from EBCDIC to ASCII
	//! \pre the character encoding is currently EBCDIC
	//! this is done inplace using our special functions
	//! do not know what happens to charachters not representable by ASCII
	void ToASCII();
	//! convert all characters from ASCII to EBCDIC
	//! \pre the character encoding is currently ASCII
	//! this is done inplace using our special functions
	//! do not know what happens to charachters not representable by EBCDIC
	void ToEBCDIC();
#endif

	//! debug operation, places String internals onto Syslog
	void Dump() const;

	//! compare two C strings without considering ascii letter case
	//! maps to strcasecmp on non NT machines
	//! null-pointer safe
	static long CaselessCompare(const char *s1, const char *s2);

	//! internal output routine with masking and embedding in quote characters
	//! to be used by AnyStringImpl
	//! example:
	//! String("Hello World\n").IntPrintOn(cerr)
	//! delivers
	//! "Hello World\x0A"
	//! on stdout
	ostream &IntPrintOn(ostream &os, const char quote = '\"') const;

	//! output routine to dump the string content as hexadecimal numbers in the form:
	//!
	ostream &DumpAsHex(ostream &os, long dumpwidth = 16L) const;

	//! internal input routine with masking and embedding in quote characters
	//! symmetric function to IntPrintOn
	//! if quote is '\0' reads up to the next white space character
	//! otherwise it assumes that quote is the first char on
	//! the stream. The previous content of the string object is deleted
	//! in any case
	//! \return the number of newline characters read for adjusting line count in parsing
	long IntReadFrom(istream &os, const char quote = '\"');
	//! canonical input operator for strings
	//! reads up to the next whitespace character
	//! use getline() function for reading lines
	friend EXPORTDECL_FOUNDATION istream &operator>>(istream &is, String &s);

// We don't use ostream directly because of the locking overhead. Because of
// a compiler weakness we have to undef ostream, so we can define operator<<
// for both types of streams.
// Otherwise operator<< wouldn't compile when using cerr, cerr or clog
#if defined(__SUNPRO_CC)&& !defined(__STD_OSTREAM__)
#undef ostream
	friend unsafe_ostream  &operator<<(unsafe_ostream &os, const String &s);
#endif

	//! canonical output operator for strings
	friend EXPORTDECL_FOUNDATION ostream  &operator<<(ostream &os, const String &s);

#if defined(__SUNPRO_CC) && !defined(__STD_OSTREAM__)
#define ostream unsafe_ostream
#endif

	//! function for reading strings from a stream up to a delimiter
	//! \param is istream read from
	//! \param s result of input is stored here
	//! \param c delimiting character, usually newline
	friend EXPORTDECL_FOUNDATION istream  &getline(istream &is, String &s, char c);

	//! manually set the allocator (should not usually be used...)
	bool SetAllocator(Allocator *a) ;
protected:

	enum Pilfer { STEAL }; // for syntactical difference of CTOR
//!  buffer stealing ctor to implement canonical operators like +
//!  see OOPSLA 98, Anthony J. H. Simmons: "Borrow, Copy or Steal? Loans and
//!     Larceny in the Orthodox Canonical From".
	String (String &, Pilfer);

	//! internal bottleneck routine for string allocation and content adjustment
	//! replace and reserve l characters of string s at position start
	//! if s is null or shorter than l just reserve memory for the rest
	//! if string gets longer allocates more memory
	//! \param start index position where new characters belong
	//! \param s pointer to new characters to be placed into string
	//! \param l number of characters to be taken from s and placed into this
	void Set(long start, const char *s, long l);

protected:

	//! struct that uses a space efficient trick to implement string
	struct StringImpl {
		//! the size of the allocated buffer
		long fCapacity;
		//! the length of the String as perceived by the user of this class
		long fLength;
		//! the allocated characters follow just after this so the start of the characters is (char *)(fStringImpl+1);
		char *Content() {
			return (char *)(this + 1);
		}
		//! the allocated characters follow just after this so the start of the characters is (char *)(fStringImpl+1);
		const char *Content() const {
			return (char *)(this + 1);
		}
	} *fStringImpl;

	Allocator *fAllocator;

	const String::StringImpl *GetImpl() const {
		return fStringImpl;
	}
	String::StringImpl *GetImpl() {
		return fStringImpl;
	}
	Allocator *GetAllocator() {
		return fAllocator;
	}
	const Allocator *GetAllocator() const {
		return fAllocator;
	}

	char *GetContent() {
		return GetImpl()->Content();   //  optimize, sanity already checked
	}
	const char *GetContent()const {
		return GetImpl()->Content();
	}

	void SetLength(long len) {
		fStringImpl->fLength = len;   // no sanity check!
	}
	void IncrementLength(long incr) {
		fStringImpl->fLength += incr;
	}

	friend class StringStreamBuf; // we directly operate on fCont, fCapacity, fLength
	friend class StringTest;
};

//---- StringTokenizer ---------------------------------------------------------
//! an efficient implementation of tokenization with one delimiter
class EXPORTDECL_FOUNDATION StringTokenizer
{
public:
	//! ctor takes content s and delimiting character
	StringTokenizer(const char *s, char delimiter);

	//! iteration
	//! \param token placeholder where the next token read is put
	//! \return true if there has been a token available, false if end of s is reached
	bool NextToken(String &token);
	String GetRemainder(bool boIncludeDelim = false);
	//! iteration, synonym for NextToken
	bool operator()(String &token);
	//! restart iteration
	//! may be a reset with a new delimiter is useful?
	void Reset() {
		fTokEnd = fString;
	}
protected:
	const char *fString;
	const char *fTokEnd;
	char fDelimiter;
	long fLength;
	friend class StringTokenizerTest;
};

//---- StringTokenizer ---------------------------------------------------------
//! more flexible but slower implementation of tokenization
//! allows several delimiting characters, instead of one
class EXPORTDECL_FOUNDATION StringTokenizer2
{
public:
	//! ctor, take content s and use whitespace (" \t\n") as delimiters
	StringTokenizer2(const char *s);
	//! ctor, take content s and all characters within the C string delimiters
	StringTokenizer2(const char *s, const char *delimiters);

	//! iteration
	//! \param token placeholder where the next token read is put
	//! \return true if there has been a token available, false if end of s is reached
	bool NextToken(String &token);

	String GetRemainder(bool boIncludeDelim = false);

	//! iteration, synonym for NextToken
	bool operator()(String &token);
	//! restart iteration
	//! may be a reset with a new delimiter set is useful?
	void Reset() {
		fPos = 0;
	}
protected:
	//! implements look ahead for tokens
	bool HasMoreTokens(long start, long &end);
	String fString, fDelimiters;
	long fPos;
	friend class StringTokenizer2Test;
};

//---- inlines -----------------------------------------------------------------

#define NotNull(s) ((s) ? (s) : "null")
#define NotNullStr(s) ((s.Length() > 0) ? (const char *)(s) : "null")

inline bool String::SetAllocator(Allocator *a)
{
	if (!fAllocator || !fStringImpl) {
		fAllocator = a;
		return true;
	}
	return false; // cannot change allocator later on
}

inline char String::operator[](long ix) const
{
	return At(ix);
}
inline char String::operator[](int ix) const
{
	return At(long(ix));
}
inline long String::Length() const
{
	if (GetImpl()) {
		return GetImpl()->fLength;
	} else {
		return 0;
	}
}
inline long String::Capacity() const
{
	if (GetImpl()) {
		return GetImpl()->fCapacity;
	} else {
		return 0;
	}
}
inline String::operator const char *() const
{
	if (GetImpl()) {
		return GetContent();
	} else {
		return "";
	}
}

inline bool String::IsEqual(const char *other) const
{
	return ! this->Compare(other);
}

inline bool String::IsEqual(const String &other) const
{
	return ((Length() == other.Length()) && ! this->Compare(other));
}
// this is just a performance shortcut

EXPORTDECL_FOUNDATION inline istream &getline(istream &is, String &s)
{
	return getline(is, s, '\n');
}
EXPORTDECL_FOUNDATION bool operator==(const String &s1, const String &s2)
{
	return s1.IsEqual(s2);
}
EXPORTDECL_FOUNDATION bool operator==(const char *s1, const String &s2)
{
	return s2.IsEqual(s1);
}
EXPORTDECL_FOUNDATION bool operator==(const String &s1, const char *s2)
{
	return s1.IsEqual(s2);
}
EXPORTDECL_FOUNDATION bool operator!=(const String &s1, const String &s2)
{
	return !s1.IsEqual(s2);
}
EXPORTDECL_FOUNDATION bool operator!=(const char *s1, const String &s2)
{
	return !s2.IsEqual(s1);
}
EXPORTDECL_FOUNDATION bool operator!=(const String &s1, const char *s2)
{
	return !s1.IsEqual(s2);
}
EXPORTDECL_FOUNDATION bool operator<(const String &s1, const char *s2)
{
	return (s1.Compare(s2) < 0);
}
EXPORTDECL_FOUNDATION bool operator>(const String &s1, const char *s2)
{
	return (s1.Compare(s2) > 0);
}

inline bool StringTokenizer::operator()(String &token)
{
	return NextToken(token);
}
inline bool StringTokenizer2::operator()(String &token)
{
	return NextToken(token);
}

#endif		//not defined _ITOString_H
