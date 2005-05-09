/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StringStream.h"

//--- standard modules used ----------------------------------------------------
#include "SysLog.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <assert.h>
#include <ctype.h>

StringStreamBuf::StringStreamBuf(int mode)
	: fStore(0), fDeleteStore(false), fOpenMode(mode)
{
	xinit(0, 0);
	setbufpointers(0, 0);
}

StringStreamBuf::StringStreamBuf(const String &s, int mode)
	: fStore(0), fDeleteStore(false), fOpenMode(mode)
{
	xinit(0, &s);
	setbufpointers(0, 0);
}

StringStreamBuf::StringStreamBuf(String *s, int mode)
	: fStore(s), fDeleteStore(false), fOpenMode(mode)
{
	xinit(s, 0);
	setbufpointers(0, 0);
}

StringStreamBuf::~StringStreamBuf()
{
	this->sync();
	if (fDeleteStore) {
		delete fStore;
		fStore = 0;
	}
	fOpenMode = 0;
}

String &StringStreamBuf::str()
{
	this->sync();
	Assert(fStore);
	return *fStore;
}

void StringStreamBuf::AdjustStringLength()
{
	if (pbase() && (fOpenMode & ios::out)) {
		Assert(pptr() && pptr() >= start());
		long newlen = pptr() - pbase();
		Assert(newlen < fStore->Capacity());
		if (newlen > fStore->Length()) {
			fStore->SetLength(newlen);
			fStore->GetContent()[newlen] = '\0'; // terminate the string
			setgetpointer(gptr() - eback()); // might be able to read more
		}
	}
}

void StringStreamBuf::xinit(  String *s, const String *contents)
{
	// adjust fOpenMode to contain valid combination of flags
	if (fOpenMode & (ios::ate | ios::app)) {
		fOpenMode |= ios::out;
	}
	if (fOpenMode & ios::trunc) { // clear the string, i.e. ignore contents
		contents = 0;
	}
	// initialize buffer, allocate new String object if required
	if (!s) {
		s = new String(16L);
		Assert(s != 0);
		fDeleteStore = true;
	}
	if (s != fStore) {
		if ( fStore && fDeleteStore ) {
			delete fStore;
		}
		fStore = s;
	}
	if (contents && (fStore != contents)) {
		(*fStore) = (*contents);    // string assignment
	}
	Assert(fStore);
}

void StringStreamBuf::setgetpointer(long getoffset)
{
	char *eg = 0;
	if (fStore && fStore->GetImpl()) {
		eg = fStore->GetContent() + fStore->Length();    // points after get area
	}
	setg((fOpenMode & ios::in) ? start() : eg, start() + getoffset , eg);
}

void StringStreamBuf::setbufpointers(long getoffset, long putoffset)
{
	char *sc = 0;
	if (fStore->GetImpl()) {
		sc = fStore->GetContent();
	}
	if ((fOpenMode & (ios::app | ios::ate)) && putoffset < fStore->Length()  ) {
		putoffset = fStore->Length(); // adjust it to the end
		if (fOpenMode & (ios::ate)) {
			fOpenMode &= ~ios::ate; // adjust it only once to the end
		}
	}
	// save 1 byte for '\0' termination
	char *endptr = sc + (fStore->Capacity() > 0 ? fStore->Capacity() - 1 : 0);
	// now init get an put area
	setgetpointer(getoffset);
	// if not reading the get area is defined empty.
	setp(sc, endptr);
	if (pbase()) {
		pbump(putoffset);    // needed because we no longer carry base()
	}
	if (! sc) {
		Assert(!start());
		Assert(!pbase());
		Assert(!gptr());
		Assert(!pptr());
	}
//	Assert(start() == fStore->GetContent());
	Assert(!pptr() || ( pptr() >= fStore->GetContent() && pptr() < fStore->GetContent() + fStore->Capacity()));
	Assert(!gptr() || (gptr() >= start() && gptr() <= egptr()));
}

// we only get overflow if the string is written past the current end
// adjust the string size accordingly
int StringStreamBuf::overflow( int c )
{
	Assert( pbase() ? pbase() <= pptr() : 0 == pptr() );

	if (c != EOF) {
		if (reserve(pptr() - pbase() + 1)) { // this will extend the string
			Assert(pptr() < epptr());
			return sputc(c);
		}
		// we failed somehow. return EOF
		return EOF;
	}
	AdjustStringLength();
	return 0;
} // overflow

int StringStreamBuf::underflow()
{
	if (gptr() < egptr()) {
		return ZAPEOF(*gptr());    // we still got something, false alarm
	}
	return EOF; // we never handle underflow, because our buffer is the String content
} // underflow

int StringStreamBuf::sync()
{
	if (fOpenMode & ios::out) {
		AdjustStringLength();
	}
	return 0; // never an error, (-1) would be
} // sync

bool StringStreamBuf::reserve(long newlength)
{
	// we might check if newlength is really bigger than fLength
	// second, only if fProtection includes eWrite it is possible to enlarge the file
	if (newlength >= fStore->Capacity()) {
		if (fOpenMode & ios::out) {
			AdjustStringLength();
			Assert((pptr() - pbase()) == fStore->Length());
			long putoffset = 0; // remember offsets if we readjust the mmap/file
			long getoffset = 0;
			Assert(start() == pbase());
			if (pbase() != 0 ) {
				if (pptr() > pbase()) {
					putoffset = pptr() - pbase();    // remember where we have been
				}
				if (gptr() > eback()) {
					getoffset = gptr() - eback();
				}
			}
			fStore->Reserve(newlength - fStore->Capacity() + 1); // allow 1 more for '\0'
			// optimization of enlargement is done by String class.
			setbufpointers(getoffset, putoffset); // readjust, pointer might have changed
			Assert((epptr() - pbase()) == fStore->Capacity() - 1); // allow 1 more for '\0'
			return true;
		} else {
			return false;
		}
	}
	return true; // space is still available, false alarm
}

//
// if p is in range it sets the position for further get operations
// otherwise it returns EOF;
// This function isn't implemented for positioning of the output pointer;
//
StringStreamBuf::pos_type StringStreamBuf::seekpos(StringStreamBuf::pos_type p, StringStreamBuf::openmode mode)
{
	AdjustStringLength();
	if (long(p) >= long(fStore->Capacity())) {
		// we need to enlarge the string
		// we can only if we write
		if (! (mode & ios::out) || !reserve((long)p)) {
			// OOPS we got a problem
			return pos_type(EOF);
		}
	}
	if (mode & ios::in) {
		setgetpointer(p);
	}
	if (mode & ios::out) {
		if (fOpenMode & ios::app) { // do this on a best try basis
			if (p < fStore->Length() && fStore->Length() > 0) {
				p = fStore->Length();    // always go to the end
			}
		}
		// PS: should not be needed: if (p > fFileLength) fFileLength = p;
		setp(start(), epptr());
		if (pbase()) {
			pbump(p);    // needed, because we no longer carry base()
		}
	}
	return p;
} // seekpos

//
// Sets the relative position for further get operations;
// returns EOF if out of range or input isn't allowed;
//
//! standard iostream behavior, adjust put or get position relatively
StringStreamBuf::pos_type StringStreamBuf::seekoff(StringStreamBuf::off_type of, StringStreamBuf::seekdir dir, StringStreamBuf::openmode mode)
{
	//sync(); // will adjust fFileLength if needed
	AdjustStringLength(); // recognize where we have been with putting
	long pos = long(of);
	pos += (dir == ios::cur) ? long((mode & ios::in ? gptr() : pptr()) - pbase()) :
			   (dir == ios::end && fStore->Length() > 0) ? long(fStore->Length()) : 0L;
	if (pos < 0L ) {
		return pos_type(EOF);
	}
	return seekpos(pos, mode);
} // seekoff

bool StringStream::PlainCopyStream2Stream(istream *streamSrc, ostream &streamDest, long &copiedBytes, long lBytes2Copy)
{
	StartTrace(StringStream.PlainCopyStream2Stream);
	streamsize read = 0;
	Trace("Bytes to copy: " << lBytes2Copy);
	String sBuf(lBytes2Copy);
	char *buf = (char *)(const char *) sBuf;
	copiedBytes = 0L;
	bool bRet = true;
	while ( lBytes2Copy > 0L ) {
		if ( streamSrc->good() ) {
			if ( streamDest.good() ) {
				streamSrc->read(buf, (int)lBytes2Copy);
				read = streamSrc->gcount();
				streamDest.write(buf, read);
				copiedBytes += (long)read;
				lBytes2Copy -= (long)read;
				Trace("Bytes copied so far: " << copiedBytes);
			} else {
				Trace("Destination stream is not good , aborting copy!");
				bRet = false;
				break;
			}
		} else {
			// test if we reached eof of streamSrc
			bRet = (streamSrc->eof() != 0);
			Trace("Source stream is not good anymore" << (bRet ? " (eof)" : "") << ", finishing copy!" );
			break;
		}
	}
	Trace("bytes copied this time: " << copiedBytes);
	return bRet;
}
