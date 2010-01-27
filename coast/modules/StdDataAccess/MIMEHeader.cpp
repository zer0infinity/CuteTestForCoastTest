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
#include "StringStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

MIMEHeader::MIMEHeader(URLUtils::NormalizeTag normalizeKey, MIMEHeader::ProcessMode splitHeaderFields)
	: fBoundaryChecked(false)
	, fNormalizeKey(normalizeKey)
	, fSplitHeaderFields(splitHeaderFields)
	, fAreSuspiciosHeadersPresent(false)
{
	StartTrace(MIMEHeader.Ctor);
}

bool MIMEHeader::DoReadHeader(istream &in, long maxlinelen, long maxheaderlen)
{
	StartTrace(MIMEHeader.DoReadHeader);
	String line;
	long headerlength = 0;

	while (in.good()) {
		// reset line
		line = "";
		// read line up to but not including next \n
		line.Append(in, maxlinelen + 2, '\n');
		Trace("Line [" << line.Length() << "] maxlinelen [" << maxlinelen << "]\nLineContent: <" << line << "\n>");
		headerlength += line.Length();
		Trace("headerlength [" << headerlength << "]  [maxheaderlen] " << maxheaderlen);
		if ((line.Length() > maxlinelen) 	||
			(headerlength > maxheaderlen)   ||
			(!ConsumeEOL(in)) 				||
			((line != "\r") &&  (!DoParseHeaderLine(line)) )) {
			// bail out we got an error
			return false;
		}

		// we are done with the header
		if (line == "\r" || line == "") {
			return true;
		}
	}
	return true;
}

MIMEHeader::ProcessMode MIMEHeader::GetDoSplitHeaderFieldsState(const String &fieldNameUpperCase)
{
	StartTrace(MIMEHeader.GetDoSplitHeaderFieldsState);
	MIMEHeader::ProcessMode splitHeaderFields;
	if ( fieldNameUpperCase == "SET-COOKIE" ) {
		splitHeaderFields = eDoNotSplitHeaderFields;
	} else if ( fieldNameUpperCase == "COOKIE" ) {
		splitHeaderFields = eDoSplitHeaderFieldsCookie;
	} else {
		splitHeaderFields = fSplitHeaderFields;
	}
	return splitHeaderFields;
}

bool MIMEHeader::DoParseHeaderLine(String &line)
{
	StartTrace1(MIMEHeader.DoParseHeaderLine, "Line: <<" << line << "\n>>");

	TrimEOL(line);
	String fieldname;
	GetNormalizedFieldName(line, fieldname);
	String fieldNameUpperCase(fieldname);
	fieldNameUpperCase.ToUpper();
	ParseField(line, GetDoSplitHeaderFieldsState(fieldNameUpperCase));
	Trace("My marker...");
	if (fieldNameUpperCase == "CONTENT-DISPOSITION" ) {
		if ( fSplitHeaderFields == eDoSplitHeaderFields ) {
			fHeader[fieldname] = SplitLine(fHeader[fieldname].AsCharPtr());
		}
	}
	TraceAny(fHeader, "fHeader on exit");
	return true;
}

Anything MIMEHeader::SplitLine(const String &line, URLUtils::NormalizeTag shift)
{
	StartTrace1(MIMEHeader.SplitLine, "Line: " << line);
	Anything values;

	URLUtils::Split(line, ';', values, '=', shift);

	TraceAny(values, "values: ");
	return values;
}

long MIMEHeader::GetNormalizedFieldName(String &line, String &fieldname)
{
	StartTrace1(MIMEHeader.GetNormalizedFieldName, "Line: <<<" << line << ">>>");
	// following headerfield specification of HTTP/1.1 RFC 2068
	long pos = 0;
	pos = line.StrChr(':');
	if (pos > 0) {
		fieldname = line.SubString(0, pos);
		Normalize(fieldname);
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
			StringTokenizer st1(((const char *)line) + pos + 1, (splitHeaderFields == eDoSplitHeaderFields ? ',' : ';'));
			while (st1.NextToken(fieldvalue)) {
				Trace("fieldname: [" << fieldname << "] fieldvalue: [" << fieldvalue << "]");
				if ( fieldvalue.Length() ) {
					URLUtils::TrimBlanks(fieldvalue);
					URLUtils::RemoveQuotes(fieldvalue);
					CheckValues(fieldvalue);
					if ( splitHeaderFields == eDoSplitHeaderFields ) {
						URLUtils::AppendValueTo(fHeader, fieldname, fieldvalue);
					} else if ( splitHeaderFields == eDoSplitHeaderFieldsCookie ) {
						Anything tmp;
						tmp = SplitLine(fieldvalue, URLUtils::eUntouched);
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
				URLUtils::TrimBlanks(fieldvalue);
				CheckValues(fieldvalue);
				URLUtils::AppendValueTo(fHeader, fieldname, fieldvalue);
				TraceAny(fHeader, "fHeader");
			}
		}
	} else {
		return false;
	}

	return (fieldname.Length() > 0);
}

bool MIMEHeader::CheckValues(String &value)
{
	StartTrace(MIMEHeader.CheckValues);
	String work(value);
	work.ToUpper();
	bool ret = work.StartsWith("GET") || work.StartsWith("POST");
	if (ret) {
		fAreSuspiciosHeadersPresent = true;
	}
	return ret;
}

bool MIMEHeader::AreSuspiciosHeadersPresent()
{
	StartTrace(MIMEHeader.AreSuspiciosHeadersPresent);
	return fAreSuspiciosHeadersPresent;
}

void MIMEHeader::SetAreSuspiciosHeadersPresent(bool newValue)
{
	StartTrace(MIMEHeader.SetAreSuspiciosHeadersPresent);
	fAreSuspiciosHeadersPresent = newValue;
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

const String &MIMEHeader::GetBoundary()
{
	StartTrace(MIMEHeader.GetBoundary);
	return fBoundary;
}

long MIMEHeader::GetContentLength()
{
	StartTrace(MIMEHeader.GetContentLength);
	// inhibit side-effect if not set
	return ROAnything(fHeader)["CONTENT-LENGTH"].AsLong(-1);
}

bool MIMEHeader::ConsumeEOL(istream &in) const
{
	StartTrace(MIMEHeader.ConsumeEOL);

	if ((in.peek() == '\n') && in.good()) {
		in.get();
		Trace("leaving with true");
		return true;
	}
	Trace("leaving with false");
	return false;
}

void MIMEHeader::TrimEOL(String &line) const
{
	// we cut off \r\n at end of line
	long llen = line.Length();
	if (line[(long)(llen-2)] == '\r' && line[(long)(llen-1)] == '\n') {
		line.Trim(llen - 2);
	} else if (line[(long)(llen-1)] == '\r') {
		line.Trim(llen - 1);
	}
}

void MIMEHeader::Normalize(String &str) const
{
	URLUtils::Normalize(str, fNormalizeKey);
}

bool MIMEHeader::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const
{
	StartTrace1(MIMEHeader.DoLookup, "key: <" << NotNull(key) << ">" );
	String normKey(key);
	Normalize(normKey);
	return ROAnything(fHeader).LookupPath(result, normKey, delim, indexdelim);
}
