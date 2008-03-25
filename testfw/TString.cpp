/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "TString.h"

//--- c-library modules used ---------------------------------------------------
#include <cstdlib>

#if defined(ONLY_STD_IOSTREAM)
#include <sstream>
#include <iostream>
using namespace std;
#else
#if !defined(WIN32)
#include <strstream.h>
#else
#include <iostream.h>
#endif
#include <stdio.h>
#include <ctype.h>
#endif

#if defined(__370__) || defined(_ARCH_COM)
#include <stdlib.h>
#else
#if !defined(ONLY_STD_IOSTREAM)
#include <malloc.h>
#endif
#endif

//---- TString ----------------------------------------------------------------
const long cStrAllocLimit = 4096;
const long cStrAllocIncrement =  1024;

void TString::alloc(long capacity)
{
	// this method is extremly performance sensitive
	// beware of overhead
	fCapacity = ( capacity <= 0 ) ? 1 : capacity;

	fCont = (char *) calloc(fCapacity, sizeof( char ));
	if (!fCont) {
		//--- allocation failed
		cerr << "FATAL: TString::alloc calloc failed. I will crash :-(" << endl << flush;
	}
}

TString::TString()
	: fCont(0), fCapacity(0), fLength(0)
{
}

TString::TString(long capacity)
	: fCont(0), fCapacity(capacity), fLength(0)
{
	this->alloc(capacity);
}

TString::TString(const char *s, long l)
	: fCont(0), fCapacity(0), fLength(0)
{
	if (s) {
		Set(0, s, l);
	}
}

TString::TString(const TString &s)
	: fCont(0), fCapacity(0), fLength(0)
{
	if ( s.fCont ) {
		Set(0, s.fCont, s.Length());
	}
}

TString::~TString()
{
	if (fCont) {
		free(fCont);
		fCont = 0;
		fLength = fCapacity = 0;
	}
}

// assignment operators ar asymmetric !!!
TString &TString::operator= (const char *s)
{
	Set(0, s, -1);
	return *this;
}

TString &TString::operator= (const TString &s)
{
	Set(0, s.fCont, s.Length());
	return *this;
}

void TString::Set(long start, const char *s, long len)
{
	// safe old length just in case...
	long oldLength(fLength);

	// check for start integrity
	if ( start < 0 ) {
		start = 0;
	}

	// check for length integrity
	// calculate length to add from s
	// if length is not provided
	if (len < 0) {
		len = s ? strlen(s) : 0;
	}

	// calculate new TString length
	fLength = start + len;

	// expand TString if necessary
	if ( (fLength > 0) && (fLength + 1) > fCapacity ) {

		// safe old params just in case allocation fails
		long oldCapacity(fCapacity);
		long newCapacity = (fLength + 1) * 2;	// double needed length for small buffers

		if ( fLength >= cStrAllocLimit ) {	// increment in fixed sizes with large buffers
			newCapacity = fLength + cStrAllocIncrement;
		}

		char *oldBuf = fCont;
		this->alloc(newCapacity);
		if (fCont) {
			if (oldBuf) {
				memcpy(fCont, oldBuf, oldLength);
				free(oldBuf);
			}
		} else {
//			SysLog::Error("Memory allocation failed (calloc, TString::Set)");
			// restore TString
			fCapacity = oldCapacity;
			fLength = oldLength;
			fCont = oldBuf;
			return;
		}
	}
	// add TString to fCont
	if (s && fCont) {
		memcpy(&fCont[start], s, len);
	}

	if (fCont) {
		// terminate TString properly
		fCont[fLength] = 0;
	}
}

TString &TString::Append(char c)
{
	Set(fLength, 0, 1);
	fCont[fLength-1] = c;
	return *this;
}

TString &TString::Append(const char *s, long len)
{
	Set(fLength, s, len);
	return *this;
}

TString &TString::Append(const TString &s)
{
	Set(fLength, s.fCont, s.Length());
	return *this;
}

TString &TString::Append(int number)
{
	char str[100] = {0};
	snprintf(str, sizeof(str), "%d", number );
	Append( str, (long)strlen(str) );  // Append(const char *s, long len)
	return *this;
}

TString &TString::Append(long number)
{
	char str[100] = {0};
	snprintf(str, sizeof(str), "%ld", number );
	Append( str, (long)strlen(str) );  // Append(const char *s, long len)
	return *this;
}

TString &TString::Append(unsigned long number)
{
	char str[100] = {0};
	snprintf(str, sizeof(str), "%lu", number );
	Append( str, (long)strlen(str) );  // Append(const char *s, long len)
	return *this;
}

TString &TString::Append(long long number)
{
	char str[100] = {0};
	snprintf(str, sizeof(str), "%lld", number );
	Append( str, (long)strlen(str) );  // Append(const char *s, long len)
	return *this;
}

TString &TString::Append(unsigned long long number)
{
	char str[100] = {0};
	snprintf(str, sizeof(str), "%llu", number );
	Append( str, (long)strlen(str) );  // Append(const char *s, long len)
	return *this;
}

TString &TString::Append(double number)
{
	char str[100] = {0};
	snprintf( str, sizeof(str), "%.8f", number );
	Append( str, (long)strlen(str) );  // Append(const char *s, long len)
	return *this;
}

TString &TString::AppendAsHex(unsigned char cc)
{
	return Append("0123456789ABCDEF"[(cc >> 4) & 0x0f]).Append("0123456789ABCDEF"[	   cc & 0x0f]);
}

TString &TString::AppendTwoHexAsChar(const char *p)
{
	char high = 0, low = 0;
	if (p && isxdigit(p[0]) && isxdigit(p[1])) {
		high = p[0];
		// PS: streamline comparison post CR
		if (isdigit(high)) { // if ('0' <= high && high <= '9')
			high -= '0';
		} else if ('a' <= high) { // if ('a' <= high&& high <= 'f')  // assume 'A' < 'a'
			high -= ('a' - 10);
		} else  {//if ('A' <= high && high <= 'F')
			high -= ('A' - 10);
		}
		low = p[1];
		if (isdigit(low)) { // if ('0' <= low && low <= '9')
			low -= '0';
		} else if ('a' <= low) { // if ('a' <= low&& low <= 'f')  // assume 'A' < 'a'
			low -= ('a' - 10);
		} else  {//if ('A' <= low && low <= 'F')
			low -= ('A' - 10);
		}
	}
	return Append(char(16 * high + low));
}

void TString::DumpAsHex(TString &outbuf, long dumpwidth) const
{
	if (Length() > 0) {
		long lTotalLen = (4L * dumpwidth + 1L);
		TString hexcode("0123456789ABCDEF");
		// set string width to TotalLen
		TString tmpBuf;
		tmpBuf.Set(0, 0, lTotalLen);
		long x = 0L, l = 0L, sz = 0;
		for (l = 0, sz = Length(); l < sz; ++l, ++x) {
			if (l % dumpwidth == 0) {
				x = 0L;
				if (l > 0) {
					// this here is to force a line break
					outbuf.Append(tmpBuf) << '\0';
				}
				// fill/clear the whole string with spaces
				for (long q = 0L; q < lTotalLen; ++q) {
					tmpBuf.fCont[q] = ' ';
				}
			}
			unsigned char c = fCont[l];
			// first fill hexnumber of character
			tmpBuf.fCont[x*3L] = hexcode[ (c >> 4) & 0x0f ];
			tmpBuf.fCont[x*3L+1L] = hexcode[ c & 0x0f ];
			if ( !isprint((unsigned char) c) ) {
				// print a dot for unprintable characters
				c = '.';
			}
			tmpBuf.fCont[3L*dumpwidth+1L+x] = c;
		}
		outbuf.Append(tmpBuf) << '\0';
	}
}

long TString::DiffDumpAsHex(TString &outbuf, const TString &strRight) const
{
	long lDiffPos = -1L;
	if (Length() > 0 || strRight.Length() > 0) {
		long maxLength = (Length() > strRight.Length()) ? Length() : strRight.Length();
		long dumpwidth = 16L;
		const char sep = '|';
		const char LF = '\n';
		// two times the hexoutput + a separator
		const long lWidth = (4L * dumpwidth + 1L);
		const long rightOffset = lWidth + 3L;
		const long sepPos = lWidth + 1L;
		const long lTotalLen = lWidth + rightOffset;
		const TString hexcode("0123456789ABCDEF");
		// set string width to TotalLen
		TString tmpBuf;
		tmpBuf.Set(0, 0, lTotalLen);
		long x = 0L, l = 0L;
		bool bEqual = true;
		for (l = 0; l < maxLength; ++l, ++x) {
			if (l % dumpwidth == 0) {
				x = 0L;
				if (l > 0) {
					// this here is to force a line break
					outbuf.Append(tmpBuf) << LF;
				} else {
					// first line
					TString firstLine;
					long l1, q;
					tmpBuf.Set(0, "expected", -1);
					l1 = tmpBuf.Length();
					tmpBuf.Set(lWidth, " | ", -1);
					for (q = l1; q < sepPos; ++q) {
						tmpBuf.fCont[q] = ' ';
					}
					tmpBuf.Set(rightOffset, "differences", -1);
					l1 = tmpBuf.Length();
					tmpBuf.Set(0, 0, lTotalLen);
					for (q = l1; q < lTotalLen; ++q) {
						tmpBuf.fCont[q] = ' ';
					}
					outbuf.Append(tmpBuf) << LF;
				}
				// fill/clear the whole string with spaces
				for (long q = 0L; q < lTotalLen; ++q) {
					tmpBuf.fCont[q] = ' ';
				}
				// add separator to string
				tmpBuf.fCont[sepPos] = sep;
			}
			unsigned char cLeft = '\0', cRight = '\0';
			if (l < Length()) {
				cLeft = fCont[l];
			}
			if (l < strRight.Length()) {
				cRight = strRight.fCont[l];
			}
			// compare characters
			if (bEqual && cLeft == cRight) {
				// do not print the ascii representation on the right side if equal
				// fill hexnumber of right character
				tmpBuf.fCont[x*3L + rightOffset] = '.';
				tmpBuf.fCont[x*3L + rightOffset + 1L] = '.';
			} else {
				// print ascii representation of both sides from first different character on
				if (bEqual) {
					// store position of first difference
					lDiffPos = l;
					bEqual = false;
				}
				if (l < strRight.Length()) {
					// fill hexnumber of right character
					tmpBuf.fCont[x*3L + rightOffset] = hexcode[ (cRight >> 4) & 0x0f ];
					tmpBuf.fCont[x*3L + rightOffset + 1L] = hexcode[ cRight & 0x0f ];
				}
				if ( !isprint((unsigned char) cRight) ) {
					// print a dot for right unprintable characters
					cRight = '.';
				}
				if (l < strRight.Length()) {
					tmpBuf.fCont[3L*dumpwidth+1L+rightOffset+x] = cRight;
				}
			}
			if (l < Length()) {
				// fill hexnumber of left character
				tmpBuf.fCont[x*3L] = hexcode[ (cLeft >> 4) & 0x0f ];
				tmpBuf.fCont[x*3L + 1L] = hexcode[ cLeft & 0x0f ];
				if ( !isprint((unsigned char) cLeft) ) {
					// print a dot for left unprintable characters
					cLeft = '.';
				}
				tmpBuf.fCont[3L*dumpwidth+1L+x] = cLeft;
			}
		}
		outbuf.Append(tmpBuf) << LF;
	}
	return lDiffPos;
}

// We don't use ostream directly because of the locking overhead. Because of
// a compiler weakness we have to undef ostream, so we can define operator<<
// for both types of streams.
// Otherwise operator<< wouldn't compile when using cout, cerr or clog
#if defined(__SUNPRO_CC) && !defined(__STD_OSTREAM__) && ( __SUNPRO_CC < 0x500 )

#undef ostream
unsafe_ostream &operator<<(unsafe_ostream &os, const TString &s)
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

ostream &operator<<(ostream &os, const TString &s)
{
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

	return os;
}
