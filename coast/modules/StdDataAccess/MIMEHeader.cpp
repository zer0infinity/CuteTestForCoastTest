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
#include <cstdio>	// for EOF

MIMEHeader::MIMEHeader(Coast::URLUtils::NormalizeTag normalizeKey, MIMEHeader::ProcessMode splitHeaderFields) :
	fNormalizeKey(normalizeKey), fSplitHeaderFields(splitHeaderFields) {
	StartTrace(MIMEHeader.Ctor);
}

namespace Coast {
	namespace StreamUtils {
		void getLineFromStream(std::istream &in, String &line, long const maxlinelen) {
			StartTrace(MIMEHeader.getLineFromStream);
			if ( !in.good() ) throw MIMEHeader::StreamNotGoodException();
			// reset line
			line.Trim(0L);
			// read line up to but not including next LF
			line.Append(in, maxlinelen, LF);
			Trace("Line length: " << line.Length() << " maxlinelen: " << maxlinelen << "\n" << line.DumpAsHex());
			char c = '\0';
			if ( in.peek() != EOF && in.get(c)) {
				line.Append(c);
			}
			if (line.Length() > maxlinelen) throw MIMEHeader::LineSizeExceededException("Line size exceeded", line, maxlinelen, line.Length());
		}
	}
}

namespace {
	char const *boundarySlotname = "BOUNDARY";
	char const *contentDispositionSlotname = "CONTENT-DISPOSITION";
	char const *contentLengthSlotname = "CONTENT-LENGTH";
	char const *contentTypeSlotname = "CONTENT-TYPE";
	char const *cookieSlotname = "COOKIE";
	char const *setCookieSlotname = "SET-COOKIE";
	char const *userAgentSlotname = "USER-AGENT";
	String const boundaryToken("boundary=", Coast::Storage::Global());

	char const cookieArgumentsDelimiter = ';';
	char const headerArgumentsDelimiter = ',';
	char const headerNamedDelimiter = ':';

	String shiftedHeaderKey(String const &key, Coast::URLUtils::NormalizeTag const shiftFlag) {
		return Coast::URLUtils::Normalize(key, shiftFlag);
	}
	//!split ';' seperated list of key=value pairs into anything
	Anything SplitLine(const String &line, Coast::URLUtils::NormalizeTag const shift = Coast::URLUtils::eUpshift) {
		Anything values;
		Coast::URLUtils::Split(line, cookieArgumentsDelimiter, values, '=', shift);
		StatTraceAny(MIMEHeader.SplitLine, values, "input line [" << line << "] split into:", Coast::Storage::Current());
		return values;
	}
	long GetNormalizedFieldName(String const &line, String &fieldname, Coast::URLUtils::NormalizeTag const normTag) {
		StartTrace1(MIMEHeader.GetNormalizedFieldName, "Line: <<<" << line << ">>>");
		// following headerfield specification of HTTP/1.1 RFC 2068
		long pos = line.StrChr(headerNamedDelimiter);
		if (pos > 0) {
			fieldname = shiftedHeaderKey(line.SubString(0, pos), normTag);
		} Trace("Fieldname: " << fieldname << " Position of " << headerNamedDelimiter << " is: " << pos);
		return pos;
	}

	MIMEHeader::ProcessMode GetSplitState(String const &fieldname, MIMEHeader::ProcessMode splitHeaderFields,
			Coast::URLUtils::NormalizeTag const shiftFlag) {
		StartTrace(MIMEHeader.GetSplitState);
		if (fieldname.IsEqual(shiftedHeaderKey(setCookieSlotname, shiftFlag)) || fieldname.IsEqual(shiftedHeaderKey(userAgentSlotname,
				shiftFlag))) {
			splitHeaderFields = MIMEHeader::eDoNotSplitHeaderFields;
		} else if (fieldname.IsEqual(shiftedHeaderKey(cookieSlotname, shiftFlag))) {
			splitHeaderFields = MIMEHeader::eDoSplitHeaderFieldsCookie;
		}
		return splitHeaderFields;
	}

	void CheckMultipartBoundary(String const &contenttype, Anything &header, Coast::URLUtils::NormalizeTag const shiftFlag) {
		StartTrace1(MIMEHeader.CheckMultipartBoundary, "Content-type: <" << contenttype << ">");
		if ((contenttype.Length() >= 9) && (contenttype.Contains("multipart") != -1)) {
			long index = contenttype.Contains(boundaryToken.cstr());
			if (index > 0) {
				String strBoundary = contenttype.SubString(index + boundaryToken.Length());
				header[shiftedHeaderKey(boundarySlotname, shiftFlag)] = strBoundary;
				Trace("Multipart boundary found: <" << strBoundary << ">");
			}
		}
	}

	void SplitAndAddHeaderLine(Anything &headers, String const &line, MIMEHeader::ProcessMode splitHeaderFields,
			Coast::URLUtils::NormalizeTag const normTag) {
		StartTrace1(MIMEHeader.SplitAndAddHeaderLine, "Line: <<<" << line << ">>>");
		// following headerfield specification of HTTP/1.1 RFC 2616 (obsoletes 2068)
		String fieldname;
		long pos = GetNormalizedFieldName(line, fieldname, normTag);
		if (pos <= 0) {
			throw MIMEHeader::InvalidLineException("Missing header field name", line);
		}
		String fieldvalue;
		splitHeaderFields = GetSplitState(fieldname, splitHeaderFields, normTag);
		if ((splitHeaderFields == MIMEHeader::eDoSplitHeaderFields) || (splitHeaderFields == MIMEHeader::eDoSplitHeaderFieldsCookie)) {
			StringTokenizer st1(line.cstr() + pos + 1,
					(splitHeaderFields == MIMEHeader::eDoSplitHeaderFields ? headerArgumentsDelimiter : cookieArgumentsDelimiter));
			while (st1.NextToken(fieldvalue)) {
				Trace("fieldname: [" << fieldname << "] fieldvalue: [" << fieldvalue << "]");
				if (fieldvalue.Length()) {
					Coast::URLUtils::TrimBlanks(fieldvalue);
					Coast::URLUtils::RemoveQuotes(fieldvalue);
					if (splitHeaderFields == MIMEHeader::eDoSplitHeaderFields) {
						if (fieldname.IsEqual(shiftedHeaderKey(contentDispositionSlotname, normTag))) {
							headers[fieldname] = SplitLine(fieldvalue);
						} else {
							Coast::URLUtils::AppendValueTo(headers, fieldname, fieldvalue);
						}
					} else if (splitHeaderFields == MIMEHeader::eDoSplitHeaderFieldsCookie) {
						Anything tmp;
						tmp = SplitLine(fieldvalue, Coast::URLUtils::eUntouched);
						for (int i = 0; i < tmp.GetSize(); i++) {
							headers[fieldname][tmp.SlotName(i)] = tmp[i];
						}
					}
				}
			}
		}
		if (splitHeaderFields == MIMEHeader::eDoNotSplitHeaderFields) {
			fieldvalue = line.SubString(pos + 1);
			if (fieldvalue.Length()) {
				Coast::URLUtils::TrimBlanks(fieldvalue);
				Coast::URLUtils::AppendValueTo(headers, fieldname, fieldvalue);
			}
		}
		if (fieldname.IsEqual(shiftedHeaderKey(contentTypeSlotname, normTag))) {
			CheckMultipartBoundary(fieldvalue, headers, normTag);
		}
		TraceAny(headers, "headers on exit");
	}
}

bool MIMEHeader::ParseHeaders(std::istream &in, long const maxlinelen, long const maxheaderlen) {
	StartTrace(MIMEHeader.ParseHeaders);
	String line(maxlinelen);
	long headerlength = 0;
	while ( true ) {
		Coast::StreamUtils::getLineFromStream(in, line, maxlinelen);
		headerlength += line.Length();
		Trace("headerlength: " << headerlength << ", maxheaderlen: " << maxheaderlen);
		if (not Coast::URLUtils::TrimENDL(line).Length()) {
			return true;	//!< successful header termination with empty line
		}
		if (headerlength > maxheaderlen) throw MIMEHeader::HeaderSizeExceededException("Header size exceeded", line, maxheaderlen, headerlength);
		SplitAndAddHeaderLine(fHeader, line, fSplitHeaderFields, fNormalizeKey);
	}
	return false;
}


//================= multipart specific section

bool MIMEHeader::IsMultiPart() const {
	StartTrace(MIMEHeader.IsMultiPart);
	return (Lookup(boundarySlotname).AsString().Length() > 0);
}

String MIMEHeader::GetBoundary() const {
	String boundary = Lookup(boundarySlotname).AsString();
	StatTrace(MIMEHeader.GetBoundary, boundaryToken << boundary, Coast::Storage::Current());
	return boundary;
}

long MIMEHeader::GetContentLength() const {
	long contentLength = Lookup(contentLengthSlotname).AsLong(-1L);
	StatTrace(MIMEHeader.GetContentLength, "length: " << contentLength, Coast::Storage::Current());
	return contentLength;
}

bool MIMEHeader::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(MIMEHeader.DoLookup, "key: <" << NotNull(key) << ">" );
	return ROAnything(fHeader).LookupPath(result, shiftedHeaderKey(key, fNormalizeKey), delim, indexdelim);
}
