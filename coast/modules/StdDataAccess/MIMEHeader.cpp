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

MIMEHeader::MIMEHeader(MIMEHeader *sup, URLUtils::NormalizeTag normalizeKey, MIMEHeader::ProcessMode splitHeaderFields)
	:	fSuper(sup),
		fBoundaryChecked(false),
		fNormalizeKey(normalizeKey),
		fSplitHeaderFields(splitHeaderFields)
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

bool MIMEHeader::DoParseHeaderLine(String &line)
{
	StartTrace1(MIMEHeader.DoParseHeaderLine, "Line: <<" << line << "\n>>");

	TrimEOL(line);
	String fieldname;
	ParseField(line, fieldname);
	String fieldNameUpperCase(fieldname);
	fieldNameUpperCase.ToUpper();
	if (fieldNameUpperCase == "CONTENT-DISPOSITION" ) {
		if ( fSplitHeaderFields == eDoSplitHeaderFields ) {
			fHeader[fieldname] = SplitLine(fHeader[fieldname].AsCharPtr());
		}
	} else if (fieldNameUpperCase == "COOKIE" ) {
		fHeader[fieldname] = SplitLine(fHeader[fieldname].AsCharPtr(), URLUtils::eUntouched);
	}
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

bool MIMEHeader::ParseField(String &line, String &fieldname)
{
	StartTrace1(MIMEHeader.ParseField, "Line: <<<" << line << ">>>");
	// following headerfield specification of HTTP/1.1 RFC 2068
	long pos = 0;
	pos = line.StrChr(':');
	if (pos > 0) {
		fieldname = line.SubString(0, pos);
		Normalize(fieldname);
		String fieldvalue;
		if ( fSplitHeaderFields == eDoSplitHeaderFields ) {
			StringTokenizer st1(((const char *)line) + pos + 1, ',');
			while (st1.NextToken(fieldvalue)) {
				if ( fieldvalue.Length() ) {
					URLUtils::TrimBlanks(fieldvalue);
					URLUtils::RemoveQuotes(fieldvalue);
					URLUtils::AppendValueTo(fHeader, fieldname, fieldvalue);
				}
			}
		}
		if ( fSplitHeaderFields == eDoNotSplitHeaderFields ) {
			fieldvalue = line.SubString(pos + 1);
			if ( fieldvalue.Length() ) {
				URLUtils::TrimBlanks(fieldvalue);
				URLUtils::AppendValueTo(fHeader, fieldname, fieldvalue);
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
