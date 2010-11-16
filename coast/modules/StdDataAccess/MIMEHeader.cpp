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

MIMEHeader::MIMEHeader(Coast::URLUtils::NormalizeTag normalizeKey, MIMEHeader::ProcessMode splitHeaderFields) :
	fBoundaryChecked(false), fNormalizeKey(normalizeKey), fSplitHeaderFields(splitHeaderFields) {
	StartTrace(MIMEHeader.Ctor);
}

namespace Coast {
	namespace StreamUtils {
		bool getLineFromStream(std::istream &in, String &line, long const maxlinelen) {
			StartTrace(MIMEHeader.getLineFromStream);
			if (in.good()) {
				// reset line
				line.Trim(0L);
				// read line up to but not including next LF
				line.Append(in, maxlinelen, LF);
				Trace("Line length: " << line.Length() << " maxlinelen: " << maxlinelen << "\n" << line.DumpAsHex());
				char c = '\0';
				if (in.get(c).good()) {
					line.Append(c);
				}
				return true;
			}
			return false;
		}
	}
}

namespace {
	char const cookieArgumentsDelimiter = ';';
	char const headerArgumentsDelimiter = ',';
	char const headerNamedDelimiter = ':';
	//!split ';' seperated list of key=value pairs into anything
	Anything SplitLine(const String &line, Coast::URLUtils::NormalizeTag shift = Coast::URLUtils::eUpshift) {
		Anything values;
		Coast::URLUtils::Split(line, cookieArgumentsDelimiter, values, '=', shift);
		StatTraceAny(MIMEHeader.SplitLine, values, "input line [" << line << "] split into:", Storage::Current());
		return values;
	}
	long GetNormalizedFieldName(String const &line, String &fieldname, Coast::URLUtils::NormalizeTag const normTag) {
		StartTrace1(MIMEHeader.GetNormalizedFieldName, "Line: <<<" << line << ">>>");
		// following headerfield specification of HTTP/1.1 RFC 2068
		long pos = line.StrChr(headerNamedDelimiter);
		if (pos > 0) {
			fieldname = line.SubString(0, pos);
			Coast::URLUtils::Normalize(fieldname, normTag);
		}
		Trace("Fieldname: " << fieldname << " Position of " << headerNamedDelimiter << " is: " << pos);
		return pos;
	}
}

bool MIMEHeader::ParseHeaders(std::istream &in, long const maxlinelen, long const maxheaderlen) {
	StartTrace(MIMEHeader.ParseHeaders);
	String line(maxlinelen);
	long headerlength = 0;
	while (in.good() && Coast::StreamUtils::getLineFromStream(in, line, maxlinelen)) {
		headerlength += line.Length();
		Trace("headerlength: " << headerlength << ", maxheaderlen: " << maxheaderlen);
		if (not Coast::URLUtils::TrimENDL(line).Length()) {
			return true;	//!< successful header termination with empty line
		}
		if (line.Length() > maxlinelen || headerlength > maxheaderlen || not ParseField(line, fNormalizeKey)) {
			return false;
		}
	}
	return false;
}

bool MIMEHeader::ParseHeaderLine(String &line) {
	StartTrace1(MIMEHeader.ParseHeaderLine, "Line: <<" << line << ">>");
	return ParseField(line, fNormalizeKey);
}

namespace {
	MIMEHeader::ProcessMode GetSplitState(String fieldname, MIMEHeader::ProcessMode splitHeaderFields) {
		StartTrace(MIMEHeader.GetSplitState);
		if (fieldname.ToUpper().IsEqual("SET-COOKIE") || fieldname.IsEqual("USER-AGENT")) {
			splitHeaderFields = MIMEHeader::eDoNotSplitHeaderFields;
		} else if (fieldname.IsEqual("COOKIE")) {
			splitHeaderFields = MIMEHeader::eDoSplitHeaderFieldsCookie;
		}
		return splitHeaderFields;
	}
}

bool MIMEHeader::ParseField(String const &line, Coast::URLUtils::NormalizeTag const normTag)
{
	StartTrace1(MIMEHeader.ParseField, "Line: <<<" << line << ">>>");
	// following headerfield specification of HTTP/1.1 RFC 2068
	String fieldname;
	long pos = GetNormalizedFieldName(line, fieldname, normTag);
	if ( pos > 0 ) {
		String fieldvalue;
		MIMEHeader::ProcessMode splitHeaderFields = GetSplitState(fieldname, fSplitHeaderFields);
		if ( (splitHeaderFields == eDoSplitHeaderFields) || (splitHeaderFields == eDoSplitHeaderFieldsCookie) ) {
			StringTokenizer st1(((const char *)line) + pos + 1, (splitHeaderFields == eDoSplitHeaderFields ? headerArgumentsDelimiter : cookieArgumentsDelimiter));
			while (st1.NextToken(fieldvalue)) {
				Trace("fieldname: [" << fieldname << "] fieldvalue: [" << fieldvalue << "]");
				if ( fieldvalue.Length() ) {
					Coast::URLUtils::TrimBlanks(fieldvalue);
					Coast::URLUtils::RemoveQuotes(fieldvalue);
					if ( splitHeaderFields == eDoSplitHeaderFields ) {
						if (String(fieldname).ToUpper().IsEqual("CONTENT-DISPOSITION")) {
							fHeader[fieldname] = SplitLine(fieldvalue);
						} else {
							Coast::URLUtils::AppendValueTo(fHeader, fieldname, fieldvalue);
						}
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
			}
		}
		TraceAny(fHeader, "fHeader on exit");
		return true;
	}
	return false;
}

//================= multipart specific section

namespace {
	const char *boundaryTag = "BOUNDARY";
	void CheckMultipartBoundary(const String &contenttype, String &boundary, Anything &header) {
		StartTrace1(MIMEHeader.CheckMultipartBoundary, "Content-type: <" << contenttype << ">");
		if ((contenttype.Length() >= 9) && (contenttype.Contains("multipart") != -1)) {
			long index = contenttype.Contains("boundary=");
			if (index > 0) {
				boundary = contenttype.SubString(index + 9);
				header[boundaryTag] = boundary;
				Trace("Multipart boundary found: <" << boundary << ">");
			}
		}
	}
}

bool MIMEHeader::IsMultiPart()
{
	StartTrace(MIMEHeader.IsMultiPart);
	if (!fBoundaryChecked) {
		CheckMultipartBoundary(Lookup("CONTENT-TYPE", ""), fBoundary, fHeader);
		fBoundaryChecked = true;
	}
	return (fBoundary.Length() > 0);
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
