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

#define SS_TRACE(msg)

#if defined(WIN32)
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

#include "StringStreamBuf.h"


//! istream for Strings, read input from a string
template
<
typename BufferType
>
class  EXPORTDECL_FOUNDATION IStringStreamTmpl : public StringStreambase< typename Coast::TypeTraits::fooTypeTraits<BufferType>::ConstPlainTypePtr, Coast::TypeTraits::Int2Type<NSStringStream::eIn> > , public std::istream
{
public:
	typedef IStringStreamTmpl<BufferType> ThisClassType;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::ConstPlainTypePtr IntBufType;
	typedef typename Coast::TypeTraits::Int2Type<NSStringStream::eIn> IoDirType;
	typedef StringStreambase< IntBufType, IoDirType > StreamBaseType;
	typedef typename Coast::TypeTraits::fooTypeTraits<IntBufType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename Coast::TypeTraits::fooTypeTraits<IntBufType>::ConstPlainTypePtr ConstPlainTypePtr;
	typedef typename Coast::TypeTraits::fooTypeTraits<IntBufType>::PlainType PlainType;
	typedef typename Coast::TypeTraits::fooTypeTraits<IntBufType>::ConstPlainType ConstPlainType;

public:
	/*! ctor, take String pointed to by s to read the input direct input taken from string
		\param s use *s as the underlying input buffer, no copying, most efficient */
	explicit IStringStreamTmpl(ConstPlainTypePtr s)
		: StreamBaseType(*s, std::ios::in)
		, std::istream(StreamBaseType::rdbuf()) {
		SS_TRACE((IStringStreamTmpl.IStringStreamTmpl, "ConstPlainTypePtr"));
	}

	/*! ctor, take String value as input, read input from copy of s
		\param s use s contents as initial buffer content for input */
	explicit IStringStreamTmpl(ConstPlainTypeRef s)
		: StreamBaseType(s, std::ios::in)
		, std::istream(StreamBaseType::rdbuf()) {
		SS_TRACE((IStringStreamTmpl.IStringStreamTmpl, "ConstPlainTypeRef"));
	}
	//! dtor, not much to do
	~IStringStreamTmpl() { }

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
	if ( ( is.rdstate() & std::istream::failbit ) != 0 ) {
		l = dflt;
	}
	return is;
}
template < typename BT > IStringStreamTmpl<BT>& operator >>( IStringStreamTmpl<BT>& is, double &d )
{
	double dflt = d;
	is.operator >> (d);
	if ( ( is.rdstate() & std::istream::failbit ) != 0 ) {
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
class  EXPORTDECL_FOUNDATION OStringStreamTmpl : public StringStreambase< typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypePtr, Coast::TypeTraits::Int2Type<NSStringStream::eOut> > , public std::ostream
{
public:
	typedef OStringStreamTmpl<BufferType> ThisClassType;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypePtr IntBufType;
	typedef typename Coast::TypeTraits::Int2Type<NSStringStream::eOut> IoDirType;
	typedef StringStreambase< IntBufType, IoDirType > StreamBaseType;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypeRef PlainTypeRef;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypePtr PlainTypePtr;

public:
	//! ctor, allocate new string object internally for output
	explicit OStringStreamTmpl(int mode = std::ios::out)
		: StreamBaseType(mode | std::ios::out)
		, std::ostream(StreamBaseType::rdbuf()) {
		SS_TRACE(OStringStreamTmpl.OStringStreamTmpl);
	}

	/*! ctor, take s as output target
		\param s use *s as the underlying output buffer directly, no copying
		\param mode specify streams input/output mode */
	explicit OStringStreamTmpl(PlainTypePtr s, int mode = std::ios::app)
		: StreamBaseType(s, mode | std::ios::out)
		, std::ostream(StreamBaseType::rdbuf()) {
		SS_TRACE((OStringStreamTmpl.OStringStreamTmpl, "PlainTypePtr"));
	}

	/*! ctor, take s as output target
		i am not sure if compiler will take this correctly with the const String& constructor, needs to be tested
		\param s use s contents as initial content
		\param mode specify streams input/output mode */
	explicit OStringStreamTmpl(PlainTypeRef s, int mode = std::ios::app)
		: StreamBaseType(&s, mode | std::ios::out)
		, std::ostream(StreamBaseType::rdbuf()) {
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
	: public StringStreambase< typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypePtr, typename Coast::TypeTraits::Int2Type<NSStringStream::eOut> >
	, public std::iostream
{
public:
	typedef StringStreamTmpl<BufferType> ThisClassType;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypePtr IntBufType;
	typedef typename Coast::TypeTraits::Int2Type<NSStringStream::eOut> IoDirType;
	typedef StringStreambase< IntBufType, IoDirType > StreamBaseType;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypeRef PlainTypeRef;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::ConstPlainTypeRef ConstPlainTypeRef;
	typedef typename Coast::TypeTraits::fooTypeTraits<BufferType>::PlainTypePtr PlainTypePtr;
public:
	//! ctor, allocate new string object internally for in/output
	explicit StringStreamTmpl(int mode = std::ios::out | std::ios::in)
		: StreamBaseType(mode)
		, std::iostream(StreamBaseType::rdbuf()) {
		SS_TRACE(StringStreamTmpl.StringStreamTmpl);
	}
	/*! ctor, take s as output target, resp. input source
		\param s use *s as the underlying output buffer directly, no copying
		\param mode specify streams input/output mode */
	explicit StringStreamTmpl(PlainTypePtr s, int mode = std::ios::out | std::ios::in)
		: StreamBaseType(s, mode)
		, std::iostream(StreamBaseType::rdbuf()) {
		SS_TRACE((StringStreamTmpl.StringStreamTmpl, "PlainTypePtr"));
	}
	/*! ctor, take s as initial content,
		makes sense with mode containting std::ios::app|std::ios::ate|std::ios::in
		\param s use s contents as initial content
		\param mode specify streams input/output mode */
	explicit StringStreamTmpl(ConstPlainTypeRef s, int mode = std::ios::in)
		: StreamBaseType(s, mode)
		, std::iostream(StreamBaseType::rdbuf()) {
		SS_TRACE((StringStreamTmpl.StringStreamTmpl, "ConstPlainTypeRef"));
	}
	/*! ctor, take s as output target and input source
		i am not sure if compiler will take this correctly with the const String&
		constructor, needs to be tested
		\param s use s contents as initial content
		\param mode specify streams input/output mode */
	explicit StringStreamTmpl(PlainTypeRef s, int mode = std::ios::out | std::ios::in)
		: StreamBaseType(&s, mode | std::ios::out)
		, std::iostream(StreamBaseType::rdbuf()) {
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
	bool EXPORTDECL_FOUNDATION PlainCopyStream2Stream(std::istream *streamSrc, std::ostream &streamDest, long &copiedBytes, long lBytes2Copy = 2048L);
};

#endif