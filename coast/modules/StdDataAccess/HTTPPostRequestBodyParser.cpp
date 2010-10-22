/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Anything.h"
#include "StringStream.h"
#include "URLUtils.h"
#include "MIMEHeader.h"
#include "Dbg.h"

//--- interface include --------------------------------------------------------
#include "HTTPPostRequestBodyParser.h"

HTTPPostRequestBodyParser::HTTPPostRequestBodyParser(MIMEHeader &mainheader, std::istream &input)
	: fInput(input), fHeader(mainheader)
{
	StartTrace(HTTPPostRequestBodyParser.Ctor);
}

bool HTTPPostRequestBodyParser::Parse()
{
	StartTrace(HTTPPostRequestBodyParser.Parse);
	TraceAny(fHeader.GetInfo(), "Header: ");
	fIn = &fInput;

	if (fHeader.IsMultiPart()) {
		Trace("Multipart detected");
		return ParseMultiPart(fIn, fHeader.GetBoundary());
	} else {
		Trace("Parsing simple body");
		return ParseBody();
	}
}

bool HTTPPostRequestBodyParser::ParseBody()
{
	StartTrace(HTTPPostRequestBodyParser.ParseBody);
	long contentLength = fHeader.GetContentLength();
	String bodyStr;
	ROAnything contenttype;
	if (!fHeader.Lookup("CONTENT-TYPE", contenttype)) {
		return false;
	}
	if (contenttype == "application/x-ifa-anything") {
		// there must be exactly one anything in the body
		// handle our special format more efficient than
		// the standard cases
		Anything a;
		if (a.Import(*fIn) && !a.IsNull()) {
			TraceAny(a, "Content-type: application/x-ifa-anything");
			fContent.Append(a);
			return true;
		} // if
		else {
			Trace("application/x-ifa-anything - syntax error");
			return false;
		}
	}
	// 2 cases: content length defined, or everything
	bool readSuccess = false;
	if (contentLength >= 0) {
		fUnparsedContent.Append(*fIn, contentLength);
		readSuccess = (fUnparsedContent.Length() == contentLength);
	} else {
		// get everything
		while (fIn->good()) {
			fUnparsedContent.Append(*fIn, 4096);
		}
		readSuccess = fIn->eof();
	}
	Trace("Body: <" << fUnparsedContent << ">");
	ROAnything contentdisp;
	if ( fHeader.Lookup("CONTENT-DISPOSITION", contentdisp) && contentdisp[0L] == "form-data" ) {
		String name( contentdisp["NAME"].AsCharPtr() );
		String filename( contentdisp["FILENAME"].AsCharPtr() );
		// it's a userfile upload
		if ( name.Length() > 0 && filename.Length() > 0 ) {
			fContent.Append(fUnparsedContent);
		} else if (name.Length() > 0 || fUnparsedContent != "") {
			Decode(fUnparsedContent, fContent);
		}
	} else {
		if (fUnparsedContent != "") {
			// this is the fucking billy browser case
			// they are not able to supply a reasonable
			// mime type
			Decode(fUnparsedContent, fContent);
		}
	}
	return readSuccess;
}

void HTTPPostRequestBodyParser::Decode(String str, Anything &result)
{
	StartTrace(HTTPPostRequestBodyParser.Decode);
	// add a sanity check and remove trailing \r\n in case
	long slen = str.Length();
	if (slen >= 2 && '\r' == str[(long)(slen-2)] && '\n' == str[(long)(slen-1)]) {
		str.Trim(slen - 2);
	}
	URLUtils::Split(str, '&', result);
	URLUtils::DecodeAll(result);
}

bool HTTPPostRequestBodyParser::ReadToBoundary(std::istream *is, const String &bound, String &body)
{
	StartTrace1(HTTPPostRequestBodyParser.ReadToBoundary, "bound: <" << bound << ">");
	if ( !is ) {
		return true;
	}
	bool newLineFoundLastLine = false;
	while (is->good()) {
		bool boundaryseen = false;
		bool newLineFoundThisLine = false;

		String line;
		line.Append(*is, 4096L, '\n');
		Trace("Line: <" << line << "\n>");

		fUnparsedContent.Append(line);
		if (fHeader.ConsumeEOL(*is)) {
			Trace("consumed endline and appending \\n to unparsed content");
			fUnparsedContent.Append('\n');
			newLineFoundThisLine = true;
		}

		long nextDoubleDash = line.Contains("--");
		Trace("nextDoubleDash: " << nextDoubleDash);
		if ( nextDoubleDash != -1L ) {
			long boundPos = line.Contains(bound);
			Trace("Boundary position: <" << boundPos << ">");
			if ( boundPos != -1L ) {
				if (boundPos >= nextDoubleDash + 2L) {
					boundaryseen = true;
					if (body.Length() > 0L) {
						nextDoubleDash = line.StrChr('-', nextDoubleDash + bound.Length() + 2L);
						fHeader.TrimEOL(body);

						Trace("Body in Multipart: <" << body << ">");
						return ((nextDoubleDash != -1L) && (line[(long)(nextDoubleDash+1L)] == '-'));
					}
				}
				if (boundPos <= nextDoubleDash - bound.Length()) {
					fHeader.TrimEOL(body);
					Trace("Body in Multipart: <" << body << ">");
					// we are done
					return true;
				}
			}
		}
		if (!boundaryseen) {
			if (newLineFoundLastLine && body.Length()) {
				body << '\n'; //!@FIXME could be wrong, if last line > 4k
			}
			body << line;
		}
		newLineFoundLastLine = newLineFoundThisLine;
	}
	Trace("Body in Multipart: <" << body << ">");
	Assert(!is->good());
	return false; // was return true
}

bool HTTPPostRequestBodyParser::ParseMultiPart(std::istream *is, const String &bound)
{
	// assume next on input is bound and a line separator
	StartTrace(HTTPPostRequestBodyParser.ParseMultiPart);
	bool endReached = false;

	// ignore value of shouldbeempty
	while (!endReached && is->good()) {
		// reaching eof is an error since end of input is determined by seperators
		String body;
		endReached = ReadToBoundary(is, bound, body);
		Trace("Body: <" << body << ">");
		if ( body.Length() ) {
			IStringStream innerpart(body);
			MIMEHeader hinner;
			if (hinner.DoReadHeader(innerpart)) {
				Anything partInfo;
				if (!hinner.GetInfo().IsDefined("CONTENT-TYPE") ) {
					hinner.GetInfo()["CONTENT-TYPE"] = "multipart/part";
				}
				partInfo["header"] = hinner.GetInfo();
				TraceAny(hinner.GetInfo(), "Header: ");

				HTTPPostRequestBodyParser part(hinner, innerpart);
				part.Parse(); // if we found a boundary, could we unget it?

				partInfo["body"] = part.GetContent();
				fContent.Append(partInfo);
			}
			// Communicate with "Main" header
			if (hinner.AreSuspiciousHeadersPresent() ) {
				fHeader.SetSuspiciousHeadersPresent(true);
			}
		}
	}
	Trace("Actual length of unparsed body: " << fUnparsedContent.Length());

	return endReached;
}
