/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MIMEHeader.h"
#include "Tracer.h"
#include "RE.h"
#include "HTTPConstants.h"
//#include "RECompiler.h"
#include <cstdio>	// for EOF
#include <cstring>  // for strlen

namespace coast {
	namespace streamutils {
		void getLineFromStream(std::istream &in, String &line, long const maxlinelen) {
			StartTrace(MIMEHeader.getLineFromStream);
			if ( !in.good() ) throw MIMEHeader::StreamNotGoodException();
			// reset line
			line.Trim(0L);
			// read line up to but not including next LF
			line.Append(in, maxlinelen, LF);
			char c = '\0';
			if ( in.peek() != EOF && in.get(c)) {
				line.Append(c);
			}
			Trace("Line length: " << line.Length() << " maxlinelen: " << maxlinelen << "\n" << line.DumpAsHex());
			if (line.Length() > maxlinelen) throw MIMEHeader::LineSizeExceededException("Line size exceeded", line, maxlinelen, line.Length());
		}
	}
}

namespace {
	char const * const boundarySlotname = "BOUNDARY";
	char const * const contentLengthSlotname = "CONTENT-LENGTH";
	char const * const contentDispositionSlotname = "CONTENT-DISPOSITION";
	char const * const contentTypeSlotname = "CONTENT-TYPE";
	String const boundaryToken("boundary=", -1, coast::storage::Global());
	char const headerNamedDelimiter = ':';

	char const contentDispositionDelimiter = ';';
	char const headerArgumentsDelimiter = ',';
	char const valueArgumentDelimiter = '=';

//	Anything const headersREProgram = RECompiler().compile(coast::http::constants::splitFieldsRegularExpression);

	void StoreKeyValue(Anything &headers, String const& strKey, String const &strValue)
	{
		StartTrace(MIMEHeader.StoreKeyValue);
		//!@FIXME: use precompiled RE-Program as soon as RE's ctor takes an ROAnything as program
		RE multivalueRE(coast::http::constants::splitFieldsRegularExpression, RE::MATCH_ICASE);
		if ( multivalueRE.ContainedIn(strKey) ) {
			Anything &anyValues = headers[strKey];
			coast::urlutils::Split(strValue, headerArgumentsDelimiter, anyValues, headerArgumentsDelimiter, coast::urlutils::eUpshift);
		} else if ( strKey.IsEqual(contentDispositionSlotname)) {
			Anything &anyValues = headers[strKey];
			coast::urlutils::Split(strValue, contentDispositionDelimiter, anyValues, valueArgumentDelimiter, coast::urlutils::eUpshift);
		} else {
			coast::urlutils::AppendValueTo(headers, strKey, strValue);
		}
	}

	String shiftedHeaderKey(String const &key, coast::urlutils::NormalizeTag const shiftFlag) {
		return coast::urlutils::Normalize(key, shiftFlag);
	}

	void CheckMultipartBoundary(String const &contenttype, Anything &header, coast::urlutils::NormalizeTag const shiftFlag) {
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

	long GetNormalizedFieldName(String const &line, String &fieldname, coast::urlutils::NormalizeTag const normTag) {
		StartTrace1(MIMEHeader.GetNormalizedFieldName, "Line: <<<" << line << ">>>");
		// following headerfield specification of HTTP/1.1 RFC 2068
		long pos = line.StrChr(headerNamedDelimiter);
		if (pos > 0) {
			fieldname = shiftedHeaderKey(line.SubString(0, pos), normTag);
		} Trace("Fieldname: " << fieldname << " Position of " << headerNamedDelimiter << " is: " << pos);
		return pos;
	}

	void SplitAndAddHeaderLine(Anything &headers, String const &line, coast::urlutils::NormalizeTag const normTag) {
		StartTrace1(MIMEHeader.SplitAndAddHeaderLine, "Line: <<<" << line << ">>>");
		// following headerfield specification of HTTP/1.1 RFC 2616 (obsoletes 2068)
		String fieldname;
		long pos = GetNormalizedFieldName(line, fieldname, normTag);
		if (pos <= 0) {
			throw MIMEHeader::InvalidLineException("Missing header field name", line);
		}
		String fieldvalue = line.SubString(pos + 1);
		if (fieldvalue.Length()) {
			coast::urlutils::TrimBlanks(fieldvalue);
			StoreKeyValue(headers, fieldname, fieldvalue);
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
	while ( true ) {
		coast::streamutils::getLineFromStream(in, line, maxlinelen);
		fParsedHeaderLength += line.Length();
		Trace("headerlength: " << fParsedHeaderLength << ", maxheaderlen: " << maxheaderlen);
		if (not coast::urlutils::TrimENDL(line).Length()) {
			return true;	//!< successful header termination with empty line
		}
		if (fParsedHeaderLength > maxheaderlen) throw MIMEHeader::RequestSizeExceededException("Header size exceeded", line, maxheaderlen, fParsedHeaderLength);
		SplitAndAddHeaderLine(fHeader, line, fNormalizeKey);
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
	StatTrace(MIMEHeader.GetBoundary, boundaryToken << boundary, coast::storage::Current());
	return boundary;
}

long MIMEHeader::GetContentLength() const {
	long contentLength = Lookup(contentLengthSlotname).AsLong(-1L);
	StatTrace(MIMEHeader.GetContentLength, "length: " << contentLength, coast::storage::Current());
	return contentLength;
}

bool MIMEHeader::DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const {
	StartTrace1(MIMEHeader.DoLookup, "key: <" << NotNull(key) << ">" );
	return ROAnything(fHeader).LookupPath(result, shiftedHeaderKey(key, fNormalizeKey), delim, indexdelim);
}
