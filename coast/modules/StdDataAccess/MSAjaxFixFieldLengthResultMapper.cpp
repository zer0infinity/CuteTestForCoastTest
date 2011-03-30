/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#include "MSAjaxFixFieldLengthResultMapper.h"
#include "StringStream.h"
#include "utf8.h"
#include <algorithm>


namespace {
	char const *_FieldList = "Fields";
	char const *_FieldSeparator = "FieldSeparator";

	struct appendFunctor {
		typedef std::back_insert_iterator<String> stringInserter;
		stringInserter backinsert;
		appendFunctor(String& strRet): backinsert(strRet) {}
		void operator()(unsigned char const c) {
			StatTrace(UTF8Renderer.append, "character to add:" << static_cast<long>(c), Coast::Storage::Current());
			utf8::append(static_cast<utf8::uint32_t>(c), backinsert);
		}
	};
	String AsUTF8(String const &strInput) {
		StartTrace(UTF8Renderer.AsUTF8);
		String strRet(strInput.Length());
		std::for_each(strInput.begin(), strInput.end(), appendFunctor(strRet));
		return strRet;
	}
}

RegisterResultMapper(MSAjaxFixFieldLengthResultMapper);

bool MSAjaxFixFieldLengthResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(MSAjaxFixFieldLengthResultMapper.DoPutAny, "key [" << NotNull(key) << "]");
	if (value.GetType() != AnyArrayType) {
		String sText = value.AsString();
		ROAnything roaFieldList;
		long nFields = 0L;
		if (Lookup(_FieldList, roaFieldList, '\000') && ( nFields = roaFieldList.GetSize() ) > 0L ) {
			value = sText;
		}
	}
	return ResultMapper::DoPutAny(key, value, ctx, script);
}

bool MSAjaxFixFieldLengthResultMapper::DoPutStream(const char *key, std::istream & is, Context & ctx, ROAnything script) {
	StartTrace1(MSAjaxFixFieldLengthResultMapper.DoPutStream, "key [" << NotNull(key) << "]");
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
