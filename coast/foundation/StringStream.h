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

#include "ITOTypeTraits.h"
#include "ITOString.h"

//#define SS_TRACE(msg)	cerr << _QUOTE_(msg) << endl
#define SS_TRACE(msg)

#if defined(ONLY_STD_IOSTREAM)
using std::iostream;
#define seek_dir seekdir
#else
#if !defined(WIN32)
#include <streambuf.h>
#else
typedef int streamsize;
#endif
#endif

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

#include "StringStreamBuf.h"

//! istream for Strings, read input from a string
template
<
typename BufferType
>
class  EXPORTDECL_FOUNDATION IStringStreamTmpl : public StringStreambase< typename Loki::fooTypeTraits<BufferType>::ConstPlainTypePtr, Loki::Int2Type<NSStringStream::eIn> >, public istream
{
public:
	typedef IStringStreamTmpl<BufferType> ThisClassType;
	typedef typename Loki::fooTypeTraits<BufferType>::ConstPlainTypePtr IntBufType;
	typedef typename Loki::Int2Type< NSStringStream::eIn > IoDirType;
	typedef StringStreambase< IntBufType, IoDirType > StreamBaseType;
	typedef typename Loki::fooTypeTraits<IntBufType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename Loki::fooTypeTraits<IntBufType>::ConstPlainTypePtr ConstPlainTypePtr;
	typedef typename Loki::fooTypeTraits<IntBufType>::PlainType PlainType;
	typedef typename Loki::fooTypeTraits<IntBufType>::ConstPlainType ConstPlainType;

public:
	/*! ctor, take String pointed to by s to read the input direct input taken from string
		\param s use *s as the underlying input buffer, no copying, most efficient */
	explicit IStringStreamTmpl(ConstPlainTypePtr s)
		: StreamBaseType(*s, ios::in)
		, istream(StreamBaseType::rdbuf()) {
		SS_TRACE((IStringStreamTmpl.IStringStreamTmpl, "ConstPlainTypePtr"));
	}

	/*! ctor, take String value as input, read input from copy of s
		\param s use s contents as initial buffer content for input */
	explicit IStringStreamTmpl(ConstPlainTypeRef s)
		: StreamBaseType(s, ios::in)
		, istream(StreamBaseType::rdbuf()) {
		SS_TRACE((IStringStreamTmpl.IStringStreamTmpl, "ConstPlainTypeRef"));
	}
	//! dtor, not much to do
	~IStringStreamTmpl() { }

#if defined(WIN32) && !defined(ONLY_STD_IOSTREAM)
	ThisClassType &operator >>( unsigned long &ul ) {
		istream::operator>>(ul);
		return *this;
	}
	ThisClassType &operator >>( char *psz ) {
		istream::operator>>(psz);
		return *this;
	}
	ThisClassType &operator >>( unsigned char *pusz ) {
		istream::operator>>(pusz);
		return *this;
	}
	ThisClassType &operator >>( signed char *pssz ) {
		istream::operator>>(pssz);
		return *this;
	}
	ThisClassType &operator >>( char &rch ) {
		istream::operator>>(rch);
		return *this;
	}
	ThisClassType &operator >>( unsigned char &ruch ) {
		istream::operator>>(ruch);
		return *this;
	}
	ThisClassType &operator >>( signed char &rsch ) {
		istream::operator>>(rsch);
		return *this;
	}
	ThisClassType &operator >>( short &s ) {
		istream::operator>>(s);
		return *this;
	}
	ThisClassType &operator >>( unsigned short &us ) {
		istream::operator>>(us);
		return *this;
	}
	ThisClassType &operator >>( int &n ) {
		istream::operator>>(n);
		return *this;
	}
	ThisClassType &operator >>( unsigned int &un ) {
		istream::operator>>(un);
		return *this;
	}
	ThisClassType &operator >>( float &f ) {
		istream::operator>>(f);
		return *this;
	}
	ThisClassType &operator >>( streambuf *psb ) {
		istream::operator>>(psb);
		return *this;
	}
	ThisClassType &operator >>( istream & (*fcn)(istream &) ) {
		istream::operator>>(fcn);
		return *this;
	}
	ThisClassType &operator >>( ios & (*fcn)(ios &) ) {
		istream::operator>>(fcn);
		return *this;
	}
#endif // WIN32

private:
	IStringStreamTmpl();
	// do not allow on the fly copies put onto the stack
	IStringStreamTmpl(const char *);
	ThisClassType &operator=(const ThisClassType &);
};

// fixing bug in istream runtime lib, the default value in long/double conversion
// is overwritten in case the conversion fails. Other cases should be tested too !
template < typename BT > IStringStreamTmpl<BT>& operator >>( IStringStreamTmpl<BT>& is, long &l )
{
	long dflt = l;
	is.operator >> (l);
	if ( ( is.rdstate() & istream::failbit ) != 0 ) {
		l = dflt;
	}
	return is;
}
template < typename BT > IStringStreamTmpl<BT>& operator >>( IStringStreamTmpl<BT>& is, double &d )
{
	double dflt = d;
	is.operator >> (d);
	if ( ( is.rdstate() & istream::failbit ) != 0 ) {
		d = dflt;
	}
	return is;
}

typedef IStringStreamTmpl<String> IStringStream;

//! ostream for Strings, output to a String object
template
<
typename BufferType
>
class  EXPORTDECL_FOUNDATION OStringStreamTmpl : public StringStreambase< typename Loki::fooTypeTraits<BufferType>::PlainTypePtr, Loki::Int2Type<NSStringStream::eOut> >, public ostream
{
public:
	typedef OStringStreamTmpl<BufferType> ThisClassType;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypePtr IntBufType;
	typedef typename Loki::Int2Type<NSStringStream::eOut> IoDirType;
	typedef StringStreambase< IntBufType, IoDirType > StreamBaseType;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypeRef PlainTypeRef;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypePtr PlainTypePtr;

public:
	//! ctor, allocate new string object internally for output
	explicit OStringStreamTmpl(int mode = ios::out)
		: StreamBaseType(mode | ios::out)
		, ostream(StreamBaseType::rdbuf()) {
		SS_TRACE(OStringStreamTmpl.OStringStreamTmpl);
	}

	/*! ctor, take s as output target
		\param s use *s as the underlying output buffer directly, no copying */
	explicit OStringStreamTmpl(PlainTypePtr s, int mode = ios::app)
		: StreamBaseType(s, mode | ios::out)
		, ostream(StreamBaseType::rdbuf()) {
		SS_TRACE((OStringStreamTmpl.OStringStreamTmpl, "PlainTypePtr"));
	}

	/*! ctor, take s as output target
		i am not sure if compiler will take this correctly with the const String& constructor, needs to be tested
		\param s use s contents as initial content */
	explicit OStringStreamTmpl(PlainTypeRef s, int mode = ios::app)
		: StreamBaseType(&s, mode | ios::out)
		, ostream(StreamBaseType::rdbuf()) {
		SS_TRACE((OStringStreamTmpl.OStringStreamTmpl, "PlainTypeRef"));
	}
	//! dtor, not much to do
	~OStringStreamTmpl() { }

private:
	OStringStreamTmpl(const ThisClassType &);
	ThisClassType &operator=(const ThisClassType &);
};

typedef OStringStreamTmpl<String> OStringStream;

//! iostream for Strings, input and output to a String object
template
<
typename BufferType
>
class EXPORTDECL_FOUNDATION StringStreamTmpl
	: public StringStreambase< typename Loki::fooTypeTraits<BufferType>::PlainTypePtr, typename Loki::Int2Type<NSStringStream::eOut> >
	, public iostream
{
public:
	typedef StringStreamTmpl<BufferType> ThisClassType;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypePtr IntBufType;
	typedef typename Loki::Int2Type<NSStringStream::eOut> IoDirType;
	typedef StringStreambase< IntBufType, IoDirType > StreamBaseType;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypeRef PlainTypeRef;
	typedef typename Loki::fooTypeTraits<BufferType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename Loki::fooTypeTraits<BufferType>::PlainTypePtr PlainTypePtr;
public:
	//! ctor, allocate new string object internally for in/output
	explicit StringStreamTmpl(int mode = ios::out | ios::in)
		: StreamBaseType(mode)
		, iostream(StreamBaseType::rdbuf()) {
		SS_TRACE(StringStreamTmpl.StringStreamTmpl);
	}
	/*! ctor, take s as output target, resp. input source
		\param s use *s as the underlying output buffer directly, no copying */
	explicit StringStreamTmpl(PlainTypePtr s, int mode = ios::out | ios::in)
		: StreamBaseType(s, mode)
		, iostream(StreamBaseType::rdbuf()) {
		SS_TRACE((StringStreamTmpl.StringStreamTmpl, "PlainTypePtr"));
	}
	/*! ctor, take s as initial content,
		makes sense with mode containting ios::app|ios::ate|ios::in
		\param s use s contents as initial content */
	explicit StringStreamTmpl(ConstPlainTypeRef s, int mode = ios::in)
		: StreamBaseType(s, mode)
		, iostream(StreamBaseType::rdbuf()) {
		SS_TRACE((StringStreamTmpl.StringStreamTmpl, "ConstPlainTypeRef"));
	}
	/*! ctor, take s as output target and input source
		i am not sure if compiler will take this correctly with the const String&
		constructor, needs to be tested
		\param s use s contents as initial content */
	explicit StringStreamTmpl(PlainTypeRef s, int mode = ios::out | ios::in)
		: StreamBaseType(&s, mode | ios::out)
		, iostream(StreamBaseType::rdbuf()) {
		SS_TRACE((StringStreamTmpl.StringStreamTmpl, "PlainTypeRef"));
	}
	//! dtor, not much to do
	~StringStreamTmpl() { }

private:
	StringStreamTmpl(const ThisClassType &);
	ThisClassType &operator=(const ThisClassType &);
};

typedef StringStreamTmpl<String> StringStream;

namespace NSStringStream
{
	/*! Utility function to copy uninterpreted (plain) bytes from stream to stream.
		\note This is a blocking call if used with SocketStreams!
		\param streamSrc stream to read from
		\param streamDest stream to write into
		\param copiedBytes number of bytes copied during this call, depending on lBytes2Copy subsequent calls might be needed to copy everything from source to destination
		\param lBytes2Copy number of bytes to copy per call
		\return true if copying was successful. If copiedBytes is lower than lBytes2Copy and the call returned true everything was read from streamSrc (eof) and copied into streamDest. If copiedBytes is equal to lBytes2Copy and the call returned true the function should be called again to consume remaining bytes from streamSrc. False indicates either an error condition on streamSrc or streamDest. */
	bool EXPORTDECL_FOUNDATION PlainCopyStream2Stream(istream *streamSrc, ostream &streamDest, long &copiedBytes, long lBytes2Copy = 2048L);
};

#endif
