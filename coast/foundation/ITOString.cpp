/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface
#include "ITOString.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "StringStream.h"

//--- c-library modules used ---------------------------------------------------
#ifdef __370__
//	#include "a2ee2a.h"
#endif
#include <ctype.h>
#include <string.h>
#ifdef _AIX
#include <strings.h>
#endif

//---- String ----------------------------------------------------------------
// tunable parameters for string implementation
const long cStrAllocMinimum = 32; // minimum size of buffer, tuning param for optimizing
const long cStrAllocLimit = 4096; // cStrAllocMinimum < StrAllocLimit
const long cStrAllocIncrement =  1024;

void String::alloc(long capacity)
{
	// this method is extremly performance sensitive
	// beware of overhead
	// make initial capacity some power of 2 for saving allocs with short
	// strings, only if we have fixed size initializers string buffers
	// smaller than cStrAllocMinimum are created
	if ( capacity <= 0 ) {
		capacity = cStrAllocMinimum ;
	}
	capacity += sizeof(*fStringImpl); // add tara

	capacity = GetAllocator()->SizeHint(capacity);
	fStringImpl = (StringImpl *)	fAllocator->Calloc(capacity, sizeof( char ));

	if (!fStringImpl) { //--- allocation failed
		SysLog::Error("String::alloc: Memory allocation failed!");
	} else {
		// substract to get netto capacity again.
		fStringImpl->fCapacity = capacity - sizeof(*fStringImpl);
		// length is 0 by using Calloc
	}
}

long String::allocCapacity(long newLength)
{
// calculate capacity for optimal allocation
	// safe old params just in case allocation fails

	long newCapacity = 0;
	if (GetImpl()) {
		if ( newLength < cStrAllocLimit )	{
			newCapacity = newLength < cStrAllocMinimum ?
						  cStrAllocMinimum // initial size for very small buffers
						  : newLength * 2; // double capacity for small buffers
		} else {	// increment in fixed sizes with large buffers
			newCapacity = newLength + cStrAllocIncrement;
		} // if
	} else {
		newCapacity = newLength + 1; // exact size
	}
	return newCapacity;
}
String::String(Allocator *a)
	: fStringImpl(0),  fAllocator((a) ? a : Storage::Current())
{

}

String::String(long capacity, Allocator *a)
	: fStringImpl(0), fAllocator((a) ? a : Storage::Current())
{
	this->alloc(capacity);
}

String::String(const char *s, long l, Allocator *a)
	: fStringImpl(0), fAllocator((a) ? a : Storage::Current())
{
	if (s) {
		long sLen = strlen(s);
		Set(0, s, (sLen < l) ? sLen : l);
	}
}

String::String(void *s, long l, Allocator *a)
	: fStringImpl(0), fAllocator((a) ? a : Storage::Current())
{
	if (l > 0) { // should check l for sanity
		if (s) {
			Set(0, (const char *)s, l);
		} else {
			this->alloc(l);    // just reserve the space as with String(long)
		}
	}
	// no op --> empty string for insane parameters
}

String::String(const String &s, Allocator *a)
	: fStringImpl(0), fAllocator((a) ? a : s.fAllocator)
{
	// copies are always made within the same memory manager
	// transfers to different allocators occur only in operator=
	if ( s.GetImpl() ) {
		Set(0, s.GetContent(), s.Length());
	}
}

String &String::Append(istream &is, long length, char delim)
// reads at most length chars until delim
// semantics like istream.get(char *, len, delim)
{
	Set(Length(), 0, length);
	if (!(delim == is.peek())) {
		is.get(GetContent() + Length(), length, delim); // should do some error checking?
		long l = is.gcount(); // should be fLength

		Assert(l <= length);
		IncrementLength(l); // PT: always adjust length now
		GetContent()[Length()] = '\0';
	} // else nothing to read at all, must consume delim char elsewhere
//	fgTotal++;
	return *this;
}

String::~String()
{
	if (GetImpl()) {
		fAllocator->Free(GetImpl());
		fStringImpl = 0;
	}
}
// assignment operators are asymmetric !!!
String &String::operator= (const char *s)
{
	Set(0, s, -1);
	return *this;
}

String &String::operator= (const String &s)
{
	// string keeps its allocator for its whole life...
	if (s.GetImpl()) {
		Set(0, s.GetContent(), s.Length());
	} else if (GetImpl()) {
		Set(0, 0, 0); // make it empty
	}// if we are already empty do a nop
	return *this;
}

void String::Set(long start, const char *s, long len)
/* in: start: at this position we start to copy the contents of s
		   s: source from where to copy, may be 0
		 len: number of bytes to copy
			  (or intention to copy afterwards if s=0)
changes: fCont, fCapacity (if necessary) and fLength
 what: len bytes from s are copied after position start of fCont,
	   fLength and fCapacity are adjusted accordingly.
	   If necessary the old buffer is copied to a new location where enough
	   memory is available.
change: PT: string terminated at start if s==0 (no bytes copied anyways),
			in this case only the capacity is adjusted
note: if start > fLength then the new buffer will contain undefined
	  memory between oldLength and start!

*/
{
	//char *oldBuf=0;
	StringImpl *oldImpl = 0;
	// to store old buffer if new memory has to be allocated

	// check for start integrity
	if ( start < 0 ) {
		start = 0;
	}

	// check for length integrity: calculate length to add from s
	// if length is not provided
	if (len < 0) {
		len = s ? strlen(s) : 0;
	}
	// maybe <= is appropriate here? nothing happens for len=0

	// calculate new string length
	long newLength = start + len;

	// expand string if necessary
	if ( (newLength > 0) && (newLength >= Capacity() ) ) {
		oldImpl = GetImpl();
		this->alloc(allocCapacity(newLength));
		if (GetImpl()) {
			if (oldImpl) {
				// copy the part of the old string which is not overwritten afterwards
				long oldLength = oldImpl->fLength;
				long tocopy = (start > oldLength) ? oldLength : start;
				if (tocopy) {
					memcpy(GetContent(), oldImpl->Content(), tocopy);    //PS to be tested
				}
			}
		} else {
			static const char crashmsg[] = "FATAL: String::Set allocation failed. I will crash :-(\n";
			SysLog::WriteToStderr(crashmsg, sizeof crashmsg);
			// restore string
			fStringImpl = oldImpl;
			return;
			// no new memory allocated, oldImpl must not be freed
		}
	}
	Assert((newLength == 0) || GetImpl());
	// add s to fCont after position start
	if (GetImpl()) {
		if (s ) {
			memcpy(GetContent() + start, s, len);
			SetLength(newLength);
		} else {
			SetLength(start);	// PT: if nothing is copied the string is terminated at start
		}
		GetContent()[Length()] = 0;
	}

	if (oldImpl) {	// don't forget to free old buffer if new memory was allocated
		fAllocator->Free(oldImpl);
	}
}

String &String::Append(char c)
{
	if (Length() + 2 > Capacity()) {
		Set(Length(), &c, 1);    // PT: use the generic copying here
	} else { // optimize for inline expansion, Set allocates additional stuff
		GetContent()[Length()] = c;
		IncrementLength(1);
		GetContent()[Length()] = 0;
	}
	return *this;
}
String &String::Append(const char *s, long len)
{
	if (s) {
		long sLen = strlen(s);
		Set(Length(), s, (sLen < len) ? sLen : len);
	}
	return *this;
}
String &String::Append(const void *s, long len)
{
	if (s && len > 0) {
		Set(Length(), (const char *)s, len);
	}
	return *this;
}
// PS: implement similar of IStream!
String &String::Append(istream &is, long length)
{
	if (length > 0) {
		Set(Length(), 0, length); // adjust capacity // PT: now just capacity
		is.read(GetContent() + Length(), length); // should do some error checking?

		long l = is.gcount(); // should be fLength
		//	if (l < length)
		//		; // short read
		//	else if (l > length)
		//		; // might be a big problem....read more than requested
		Assert(l <= length);
		// PT: with the new Set semantics for s=0 fLength must be set now
		IncrementLength(l);
		GetContent()[Length()] = '\0';
	} // else no-op
	return *this;
}

String &String::Append(const String &s)
{
	Set(Length(), s.GetContent(), s.Length());// FIXME provide special Set..
	return *this;
}

void String::Dump() const
{
	String logMsg;
	SysLog::Info(logMsg.Append("String::Dump: length ").Append(Length()).Append(", capacity ").Append(Capacity())  );
}

String String::DumpAsHex(long dumpwidth, const char *pcENDL) const
{
	String strResult;
	if (Length() > 0) {
		static String hexcode("0123456789ABCDEF", -1, Storage::Global());
		String outbuf;
		long lTotalLen = (4L * dumpwidth + 1L);
		long x = 0L;
		for (long l = 0, sz = Length(); l < sz; ++l, ++x) {
			if (l % dumpwidth == 0) {
				x = 0L;
				if (l > 0) {
					strResult.Append(outbuf).Append(pcENDL);
				}
				// fill/clear the whole string with spaces
				for (long q = 0; q < lTotalLen; ++q) {
					outbuf.PutAt(q, ' ');
				}
			}
			unsigned char c = At(l);
			// first fill hexnumber of character
			outbuf.PutAt(x * 3L,    hexcode[ (long)((c >> 4) & 0x0f) ]);
			outbuf.PutAt(x * 3L + 1L, hexcode[ (long)(c & 0x0f) ]);
			if ( !isprint((unsigned char) c) ) {
				// print a dot for unprintable characters
				c = '.';
			}
			outbuf.PutAt(3L * dumpwidth + 1L + x, c);
		}
		strResult.Append(outbuf);
	}
	return strResult;
}

ostream &String::DumpAsHex(ostream &os, long dumpwidth) const
{
	if (Length() > 0) {
		String strBuf = DumpAsHex(dumpwidth), strToken;
		StringTokenizer aTok(strBuf, '\n');
		while ( aTok.NextToken(strToken) ) {
			os << strToken << endl;
		}
		os << flush;
	}
	return os;
}

String &String::Append(long number)
{
	OStringStream obuf(this, ios::app);
	obuf << number;
	return *this;
}

#ifndef __370__
String &String::Append(l_long number)
{
	OStringStream obuf(this, ios::app);
#if defined(WIN32)
	obuf << (long)number;
#else
	obuf << number;
#endif
	return *this;
}
#endif

#if defined(WIN32)
String &String::Append(u_long number)
{
	OStringStream obuf(this, ios::app);
	obuf << number;
	return *this;
}
#endif

String &String::Append(double number)
{
	OStringStream obuf(this, ios::app);
	obuf << number;
	return *this;
}

char String::At(long ix) const
{
	if (ix >= 0 && ix < Length()) {
		return GetContent()[ix];
	}
	return 0; //PS: return value should be 0 for string loops to work
}

String String::SubString(long from, long len) const
{
	long l = Length();

	if (from >= l) {
		return String();
	}

	if (from < 0) {
		from = 0;
		SysLog::Error("String::GetSub: from < 0");
	}
	if ( (len < 0) || (from + len > l) ) {
		len = l - from;
	}

	if (GetContent()) {
		return String((void *)(GetContent() + from), len, const_cast<String *>(this)->GetAllocator());
	}
	return String(const_cast<String *>(this)->GetAllocator());
}

// finds first occurence of pattern in String
String String::SubString(const char *pattern) const
{
	if ( pattern ) {
		if ( GetImpl() ) {
			const char *at = strstr( GetContent(), pattern );
			if ( at ) {
				return String( at, Length() - (at - GetContent()), const_cast<String *>(this)->GetAllocator() );
			}
		}
	}
	return String(const_cast<String *>(this)->GetAllocator());
}

// finds first occurence of pattern in String
long String::Contains(const char *pattern) const
{
	long result = -1;
	if ( pattern ) {
		if ( GetImpl() ) {
			const char *at = strstr( GetContent(), pattern );
			if ( at )	{
				result = (at - GetContent());
			}
		}
	}
	return result;
}

long String::FirstCharOf(const String &charSet)
{
	size_t firstchar = strcspn((const char *)(*this), (const char *)charSet);
	if ((long)firstchar >= Length()) {
		// not found
		return -1;
	}
	return firstchar;
}

long String::LastCharOf(const String &charSet)
{
	size_t firstchar = strspn((const char *)(*this), (const char *)charSet);
	if ( firstchar == 0 ) {
		// no match at all
		return -1;
	}
	return firstchar;
}

bool String::StartsWith(const char *pattern) const
{
	if ( pattern ) {
		if ( GetImpl() ) {
			for (const char *content = GetContent(); ; ++content, ++pattern) {
				if (!(*pattern)) {
					return true;
				} else if (!(*content)) {
					return false;
				} else if (*content != *pattern) {
					return false;
				}
			}
		}
		return !(*pattern);
	}
	return false;
}

bool String::Replace(const char *pattern, const char *replacement)
{
	long pos = Contains(pattern);
	if (pos < 0) {
		return false;
	}
	String tmp(SubString(pos + strlen(pattern)));
	Trim(pos);
	Append(replacement);
	Append(tmp);
	return true;
}

// CR #10 a more robust implementation of StrChar
long String::StrChr(char c, long start) const
{
	if ( start < 0 ) {
		start = 0;
	}
	// returns index of c or - 1 if not found
	long result = -1;
	if ( GetImpl() && start <= Length() ) {
		char *res = (char *)memchr(GetContent() + start, c, Length() - start);
		if (res) {
			result = (res - GetContent());
		}
	}
	return result;
}

long String::StrRChr(char c, long start) const
{
	if ( start < 0 ) {
		start = Length();
	}
	// returns index of c or - 1 if not found
	long result = -1;
	if ( GetImpl() && start <= Length() ) {
		const char *res = GetContent() + start - 1 ; // this is not functional for binary strings!! strrchr(GetContent(), c);
		while ( ( res >= GetContent() ) && ( (*res) != c ) ) {
			// PT: some really quick thinking of mine: it only took me 10 minutes
			//     to figure out it should say >= above ... (guess I leave soon)
			--res;
		}
		if (res >= GetContent()) {
			result = res - GetContent();
		}
	}
	return result;
}

void String::TrimFront(long newstart)
{
	if ( newstart >= 0 && newstart < Length()) {
		Set(0, GetContent() + newstart, Length() - newstart);
	} else if (newstart >= Length()) {
		if (GetImpl()) {
			SetLength(0);
			*GetContent() = 0;
		}
	}

}

void String::Trim(long newlen)
{
	if ( newlen <= Length() ) {
		Set( ( newlen > 0 ) ? newlen : 0, 0, 0);
	}
}

void String::Reserve(long minreserve)
/* in: minreserve: minimal reserve (GetCapacity()-fLength) after this operation
 what: adjusts capacity such that at least minreserve bytes are available
*/
{
	Set(Length(), 0, ( minreserve > 0 ) ? minreserve : 0);
}

int String::Compare(const char *other) const
{
	if (GetImpl() && other) {
		return strcmp(GetContent(), other);
	} else if ((GetImpl() && *GetContent()) && ! other ) {
		return 1;
	} else if (! GetImpl() && (other && *other) ) {
		return -1;
	} else {
		return 0;    // both are empty
	}
}

int String::Compare(const String &other) const
{
	if (GetImpl()) {		// (const char *)other never returns 0!
		long otherLength = other.Length();
		long len = (Length() < otherLength) ? Length() : otherLength;

		long res = memcmp(GetContent(), (const char *)other, len);
		if (res != 0) {
			return res;
		} else {
			if (Length() == otherLength) {
				return 0;
			} else if (Length() < otherLength) {
				return -1;
			} else {
				return 1;
			}
		} // if
	} else {
		return (Length() < other.Length()) ? -1 : 0;
	}
} // Compare

int  String::CompareN(const char *other, long length, long start) const
{

	if ( start >=  Length() ) {
		start =  Length() - 1;    // length should be 0 if !GetContent()
	}
	if (start < 0) {
		start = 0;    // PS this is required for robustness.
	}
	if (!GetImpl() || length <= 0) {	// nothing to compare means equal!
		// empty string always smaller, except for empty
		if (length > 0 && other && *other) {
			return -1;
		}
		return 0;
	}
	const char *contPtr = GetContent() + start;
	if (contPtr && other) {
		return strncmp(contPtr, other, length);
	} else if ((contPtr && *contPtr) && ! other ) {
		return 1;
	} else if (! contPtr && (other && *other) ) {
		return -1;    // PS there was an error here!
	} else {
		return 0;    // both are empty
	}
}

long String::CopyTo(char *buf, long n, long pos) const
{

	if (pos < 0) {
		pos = 0;    // must not copy from outside
	}

	long  rest = Length() - pos; // do not copy too much

	if (n > rest) {
		n = rest;
	}
	if (n > 0 ) {
		memcpy(buf, GetContent() + pos, n);
	}

	return n;
}

void String::PutAt( long pos, char c)
{
	if (pos >= 0  ) {
		if (GetImpl() && pos < Length()) {
			GetContent()[pos] = c; // just exchange char
		} else {
			Set(pos, &c, 1); // adjust string as needed
		}
	}
}

void String::ReplaceAt( long pos, const char *s, long len)
{
	if (s && pos >= 0 && len > 0) { // parameter sanity check, prevent no-op
		if (len > (Length() - pos)) { // new string will be longer
			Set(pos, s, len);
		} else { // replacement within the string buffer
			// use memcpy in this case, since "Set" terminates
			// the string right after the replaced buffer
			memcpy(GetContent() + pos, s, len);
		}
	}
}

/* in: s1, s2: C-Strings to compare
 rets: -1 if s1 is caselessly lexicographically smaller than s2
		0 if s1 is caselessly equal to s2
		1 if s1 is caselessly lexicographically larger than s2
 what: checks if both strings are different from 0 and calls the C-function
 note: only works for null terminated C-Strings, everything after the first
	   \000 byte in either of the strings is ignored (especially notable for
	   applications involving the String class)
*/
long String::CaselessCompare(const char *s1, const char *s2)

{
	if ((0 != s1) && ( 0 != s2)) {
#if !defined(__370__) && !defined(WIN32)
		return strcasecmp(s1, s2);
#else
		{
			// atraxis - edwin
			String string1 = s1;             // atraxis - edwin
			String string2 = s2;             // atraxis - edwin
			string1.ToUpper();               // atraxis - edwin
			string2.ToUpper();               // atraxis - edwin
			return(strcmp(string1, string2)); // atraxis - edwin
		}                                  // atraxis - edwin

# endif

	} else if (s1 != 0) {
		return 1;    // only s1 is there
	} else if (s2 != 0) {
		return -1;    // only s2 is there
	} else {
		return 0;    // both are 0
	}
} // CaselessCompare

long String::ContainsCharAbove(unsigned highMark, const String excludeSet)
{
	long ret = -1L;
	if (highMark > 255) {
		return 0;
	}
	long excludeSetLength = excludeSet.Length();
	for ( long i = 0, sz = Length(); i < sz; ++i) {
		unsigned char c = At(i);
		if ( c > highMark ) {
			if ( excludeSetLength == 0L ) {
				return i;
			} else {
				bool found = false;
				for ( long ii = 0; ii < excludeSetLength; ++ii) {
					if ( found = ((unsigned char) excludeSet.At(ii) == c) ) {
						break;
					}
				}
				if ( !found ) {
					return i;
				}
			}
		}
	}
	return ret;
}

String &String::ToLower()
{
	if (GetImpl()) {
		char *s = GetContent();
		while ( *s ) {
			*s = tolower(*s);
			++s;
		}
	}
	return *this;
}

String &String::ToUpper()
{
	if (GetImpl()) {
		char *s = GetContent();
		while ( *s ) {
			*s = toupper(*s);
			++s;
		}
	}
	return *this;
}

#ifdef __370__
void String::ToEBCDIC()
{
	if (GetImpl())
		ascii2ebcdic((void *)(const char *)GetContent(), (const void *)(const char *)GetContent(),
					 Length());
} // ToEBCDIC

void String::ToASCII()
{
	if (GetImpl())
		ebcdic2ascii((void *)(const char *)GetContent(), (const void *)(const char *)GetContent(),
					 Length());
} // ToASCII
#endif

// CR #16
// internal IO routines with masking and embedding in quote characters
// to be used by AnyStringImpl

// String("Hello World\n").IntPrintOn(cerr)
// delivers
// "Hello World\x0A"
// on stdout
ostream &String::IntPrintOn(ostream &os, const char quote) const
{
	if (quote) {
		os.put(quote);
		// iterate the string and look for characters to mask
		unsigned char c = '\0';
		for ( long i = 0, sz = Length(); i < sz; ++i) {
			c = At(i);
			if ( !isprint(c) ) { // http server doesn't like / unencoded in path expressions
				// use hex \0x20 rep: PS' simple converter
				os.put('\\');
				if (c == '\n') {
					// use special char for nicer look
					os.put('n');
				} else {
					// use hex encoding
					os.put('x');
					os.put("0123456789ABCDEF"[ (c >> 4) & 0x0f]);
					os.put("0123456789ABCDEF"[ c  & 0x0f]);
				}
			} else {
				// mask also quote and mask character
				if (c == quote || c == '\\') {
					os.put('\\');
				}
				os.put(c);
			}
		}
		os.put(quote);
	} else {
		// no quoting, keep fingers crossed that no special chars are
		// within the string value
		os << (const char *)*this;
	}
	return os;
}
// symmetric function to IntPrintOn
// if quote is '\0' reads up to the next white space character
// otherwise it assumes that quote is the first char on
// the stream. The previous content of the string object is deleted
// in any case
// returns the number of newlines if any are read by this method
// PS: post CR:
// unmasked newline characters also terminate a string
// check if this "feature" has been used by projects?
long String::IntReadFrom(istream &is, const char quote)
{
	long newlinecounter = 0;
	if (GetImpl()) {
		*GetContent() = 0;
		SetLength(0);
	}
	if (quote) {
		char c = 0;

		// read quote character
		is.get(c);

		if (!is || !is.good()) {
			// fail gracefully
			if (c != quote) {
				is.clear(); // setf(0); should reset state to good()
				is.putback(c); // put it into the string
			}
		}
		// now read up to quote character or EOF or EOL (PS)
		// now behaves more nicely in case of \r\n combinations
		// the combination \n\r is not treated as new-line
		while ( (!(!is) && is.good()) ) {
			c = 0; // PS: post CR for additional safety at EOF, might duplicate c
			is.get(c);
			if (c == quote || ! is.good()) {
				break;
			}

			if ( c == '\\') {
				// special cases of masked characters
				is.get(c);
				if (c == '\\' || c == quote) {
					// '\\' or '\"'
					this->Append(c);
				} else if (c == 'x') {
					// hex char e.g. '\xAB?
					char h[2];
					is.get(h[0]);
					if (isxdigit( (unsigned char) h[0])) {
						is.get(h[1]);
						if (! isxdigit( (unsigned char) h[1])) {
							// a single xdigit n pass 0n to it.
							is.putback(h[1]);
							h[1] = h[0];
							h[0] = '0';
						}
						this->AppendTwoHexAsChar(h);
					} else {
						// no hex char follows \x assume its a literal
						this->Append("\\x");
						is.putback(h[0]); // PS: we forgot the putback
					}
				} else if (c >= '0' && c <= '7') {
					// allow for octal read up to two more
					char val = c - '0';
					is.get(c);
					if ( c >= '0' && c <= '7' ) {
						val = val * 8 + (c - '0');
						is.get(c);
						if ( c >= '0' && c <= '7' ) {
							val = val * 8 + (c - '0');
						} else { // premature end of ocal code
							is.putback(c);
						}
					} else {
						// premature end of ocal code
						is.putback(c);
					}
					this->Append(val);
				} else if (c == 'n') {
					// translate \n coding
					this->Append('\n');
				} else if (c == 'r') {
					// translate \r coding
					this->Append('\r');
				}
				// PS: post CR for safe \r\n handling in strings
				else if ('\r' == c) {
					// need to check for masked \r\n sequences
					++newlinecounter;
					is.get(c);
					if ('\n' != c) {
						is.putback(c);    // do not ignore it.
					}
				} else if (c == '\n') {
					// a masked newline is ignored, take it as a continuation line
					++newlinecounter;
				} else {
					// unknown stuff take it literally
					this->Append('\\');
					this->Append(c);
				}
			} else {
				// we shouldn't accept unmasked newline characters
				// make it terminating the string
				// this allows us to track config file errors stating unmatched quotes
				// PS: post CR
				// may be we should log an error here.
				if ('\r' == c) {
					// need to check for \r\n sequences
					is.get(c);
					if ('\n' != c) {
						is.putback(c);
						c = '\n';
					}
					// ensure next if becomes true, accept \r as line separator
				}
				if (c == '\n') {
					// should break strings with newlines, but feature already used
					// by WebBanking project. Should state implications
					// to the project.
					// do a nasty trick, return a negative number to
					// show this syntactical error
					// sorry there is no error message (yet)
					//SysLog::Warning("String::IntReadFrom: unexpected end of line (missing quote?)");
					++newlinecounter;
					return 0 - newlinecounter;
				}
				this->Append(c);
			}
		} // while
	} else {
		// unquoted string read to next whitespace
		is >> *this;
	}
	return newlinecounter;
}

String &String::AppendAsHex(unsigned char cc)
{
	char hexDigits[2];

	hexDigits[0] = "0123456789ABCDEF"[(cc >> 4) & 0x0f];
	hexDigits[1] = "0123456789ABCDEF"[ cc       & 0x0f];

	// Force String::Append(void *,	long);
	return Append((void *) hexDigits, 2L);
}

String &String::AppendAsHex(const unsigned char *cc, long len, char delimiter)
{
	for (long i = 0; i < len; ++i) {
		AppendAsHex(cc[i]);
		if ( (delimiter != '\0') && (i + 1 < len) ) {
			Append(delimiter);
		}
	}
	return *this;
}

/*
 * the string starts with two hex characters.
 * append a char to the string formed from them.
 * return this
 */
String &String::AppendTwoHexAsChar(const char *p)
{
	char high = 0, low = 0;
	if (p && isxdigit( (unsigned char) p[0]) && isxdigit( (unsigned char) p[1])) {
		high = p[0];
		// PS: streamline comparison post CR
		if (isdigit( (unsigned char) high)) { // if ('0' <= high && high <= '9')
			high -= '0';
		} else if ('a' <= high) { // if ('a' <= high&& high <= 'f')  // assume 'A' < 'a'
			high -= ('a' - 10);
		} else  {//if ('A' <= high && high <= 'F')
			high -= ('A' - 10);
		}
		low = p[1];
		if (isdigit( (unsigned char) low)) { // if ('0' <= low && low <= '9')
			low -= '0';
		} else if ('a' <= low) { // if ('a' <= low&& low <= 'f')  // assume 'A' < 'a'
			low -= ('a' - 10);
		} else  {//if ('A' <= low && low <= 'F')
			low -= ('A' - 10);
		}

	}
	return Append(char(16 * high + low));
}

String &String::AppendTwoHexAsChar(const char *cc, long len, bool delimiter)
{
	const char steps = (delimiter ? 3 : 2);

	for (long i = 0; i < (len * steps); i += steps) {
		AppendTwoHexAsChar(cc + i);
	}
	return *this;
}

long String::AsLong(long dflt)
{
	if (this->Length()) {
		IStringStream is(this);
		is >> dec; // sets decimal
		is >> dflt; // if it fails because of format error dflt remains unchanged
	}
	return dflt;
}

l_long String::AsLongLong(l_long dflt)
{
	if (this->Length()) {
		IStringStream is(this);
		is >> dec; // sets decimal
		is >> dflt; // if it fails because of format error dflt remains unchanged
	}
	return dflt;
}

double String::AsDouble(double dflt)
{
	if (this->Length()) {
		IStringStream is(this);
		is >> dec; // sets decimal
		is >> dflt; // if it fails because of format error dflt remains unchanged
	}
	return dflt;
}

// PS: try OOPSLA 98 canonical form operations with optimal buffer stealing
String::String (String &subject, Pilfer)
	: fStringImpl(subject.GetImpl()), fAllocator(subject.GetAllocator())
{
	// stealing CTOR
	fStringImpl = subject.GetImpl();
	subject.fStringImpl = 0;
}

String String::Add(const String &s) const
{
	String result(Length() + s.Length() + 1); // no reallocation needed
	result.Append(*this);
	result.Append(s);
	return String(result, STEAL); // no additional buffer copying
}

//---- StringTokenizer ---------------------------------------------------------
StringTokenizer::StringTokenizer(const char *s, char delimiter)
	: fString(s)
	, fTokEnd(fString)
	, fDelimiter(delimiter)
	, fLength((s) ? (long)strlen(s) : -1L)
{
}

bool StringTokenizer::NextToken(String &token)
{
	// fPos marks the current position within the original string

	if ( (fLength > 0) && (fTokEnd <= fString + fLength) ) {
		// still got something to work with
		const char *tokStart = fTokEnd;
		while (*(fTokEnd) != '\0' && *fTokEnd != fDelimiter) {
			++fTokEnd;
		}

		if (*(fTokEnd) == fDelimiter) {
			// found delimiter
			token = String(tokStart, (fTokEnd - tokStart));
			++fTokEnd;		// start of next token
		} else {
			// no delimiter... whole string is the token
			token = String(tokStart);
			++fTokEnd;
		}
		return true;
	} else {
		// the string is empty or the whole string has been tokenized
		// there are no tokens left
		return false;
	}
}

String StringTokenizer::GetRemainder(bool boIncludeDelim)
{
	if (boIncludeDelim) {
		const char *tokStart = fTokEnd;
		if (tokStart > fString) {
			--tokStart;
		}
		return String(tokStart);
	} else {
		if (*(fTokEnd) != '\0' && (fTokEnd <= fString + fLength)) {
			return String(fTokEnd);
		}
		return String();
	}
}

//---- StringTokenizer2 ---------------------------------------------------------
StringTokenizer2::StringTokenizer2(const char *s)
	: fString(s), fDelimiters(" \t\n"), fPos(0)
{
}

StringTokenizer2::StringTokenizer2(const char *s, const char *delimiters)
	: fString(s), fDelimiters(delimiters), fPos(0)
{
}

bool StringTokenizer2::NextToken(String &token)
{
	long start = fPos;
	long end = fPos;
	if (HasMoreTokens(start, end)) {
		token = fString.SubString(start, end - start);
		fPos = end + 1;
		return true;
	}
	return false;
}

String StringTokenizer2::GetRemainder(bool boIncludeDelim)
{
	long start = fPos;
	if (boIncludeDelim) {
		if (start > 0) {
			--start;
		}
		return fString.SubString(start);
	} else {
		if (start < fString.Length()) {
			return fString.SubString(start);
		}
		return String();
	}
}

bool StringTokenizer2::HasMoreTokens(long start, long &end)
{
	const char *delims = (const char *)fDelimiters;
	long l = fString.Length();
	while (end < l && strchr(delims, fString[end]) == 0) {
		++end;
	}
	return start < l;
}

istream &operator>>(istream &is, String &s)
{
	int aChar;

#if !(defined(__SUNPRO_CC) && (__SUNPRO_CC > 0x4ff))

//	is.ipfx(1);         //  see ANSI draft rev. 5
#endif
	s.Set(0, 0, cStrAllocMinimum);		// empty string reserve cStrAllocMinimum chars, tunable param
	if (is.good() && s.GetImpl()) { // sanity checks
		is >> ws;	// skips whitespace
		//if(is) {

		while ((aChar = is.get()) != EOF) {
			if (isspace( (unsigned char) aChar)) {
				is.putback(char(aChar));
				break;
			}
			if (s.Length() + 2 > s.Capacity()) {
				char c = (char)aChar;
				s.Set(s.Length(), &c, 1);	// auto-expand
			} else { // optimize for inline expansion, Set allocates additional stuff
				s.GetContent()[s.Length()] = (char)aChar;
				s.IncrementLength(1);
			}
		}
		s.GetContent()[s.Length()] = '\0';		// add 0 byte for termination
		//}
	}
	if (is.eof() && s.Length() != 0) {
		is.clear();
	}

//#if defined(__SC__) || defined(__linux__)
//	// do nothing; call below produces segmentation fault on linux
//#else
//#if !(defined(__SUNPRO_CC) && (__SUNPRO_CC > 0x4ff))
//	is.isfx();
//#endif

	return is;
}

istream &getline(istream &is, String &s, char c)
{
	char aChar = 0;

	s.Trim(0);
	for (;;) {

		// make sure, that we have at least 120 free bytes
		long freespace = s.Capacity() - s.Length() - 1;

		if (freespace < 120) {
			s.Reserve(128);
			freespace = s.Capacity() - s.Length() - 1;
		}

		is.get(s.GetContent() + s.Length(), freespace, c);
		if (!is) {
			// This is for the guys from Watcom, who
			// love to set up their own standards.
			// Watcoms iostream sets the fail bit,
			// when an empty line is read.
			if (is.rdstate() & ios::failbit) {
				is.clear();
			} else {
				break;    // eof or bad
			}
		} else {
			// set the length explicitly
			s.IncrementLength(is.gcount());
			s.GetContent()[s.Length()] = '\0';
		}
		is.get(aChar);
		if (is.eof() || !is.good() || aChar == c) {
			break;
		}
		s.Append(aChar);
	}
	if (is.eof() && s.Length() != 0) {
		is.clear();
	}
	return is;
}

// CR #8
// We don't use ostream directly because of the locking overhead. Because of
// a compiler weakness we have to undef ostream, so we can define operator<<
// for both types of streams.
// Otherwise operator<< wouldn't compile when using cout, cerr or clog
#if defined(__SUNPRO_CC)&& !defined(__STD_OSTREAM__)
#undef ostream
unsafe_ostream &operator<<(unsafe_ostream &os, const String &s)
{

	if (!os.opfx()) { // see ANSI draft rev. 5
		return os;
	}

	size_t len = s.Length();
	size_t width = os.width();
	int left = ((os.flags() & ios::left) != 0);

	if (left) {
		os.write((const char *)s, len);    // AB: use cast to apply operator const char *
	}

	if (width && width > len) {
		size_t padlen = width - len;
		char c = os.fill();

		while (--padlen >= 0) {
			os.put(c);
		}
		os.width(0); // the iostream documentation states this behaviour
	}
	if (!left) {
		os.write((const char *)s, len);    // AB: use cast to apply operator const char *
	}

	os.osfx();

	return os;
}
#endif

ostream &operator<<(ostream &os, const String &s)
{
//#if !(defined(__SUNPRO_CC) && (__SUNPRO_CC > 0x4ff))
//	if (!os.opfx())  // see ANSI draft rev. 5
//		return os;
//#endif
	size_t len = s.Length();
	size_t width = os.width();
	int left = ((os.flags() & ios::left) != 0);

	if (left) {
		os.write((const char *)s, len);    // AB: use cast to apply operator const char *
	}

	if (width && width > len) {
		size_t padlen = width - len;
		char c = os.fill();

		while ( --padlen >= 0 ) {
			os.put(c);
		}
		os.width(0); // the iostream documentation states this behaviour
	}
	if (!left) {
		os.write((const char *)s, len);    // AB: use cast to apply operator const char *
	}

//#if !(defined(__SUNPRO_CC) && (__SUNPRO_CC > 0x4ff))
//	os.osfx();
//#endif
	return os;
}

#if defined(__SUNPRO_CC) && !defined(__STD_OSTREAM__)
#define ostream unsafe_ostream
#endif
