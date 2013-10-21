/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HTTPPostRequestBodyParser.h"
#include "StringStream.h"
#include "MIMEHeader.h"
#include "Tracer.h"

bool HTTPPostRequestBodyParser::Parse(std::istream &input) {
	StartTrace(HTTPPostRequestBodyParser.Parse);
	TraceAny(fHeader.GetInfo(), "Header: ");
	if (fHeader.IsMultiPart()) {
		Trace("Multipart detected");
		return ParseMultiPart(input, fHeader.GetBoundary());
	} else {
		Trace("Parsing simple body");
		return ParseBody(input);
	}
}

bool HTTPPostRequestBodyParser::ParseBody(std::istream &input) {
	StartTrace(HTTPPostRequestBodyParser.ParseBody);
	ROAnything contenttype;
	if (fHeader.Lookup("CONTENT-TYPE", contenttype) && contenttype.AsString().Contains(coast::http::contentTypeAnything) != -1) {
		// there must be exactly one anything in the body handle our special format more efficient than the standard cases
		Anything a;
		if (not a.Import(input) || a.IsNull()) {
			Trace("" << coast::http::contentTypeAnything << " - syntax error");
			return false;
		}
		TraceAny(a, "Content-type: " << contenttype.AsString());
		fContent.Append(a);
		return true;
	}
	// 2 cases: content length defined, or everything
	bool readSuccess = false;
	long contentLength = fHeader.GetContentLength();
	if (contentLength >= 0) {
		Trace("content length: " << contentLength);
		fUnparsedContent.Append(input, contentLength);
		readSuccess = (fUnparsedContent.Length() == contentLength);
	} else {
		Trace("get everything until eof()");
		while (input.good()) {
			fUnparsedContent.Append(input, 4096);
		}
		readSuccess = input.eof();
	}
	Trace("Body: <" << fUnparsedContent << ">");
	ROAnything contentdisp;
	String name, filename;
	if (fHeader.Lookup("CONTENT-DISPOSITION", contentdisp) && contentdisp.Contains("form-data")) {
		name = contentdisp["NAME"].AsString();
		filename = contentdisp["FILENAME"].AsString();
	}
	if (name.Length() && filename.Length()) {
		fContent.Append(fUnparsedContent);
	} else if (fUnparsedContent.Length()) {
		Decode(fUnparsedContent, fContent);
	}
	return readSuccess;
}

void HTTPPostRequestBodyParser::Decode(String str, Anything &result) {
	StartTrace(HTTPPostRequestBodyParser.Decode);
	coast::urlutils::Split(coast::urlutils::TrimENDL(str), '&', result);
	coast::urlutils::DecodeAll(result);
}

bool HTTPPostRequestBodyParser::ReadToBoundary(std::istream &input, const String &bound, String &body) {
	StartTrace1(HTTPPostRequestBodyParser.ReadToBoundary, "bound: <" << bound << ">");
	char c = '\0';
	bool newLineFoundLastLine = false;
	while (input.good()) {
		bool boundaryseen = false;
		bool newLineFoundThisLine = false;

		String line;
		line.Append(input, 4096L, coast::streamutils::LF);
		fUnparsedContent.Append(line);
		if (coast::streamutils::LF == input.peek() && input.get(c).good()) {
			fUnparsedContent.Append(c);
			newLineFoundThisLine = true;
		}
		Trace("Length: " << line.Length() << "\nLineContent: <" << line << ">");

		long nextDoubleDash = line.Contains("--");
		Trace("nextDoubleDash: " << nextDoubleDash);
		if (nextDoubleDash != -1L) {
			long boundPos = line.Contains(bound);
			Trace("Boundary position: <" << boundPos << ">");
			if (boundPos != -1L) {
				if (boundPos >= nextDoubleDash + 2L) {
					boundaryseen = true;
					if (body.Length() > 0L) {
						nextDoubleDash = line.StrChr('-', nextDoubleDash + bound.Length() + 2L);
						coast::urlutils::TrimENDL(body);

						Trace("Body in Multipart: <" << body << ">");
						return ((nextDoubleDash != -1L) && (line[(long) (nextDoubleDash + 1L)] == '-'));
					}
				}
				if (boundPos <= nextDoubleDash - bound.Length()) {
					coast::urlutils::TrimENDL(body);
					Trace("Body in Multipart: <" << body << ">");
					return true;
				}
			}
		}
		if (!boundaryseen) {
			if (newLineFoundLastLine && body.Length()) {
				body << coast::streamutils::LF; //!@FIXME could be wrong, if last line > 4k
			}
			body << line;
		}
		newLineFoundLastLine = newLineFoundThisLine;
	}
	Trace("Body in Multipart: <" << body << ">");Assert(!input.good());
	return false;
}

bool HTTPPostRequestBodyParser::ParseMultiPart(std::istream &input, const String &bound) {
	// assume next on input is bound and a line separator
	StartTrace(HTTPPostRequestBodyParser.ParseMultiPart);
	bool endReached = false;

	// ignore value of shouldbeempty
	while (!endReached && input.good()) {
		// reaching eof is an error since end of input is determined by seperators
		String body;
		endReached = ReadToBoundary(input, bound, body);
		Trace("Body: <" << body << ">");
		if (body.Length()) {
			IStringStream innerpart(body);
			MIMEHeader hinner;
			try {
				if (hinner.ParseHeaders(innerpart)) {
					Anything partInfo;
					if (!hinner.GetInfo().IsDefined("CONTENT-TYPE")) {
						hinner.GetInfo()["CONTENT-TYPE"] = "multipart/part";
					}
					partInfo["header"] = hinner.GetInfo();
					TraceAny(hinner.GetInfo(), "Header: ");

					HTTPPostRequestBodyParser part(hinner);
					part.Parse(innerpart); // if we found a boundary, could we unget it?

					partInfo["body"] = part.GetContent();
					fContent.Append(partInfo);
				}
			} catch (MIMEHeader::StreamNotGoodException &e) {
				;
			}
		}
	}
	Trace("Actual length of unparsed body: " << fUnparsedContent.Length());

	return endReached;
}
