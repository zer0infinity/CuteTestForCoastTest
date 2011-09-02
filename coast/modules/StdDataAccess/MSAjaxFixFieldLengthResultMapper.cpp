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
#include "AnyIterators.h"

namespace {
	char const *_FieldList = "Fields";
	char const *_FieldSeparator = "FieldSeparator";
	long const _lenMarker = 0L;
	char const _addMarker = '+';

	Anything getEntryFromString(String &sText, char const delim, long const nFields, ROAnything roaFieldList) {
		Anything anyEntry;
		long const _delimLen = 1;
		for ( long lIdx=0, delimPos=0; lIdx < nFields; ++lIdx) {
			delimPos = sText.StrChr(delim);
			anyEntry[roaFieldList.SlotName(lIdx)] = sText.SubString(0, delimPos);
			sText.TrimFront(delimPos+_delimLen);
		}
		return anyEntry;
	}
	long getStringLength(String const &str) {
		long len = 0L;
		try {
			len = utf8::distance(str.cstr(), str.cstr()+str.Length());
		} catch (utf8::invalid_utf8& e) {
			len = str.Length();
		}
		StatTrace(MSAjaxFixFieldLengthResultMapper.getStringLength, "len: " << len << " str [" << str << "]", Coast::Storage::Current());
		return len;
	}
	void adjustLengthField(Anything &anyEntry, ROAnything roaFieldList) {
		StartTrace(MSAjaxFixFieldLengthResultMapper.adjustLenghtField);
		long len = 0L;
		AnyExtensions::Iterator<ROAnything> fieldIterator(roaFieldList);
		ROAnything roaField;
		String strSlotname;
		long lenIdx = -1L;
		TraceAny(anyEntry, "unmodified entry");
		while ( fieldIterator.Next(roaField) ) {
			TraceAny(roaField, "current field");
			if ( roaField.IsNull() ) continue;
			if ( roaField.AsString()[0L] == _addMarker ) {
				if ( fieldIterator.SlotName(strSlotname) ) {
					len += getStringLength(anyEntry[strSlotname].AsString());
				} else {
					len += getStringLength(anyEntry[fieldIterator.Index()].AsString());
				}
			} else if ( roaField.AsLong(-1) == _lenMarker ) {
				lenIdx = fieldIterator.Index();
			}
		}
		if ( lenIdx >= 0L ) {
			anyEntry[lenIdx] = len;
		}
		TraceAny(anyEntry, "modified entry");
	}
	void appendEntryToString(String &strOut, char const delim, ROAnything roaEntry) {
		AnyExtensions::Iterator<ROAnything> fieldIterator(roaEntry);
		ROAnything roaField;
		while ( fieldIterator.Next(roaField) ) {
			strOut.Append(roaField.AsString()).Append(delim);
		}
	}
}

RegisterResultMapper(MSAjaxFixFieldLengthResultMapper);

bool MSAjaxFixFieldLengthResultMapper::DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script) {
	StartTrace1(MSAjaxFixFieldLengthResultMapper.DoPutAny, "key [" << NotNull(key) << "]");
	String sText = value.AsString();
	ROAnything roaFieldList;
	long nFields = 0L;
	if (Lookup(_FieldList, roaFieldList, '\000') && ( nFields = roaFieldList.GetSize() ) > 0L ) {
		TraceAny(roaFieldList, "field list");
		char const delim = Lookup(_FieldSeparator, "|")[0];
		String strOut;
		while ( sText.Length() ) {
			Anything anyEntry = getEntryFromString(sText, delim, nFields, roaFieldList);
			if ( anyEntry.GetSize() == nFields ) {
				adjustLengthField(anyEntry, roaFieldList);
				appendEntryToString(strOut, delim, anyEntry);
			}
		}
		value = strOut;
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
