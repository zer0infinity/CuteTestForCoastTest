/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _STRINGSTREAM_H
#define _STRINGSTREAM_H

#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION

#if defined(ONLY_STD_IOSTREAM)
#include <iostream>
#include <iomanip>
using namespace std;
#define seek_dir seekdir
#else
#if !defined(WIN32)
#include <streambuf.h>
#else
typedef int streamsize;
#endif
#include <iostream.h>
#include <iomanip.h>
#endif

#include "ITOString.h"

#if defined(WIN32)
#if !defined(ONLY_STD_IOSTREAM)
inline ostream &operator<< (ostream &os, __int64 value)
{
	char buf[22] = { '\0' };
	os << _i64toa( value, buf, 10 );
	return os;
}
inline ostream &operator<< (ostream &os, unsigned __int64 value)
{
	char buf[22] = { '\0' };
	os << _ui64toa( value, buf, 10 );
	return os;
}
inline istream &operator>> (istream &is, __int64 &value)
{
	String strBuf;
	// read up to next whitespace
	is >> strBuf;
	// check for string sanity, format: [sign]digits
	if ( strBuf.Length() && ( isdigit(strBuf.At(0)) || ( (strBuf.At(0) == '+' || strBuf.At(0) == '-') && (strBuf.Length() > 1 && isdigit(strBuf.At(1))) ) ) ) {
		__int64 llVal = _atoi64( (const char *)strBuf );
		// converter returns 0 in case of a failure
		value = llVal;
	}
	return is;
}
inline istream &operator>> (istream &is, unsigned __int64 &value)
{
	__int64 llVal = (__int64)value;
	is >> llVal;
	value = (unsigned __int64)llVal;
	return is;
}
#else
inline std::ostream &operator<< (std::ostream &os, __int64 value)
{
	char buf[22] = { '\0' };
	os << _i64toa( value, buf, 10 );
	return os;
}
inline std::ostream &operator<< (std::ostream &os, unsigned __int64 value)
{
	char buf[22] = { '\0' };
	os << _ui64toa( value, buf, 10 );
	return os;
}
inline std::istream &operator>> (std::istream &is, __int64 &value)
{
	String strBuf;
	// read up to next whitespace
	is >> strBuf;
	// check for string sanity, format: [sign]digits
	if ( strBuf.Length() && ( isdigit(strBuf.At(0)) || ( (strBuf.At(0) == '+' || strBuf.At(0) == '-') && (strBuf.Length() > 1 && isdigit(strBuf.At(1))) ) ) ) {
		__int64 llVal = _atoi64( (const char *)strBuf );
		// converter returns 0 in case of a failure
		value = llVal;
	}
	return is;
}
inline std::istream &operator>> (std::istream &is, unsigned __int64 &value)
{
	__int64 llVal = (__int64)value;
	is >> llVal;
	value = (unsigned __int64)llVal;
	return is;
}
#endif
#endif

//! StringStreamBuf adapts String objects to the iostream framework
/*! the underlying string is used directly as the buffer to save copying overhead */
class EXPORTDECL_FOUNDATION StringStreamBuf : public streambuf
{
public:
	/*! default ctor, allocates new internal String object for output
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreamBuf(int mode = ios::out);
	/*! ctor usually used for input
		\param s contains characters to be read
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreamBuf(const String &s, int mode = ios::in);
	/*! ctor usually used for output
		\param s target string to be filled,
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreamBuf(String *s, int mode = ios::out);
	/*! dtor, deletes string object if it has been allocated by this */
	virtual ~StringStreamBuf();
	/*! not much to do when synchronizing, just insert string termination character */
	virtual int sync();

	/*! reveal underlying string.
		\return underlying string buffer
		\note do not change it when continuing using the stream */
	String &str();

#if defined(ONLY_STD_IOSTREAM)
protected: // seekxxx are protected in the std..
	typedef std::streambuf::pos_type	pos_type;
	typedef std::streambuf::off_type	off_type;
	typedef std::ios::seekdir	seekdir;
	typedef std::ios::openmode	openmode;
#else
	typedef streampos pos_type;
	typedef	streamoff off_type;
	typedef ios::seek_dir seekdir;
	typedef ios::openmode openmode;
#endif

	/*! standard iostream behavior, adjust put or get position absolutely */
	virtual pos_type seekpos(pos_type pos, openmode mode = ios::in | ios::out);
	/*! standard iostream behavior, adjust put or get position relatively */
	virtual pos_type seekoff(off_type off, seekdir dir, openmode mode = ios::in | ios::out);

protected:
	/*! standard iostream behavior, extends the String
		\return EOF if extension is not possible (this might be a big problem) */
	virtual int overflow(int c = EOF);
	/*! standard iostream behavior, look for new characters available (no op for Strings)
		can't produce new chars because we always use the whole String
		\return EOF if no more characters for input available = EOF reached */
	virtual int underflow();

private:
	/*! auxiliary StringStreamBuf initialization */
	void xinit( String *s = 0, const String *contents = 0);
	/*! auxiliary operation to keep track of really output bytes, terminate string */
	void AdjustStringLength() ;
	/*! auxiliary operation to set iostream get buffer pointers according to parameters */
	void setgetpointer(long getoffset);
	/*! auxiliary operation to set iostream buffer pointers according to parameters */
	void setbufpointers(long getoffset, long putoffset);
	/*! enlarge the underlying String, adjust buffer pointers if needed (expand file size and mapped region) */
	bool reserve ( long newlength );
	/*! initialize get and put pointers of streambuf (set internal put und get buffer and position) */
	void initGetPut();
	/*! auxiliary functions for interfacing to setp and setg, getting rid of
		old streambuf style setb() and base() */
	char *start() {
		return (fStore && fStore->GetImpl() ? fStore->GetContent() : 0);
	}
	char *endbuf() {
		if (fStore && fStore->GetImpl()) {
			return fStore->GetContent() + fStore->Capacity() - 1;
		}
		return 0;
	}
	char *endget() {
		if (fStore && fStore->GetImpl()) {
			return fStore->GetContent() + fStore->Length();
		}
		return 0;
	}

	//! internal buffer of stream
	String *fStore;
	//! flag for remembering who allocated fStore
	//! is true when the default constructor has created a string
	bool fDeleteStore;
	//! saves open mode from ctor params
	int    fOpenMode;
};

//! adapts ios to a StringStream buffer --> THIS CLASS IS NOT TO BE INSTANTIATED
//! may be unsafe_ios
class EXPORTDECL_FOUNDATION StringStreambase : virtual public ios
{
public:
	/*! default ctor, allocates new internal String object for output
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreambase(int mode = ios::out)
		: fSSBuf(mode) {
		// init from ios is needed, because ios() won't do the job; (see comment in iostream.h)
		init(&fSSBuf);
	}
	/*! ctor usually used for input
		\param s contains characters to be read
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreambase(const String &s, int mode = ios::in)
		: fSSBuf(s, mode) {
		init(&fSSBuf);
	}

	/*! ctor usually used for output
		\param s target string to be filled,
		\param mode ios modes, bitwise or of [in|out|app|ate]: if mode is ios::app or ios::ate output is appended */
	StringStreambase(String *s, int mode = ios::out)
		: fSSBuf(s, mode) {
		init(&fSSBuf);
	}

	//! dtor not much to do
	virtual ~StringStreambase() {}

	/*! reveal underlying streambuf implementation
		\return underlying streambuf */
	StringStreamBuf *rdbuf()  {
		return &fSSBuf;
	}

	/*! reveal underlying string.
		\return underlying string buffer
		\note do not change it when continuing using the stream */
	String &str()         {
		return fSSBuf.str();
	}

protected:
	//! the buffer with its underlying String
	StringStreamBuf fSSBuf;
};

//! istream for Strings, read input from a string
class  EXPORTDECL_FOUNDATION IStringStream : public StringStreambase, public istream
{
public:
	/*! ctor, take String pointed to by s to read the input direct input taken from string
		\param s use *s as the underlying input buffer, no copying, most efficient */
	IStringStream(String *s)
		: StringStreambase(s, ios::in)
		, istream(&fSSBuf)
	{ }
	/*! ctor, take String value as input, read input from copy of s
		\param s use s contents as initial buffer content for input */
	IStringStream(const String &s)
		: StringStreambase(s, ios::in)
		, istream(&fSSBuf)
	{ }
	//! dtor, not much to do
	~IStringStream() { }

#if defined(WIN32) && !defined(ONLY_STD_IOSTREAM)
	// fixing bug in istream runtime lib, the default value in long conversion
	// is overwritten in case the conversion fails. Other cases should be tested too !
	IStringStream &operator >>( long &l ) {
		long save = l;
		istream::operator>>(l);
		if (fail()) {
			l = save;
		}
		return *this;
	}

	IStringStream &operator >>( unsigned long &ul ) {
		istream::operator>>(ul);
		return *this;
	}
	IStringStream &operator >>( char *psz ) {
		istream::operator>>(psz);
		return *this;
	}
	IStringStream &operator >>( unsigned char *pusz ) {
		istream::operator>>(pusz);
		return *this;
	}
	IStringStream &operator >>( signed char *pssz ) {
		istream::operator>>(pssz);
		return *this;
	}
	IStringStream &operator >>( char &rch ) {
		istream::operator>>(rch);
		return *this;
	}
	IStringStream &operator >>( unsigned char &ruch ) {
		istream::operator>>(ruch);
		return *this;
	}
	IStringStream &operator >>( signed char &rsch ) {
		istream::operator>>(rsch);
		return *this;
	}
	IStringStream &operator >>( short &s ) {
		istream::operator>>(s);
		return *this;
	}
	IStringStream &operator >>( unsigned short &us ) {
		istream::operator>>(us);
		return *this;
	}
	IStringStream &operator >>( int &n ) {
		istream::operator>>(n);
		return *this;
	}
	IStringStream &operator >>( unsigned int &un ) {
		istream::operator>>(un);
		return *this;
	}
	IStringStream &operator >>( float &f ) {
		istream::operator>>(f);
		return *this;
	}
	IStringStream &operator >>( double &d ) {
		istream::operator>>(d);
		return *this;
	}
	IStringStream &operator >>( streambuf *psb ) {
		istream::operator>>(psb);
		return *this;
	}
	IStringStream &operator >>( istream & (*fcn)(istream &) ) {
		istream::operator>>(fcn);
		return *this;
	}
	IStringStream &operator >>( ios & (*fcn)(ios &) ) {
		istream::operator>>(fcn);
		return *this;
	}
#endif // WIN32
};

//! ostream for Strings, output to a String object
class  EXPORTDECL_FOUNDATION OStringStream : public StringStreambase, public ostream
{
public:
	//! ctor, allocate new string object internally for output
	OStringStream(int mode = ios::out)
		: StringStreambase(mode | ios::out)
		, ostream(&fSSBuf)
	{  }
	/*! ctor, take s as output target
		\param s use *s as the underlying output buffer directly, no copying */
	OStringStream(String *s, int mode = ios::app)
		: StringStreambase(s, mode | ios::out)
		, ostream(&fSSBuf)
	{  }
	/*! ctor, take s as initial content, makes sense with mode containting ios::app|ios::ate
		\param s use s contents as initial content */
	OStringStream(const String &s, int mode = ios::out)
		: StringStreambase(s, mode | ios::out)
		, ostream(&fSSBuf)
	{  }
	/*! ctor, take s as output target
		i am not sure if compiler will take this correctly with the const String& constructor, needs to be tested
		\param s use s contents as initial content */
	OStringStream(String &s, int mode = ios::app)
		: StringStreambase(&s, mode | ios::out)
		, ostream(&fSSBuf)
	{  }
	//! dtor, not much to do
	~OStringStream() { }
};

//! iostream for Strings, input and output to a String object
class  EXPORTDECL_FOUNDATION StringStream : public StringStreambase, public iostream
{
public:
	//! ctor, allocate new string object internally for in/output
	StringStream(int mode = ios::out | ios::in)
		: StringStreambase(mode)
		, iostream(&fSSBuf)
	{  }
	/*! ctor, take s as output target, resp. input source
		\param s use *s as the underlying output buffer directly, no copying */
	StringStream(String *s, int mode = ios::out | ios::in)
		: StringStreambase(s, mode)
		, iostream(&fSSBuf)
	{  }
	/*! ctor, take s as initial content,
		makes sense with mode containting ios::app|ios::ate|ios::in
		\param s use s contents as initial content */
	StringStream(const String &s, int mode = ios::out | ios::in)
		: StringStreambase(s, mode)
		, iostream(&fSSBuf)
	{  }
	/*! ctor, take s as output target and input source
		i am not sure if compiler will take this correctly with the const String&
		constructor, needs to be tested
		\param s use s contents as initial content */
	StringStream(String &s, int mode = ios::out | ios::in)
		: StringStreambase(&s, mode | ios::out)
		, iostream(&fSSBuf)
	{  }
	//! dtor, not much to do
	~StringStream() { }

	/*! Utility function to copy uninterpreted (plain) bytes from stream to stream.
		\note This is a blocking call if used with SocketStreams!
		\param streamSrc stream to read from
		\param streamDest stream to write into
		\param copiedBytes number of bytes copied during this call, depending on lBytes2Copy subsequent calls might be needed to copy everything from source to destination
		\param lBytes2Copy number of bytes to copy per call
		\return true if copying was successful. If copiedBytes is lower than lBytes2Copy and the call returned true everything was read from streamSrc (eof) and copied into streamDest. If copiedBytes is equal to lBytes2Copy and the call returned true the function should be called again to consume remaining bytes from streamSrc. False indicates either an error condition on streamSrc or streamDest. */
	static bool PlainCopyStream2Stream(istream *streamSrc, ostream &streamDest, long &copiedBytes, long lBytes2Copy = 2048L);
};

#endif
