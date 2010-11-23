/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MIMEHeader_H
#define _MIMEHeader_H

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
class MIMEHeader : public LookupInterface
{
public:
	enum ProcessMode {
		eDoNotSplitHeaderFields,
		eDoSplitHeaderFields,
		eDoSplitHeaderFieldsCookie
	};

	//! represent a mime header
	MIMEHeader(Coast::URLUtils::NormalizeTag normalizeKey = Coast::URLUtils::eUpshift, ProcessMode = eDoSplitHeaderFields);

	//! read the MIME header from is
	//! reads MIME header from is withlimit the line size to
	//! detect misuse of server
	bool DoReadHeader(std::istream &is, long const maxlinelen = cDefaultMaxLineSz, long const maxheaderlen = cDefaultMaxHeaderSz);

	//! trim the line end and parse it according to the MIME Header rules
	//! appends contents to fHeader
	bool DoParseHeaderLine(String &line);

	//! answer if we are a header of a multipart MIME message
	bool IsMultiPart();

	//! return the cached boundary string that separate multipart MIME messages
	//! is only useful if Content-Type is multipart/form-data
	const String &GetBoundary() const;

	//! special case for the canonical "content-length" header field
	//! only valid if set
	//! \return returns the length as set in the header or -1 if none set
	long GetContentLength() const;

	// the complete header information as an Anything
	Anything GetInfo() {
		return fHeader;
	}

	// get name of header field and return the index where the delimiting ":" was found in the string
	long GetNormalizedFieldName(String &line, String &fieldname) const;

protected:
	//! parse a line with fieldname ": " value
	//! stores value as string in fHeader[Normalize(fieldname)]
	//! \param fieldname out the normalized fieldname
	bool ParseField(String &line, MIMEHeader::ProcessMode splitHeaderFields);

	//!find out about a multipart/form-data
	void CheckMultipartBoundary(const String &contenttype);

	// method to subclass if the lookup behaviour shall deviate from the standard
	// implementation (i.e. allow more Anys to be searched, hierarchical, etc)
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;

	// determine the processing mode depending on the config passed to MIMEHeader and the header field name
	MIMEHeader::ProcessMode GetDoSplitHeaderFieldsState(const String &fieldNameUpperCase) const;

	//!contains the request/reply header
	Anything fHeader;

	//!boundary between different part of multipart/form-data
	String fBoundary;
	bool fBoundaryChecked;
	Coast::URLUtils::NormalizeTag fNormalizeKey;
	ProcessMode fSplitHeaderFields;

private:
	MIMEHeader(const MIMEHeader &);
};

#endif
