/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MIMEHeader.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include <iostream>

namespace {
	char const cookieArgumentsDelimiter = ';';
	char const headerArgumentsDelimiter = ',';
	//!split ';' seperated list of key=value pairs into anything
	Anything SplitLine(const String &line, Coast::URLUtils::NormalizeTag shift = Coast::URLUtils::eUpshift) {
		Anything values;
		Coast::URLUtils::Split(line, cookieArgumentsDelimiter, values, '=', shift);
		StatTraceAny(MIMEHeader.SplitLine, values, "input line [" << line << "] split into:", Storage::Current());
		return values;
	}
}

MIMEHeader::MIMEHeader(Coast::URLUtils::NormalizeTag normalizeKey, MIMEHeader::ProcessMode splitHeaderFields)
	: fBoundaryChecked(false)
	, fNormalizeKey(normalizeKey)
	, fSplitHeaderFields(splitHeaderFields)
{
	StartTrace(MIMEHeader.Ctor);
}

bool MIMEHeader::DoReadHeader(std::istream &in, long const maxlinelen, long const maxheaderlen)
{
	StartTrace(MIMEHeader.DoReadHeader);
	char const LF = '\n';
	String line(maxlinelen);
	long headerlength = 0;
	char c = '\0';
	while (in.good()) {
		// reset line
		line.Trim(0L);
		// read line up to but not including next LF
		line.Append(in, maxlinelen, LF);
		if ( !in.get(c).good() || c != LF ) {
			return false;
		}
		Trace("Line length: " << line.Length() << " maxlinelen: " << maxlinelen << "\n" << line.DumpAsHex());
		headerlength += line.Length();
		Trace("headerlength: " << headerlength << ", maxheaderlen: " << maxheaderlen);
		if ( not Coast::URLUtils::TrimENDL(line).Length() ) return true;
		if (line.Length() > maxlinelen || headerlength > maxheaderlen) {
			return false;
		}
		if (not DoParseHeaderLine(line)) {
			return false;
		}
	}
	return true;
}

MIMEHeader::ProcessMode MIMEHeader::GetDoSplitHeaderFieldsState(const String &fieldNameUpperCase) const
{
	StartTrace(MIMEHeader.GetDoSplitHeaderFieldsState);
	MIMEHeader::ProcessMode splitHeaderFields = fSplitHeaderFields;
	if ( fieldNameUpperCase.IsEqual("SET-COOKIE") ) {
		splitHeaderFields = eDoNotSplitHeaderFields;
	} else if ( fieldNameUpperCase.IsEqual("COOKIE") ) {
		splitHeaderFields = eDoSplitHeaderFieldsCookie;
	}
	return splitHeaderFields;
}

bool MIMEHeader::DoParseHeaderLine(String &line)
{
	StartTrace1(MIMEHeader.DoParseHeaderLine, "Line: <<" << line << ">>");
	String fieldname;
	GetNormalizedFieldName(line, fieldname);
	String fieldNameUpperCase(fieldname);
	fieldNameUpperCase.ToUpper();
	ParseField(line, GetDoSplitHeaderFieldsState(fieldNameUpperCase));
	Trace("My marker...");
	if (fieldNameUpperCase.IsEqual("CONTENT-DISPOSITION") ) {
		if ( fSplitHeaderFields == eDoSplitHeaderFields ) {
			fHeader[fieldname] = SplitLine(fHeader[fieldname].AsString());
		}
	}
	TraceAny(fHeader, "fHeader on exit");
	return true;
}

long MIMEHeader::GetNormalizedFieldName(String &line, String &fieldname) const
{
	StartTrace1(MIMEHeader.GetNormalizedFieldName, "Line: <<<" << line << ">>>");
	// following headerfield specification of HTTP/1.1 RFC 2068
	long pos = 0;
	pos = line.StrChr(':');
	if (pos > 0) {
		fieldname = line.SubString(0, pos);
		Coast::URLUtils::Normalize(fieldname, fNormalizeKey);
	}
	Trace("Fieldname: " << fieldname << " Position of ':' is: " << pos);
	return pos;
}

bool MIMEHeader::ParseField(String &line, MIMEHeader::ProcessMode splitHeaderFields)
{
	StartTrace1(MIMEHeader.ParseField, "Line: <<<" << line << ">>>");
	// following headerfield specification of HTTP/1.1 RFC 2068
	String fieldname;
	long pos = GetNormalizedFieldName(line, fieldname);
	if ( pos > 0 ) {
		String fieldvalue;
		if ( (splitHeaderFields == eDoSplitHeaderFields) || (splitHeaderFields == eDoSplitHeaderFieldsCookie) ) {
			StringTokenizer st1(((const char *)line) + pos + 1, (splitHeaderFields == eDoSplitHeaderFields ? headerArgumentsDelimiter : cookieArgumentsDelimiter));
			while (st1.NextToken(fieldvalue)) {
				Trace("fieldname: [" << fieldname << "] fieldvalue: [" << fieldvalue << "]");
				if ( fieldvalue.Length() ) {
					Coast::URLUtils::TrimBlanks(fieldvalue);
					Coast::URLUtils::RemoveQuotes(fieldvalue);
					if ( splitHeaderFields == eDoSplitHeaderFields ) {
						Coast::URLUtils::AppendValueTo(fHeader, fieldname, fieldvalue);
					} else if ( splitHeaderFields == eDoSplitHeaderFieldsCookie ) {
						Anything tmp;
						tmp = SplitLine(fieldvalue, Coast::URLUtils::eUntouched);
						for (int i = 0; i < tmp.GetSize(); i++ ) {
							fHeader[fieldname][tmp.SlotName(i)] = tmp[i];
						}
					}
				}
			}
		}
		if ( splitHeaderFields == eDoNotSplitHeaderFields ) {
			fieldvalue = line.SubString(pos + 1);
			if ( fieldvalue.Length() ) {
				Coast::URLUtils::TrimBlanks(fieldvalue);
				Coast::URLUtils::AppendValueTo(fHeader, fieldname, fieldvalue);
				TraceAny(fHeader, "fHeader");
			}
		}
	} else {
		return false;
	}

	return (fieldname.Length() > 0);
}

bool MIMEHeader::IsMultiPart()
{
	StartTrace(MIMEHeader.IsMultiPart);
	if (!fBoundaryChecked) {
		CheckMultipartBoundary(Lookup("CONTENT-TYPE", ""));
		fBoundaryChecked = true;
	}
	return (fBoundary.Length() > 0);
}

void MIMEHeader::CheckMultipartBoundary(const String &contenttype)
{
	StartTrace1(MIMEHeader.CheckMultipartBoundary, "Content-type: <" << contenttype << ">");
	if ((contenttype.Length() >= 9) && (contenttype.Contains("multipart") != -1)) {
		const char *boundaryTag = "BOUNDARY";
		long index = contenttype.Contains("boundary=");
		if (index > 0) {
			fBoundary = contenttype.SubString(index + 9);
			fHeader[boundaryTag] = fBoundary;
			Trace("Multipart boundary found: <" << fBoundary << ">");
		}
	}
}

const String &MIMEHeader::GetBoundary() const
{
	StartTrace(MIMEHeader.GetBoundary);
	return fBoundary;
}

long MIMEHeader::GetContentLength() const
{
	StatTrace(MIMEHeader.GetContentLength, "length: " << ROAnything(fHeader)["CONTENT-LENGTH"].AsLong(-1), Storage::Current());
	// inhibit side-effect if not set
	return ROAnything(fHeader)["CONTENT-LENGTH"].AsLong(-1);
}

bool MIMEHeader::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(MIMEHeader.DoLookup, "key: <" << NotNull(key) << ">" );
	String normKey(key);
	Coast::URLUtils::Normalize(normKey, fNormalizeKey);
	return ROAnything(fHeader).LookupPath(result, normKey, delim, indexdelim);
}
