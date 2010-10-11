/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MIMEHeader_H
#define _MIMEHeader_H

#include "config_stddataaccess.h"
#include "LookupInterface.h"
#include "URLUtils.h"

// static constants must be outside class in WIN32
//!default max of header length
static const long cDefaultMaxHeaderSz = 4096;

//!default max of line length
static const long cDefaultMaxLineSz = 1024;

//! parse and store a MIME header as given from a HTTP request
//! special treatment of multipart MIME used for form data and
//! file upload by a POST request
//! usually all header fields should be treated case-insensitive.
//! because our infrastructure (Anything) is case sensitive
//! we normalize all strings used as header-field indexes to uppercase.
class EXPORTDECL_STDDATAACCESS MIMEHeader : public LookupInterface
{
public:
	enum ProcessMode {
		eDoNotSplitHeaderFields,
		eDoSplitHeaderFields,
		eDoSplitHeaderFieldsCookie
	};

	//! represent a mime header
	MIMEHeader(URLUtils::NormalizeTag normalizeKey = URLUtils::eUpshift, ProcessMode = eDoSplitHeaderFields);

	//! read the MIME header from is
	//! reads MIME header from is withlimit the line size to
	//! detect misuse of server
	bool DoReadHeader(std::istream &is, long maxlinelen = cDefaultMaxLineSz, long maxheaderlen = cDefaultMaxHeaderSz);

	//! trim the line end and parse it according to the MIME Header rules
	//! appends contents to fHeader
	bool DoParseHeaderLine(String &line);

	//! answer if we are a header of a multipart MIME message
	bool IsMultiPart();

	//! return the cached boundary string that separate multipart MIME messages
	//! is only useful if Content-Type is multipart/form-data
	const String &GetBoundary();

	//! special case for the canonical "content-length" header field
	//! only valid if set
	//! \return returns the length as set in the header or -1 if none set
	long	GetContentLength();

	// the complete header information as an Anything
	Anything GetInfo() {
		return fHeader;
	}

	//!cut off \\r\\n or only \\r at end of line
	void TrimEOL(String &line) const;

	//!normalize str according to fNormalizeKey setting
	void Normalize(String &str) const;

	//!consume \n at end of line if any
	bool ConsumeEOL(std::istream &in) const;

	// get name of header field and return the index where the delimiting ":" was found in the string
	long GetNormalizedFieldName(String &line, String &fieldname);

	// Detect headers with suspicious content
	bool AreSuspiciosHeadersPresent();

	// Set the flag externally, ussed for main MIMEHeader and "Inner" MIMEHeaders (multipart mime headers)
	void SetAreSuspiciosHeadersPresent(bool newValue);

protected:
	//! parse a line with fieldname ": " value
	//! stores value as string in fHeader[Normalize(fieldname)]
	//! \param fieldname out the normalized fieldname
	bool ParseField(String &line, MIMEHeader::ProcessMode splitHeaderFields);

	//!split ';' seperated list of key=value pairs into anything
	Anything SplitLine(const String &line, URLUtils::NormalizeTag shift = URLUtils::eUpshift);

	//!find out about a multipart/form-data
	void CheckMultipartBoundary(const String &contenttype);

	// method to subclass if the lookup behaviour shall deviate from the standard
	// implementation (i.e. allow more Anys to be searched, hierarchical, etc)
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

	// determine the processing mode depending on the config passed to MIMEHeader and the header field name
	MIMEHeader::ProcessMode GetDoSplitHeaderFieldsState(const String &fieldNameUpperCase);

	// Check headerfield value for POST or GET content
	bool CheckValues(String &value);

	//!contains the request/reply header
	Anything fHeader;

	//!boundary between different part of multipart/form-data
	String fBoundary;
	bool fBoundaryChecked;
	URLUtils::NormalizeTag fNormalizeKey;
	ProcessMode fSplitHeaderFields;
	bool fAreSuspiciosHeadersPresent;

private:
	MIMEHeader(const MIMEHeader &);
};

#endif
