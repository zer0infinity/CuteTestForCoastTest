/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#include "StringLengthResultMapper.h"
#include "StringStream.h"
#include "utf8.h"
#include "Context.h"

namespace {
	long getStringLength(String const &str) {
		long len = 0L;
		try {
			len = utf8::distance(str.cstr(), str.cstr()+str.Length());
		} catch (utf8::invalid_utf8& e) {
			len = str.Length();
		}
		StatTrace(StringLengthResultMapper.getStringLength, "len: " << len << " str [" << str << "]", Coast::Storage::Current());
		return len;
	}
}
RegisterResultMapper(StringLengthResultMapper);

bool StringLengthResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(StringLengthResultMapper.DoPutAny, "key [" << NotNull(key) << "]");
	if (value.GetType() != AnyArrayType) {
		String sText = value.AsString();
		Trace("native length of string: " << sText.Length());
		value = getStringLength(sText);
		Trace("length of value string: " << value.AsLong(-1L));
	}
	return ResultMapper::DoPutAny(key, value, ctx, script);
}

bool StringLengthResultMapper::DoPutStream(const char *key, std::istream & is, Context & ctx, ROAnything script) {
	StartTrace1(StringLengthResultMapper.DoPutStream, "key [" << NotNull(key) << "]");
	OStringStream content;
	long lRecv = 0, lToRecv = 2048;
	l_long lTotalReceived = 0LL;
	while (NSStringStream::PlainCopyStream2Stream(&is, content, lRecv, lToRecv) && lRecv == lToRecv)
		lTotalReceived += lRecv;
	lTotalReceived += lRecv;
	Trace("total characters read: " << lTotalReceived);
	Anything value = content.str();
	return DoPutAny(key, value, ctx, script);
}
