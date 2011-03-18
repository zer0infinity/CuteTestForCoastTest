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

//! Parse and store a MIME header as given from an HTTP request
/*! special treatment of multipart MIME used for form data and
	file upload by a POST request.
	Usually all header fields should be treated case-insensitive.
	Because our infrastructure (Anything) is case sensitive
	we normalize all strings used as header-field indexes to uppercase. */
class MIMEHeader: public LookupInterface {
	//!contains the request/reply header
	Anything fHeader;
	Coast::URLUtils::NormalizeTag fNormalizeKey;

	MIMEHeader(const MIMEHeader &);
public:
	//! represent a mime header
	MIMEHeader(Coast::URLUtils::NormalizeTag normalizeKey = Coast::URLUtils::eUpshift);

	//! read the MIME header from is
	/*! reads MIME header from is withlimit the line size to detect misuse of server */
	bool ParseHeaders(std::istream &is, long const maxlinelen = cDefaultMaxLineSz, long const maxheaderlen = cDefaultMaxHeaderSz);

	//! answer if we are a header of a multipart MIME message
	bool IsMultiPart() const;

	//! return the cached boundary string that separates multipart MIME messages
	/*! This function is only useful if Content-Type is multipart/form-data
	 * \return boundary String
	 */
	String GetBoundary() const;

	//! Get value of "content-length" header field
	/*! Only valid if the canonical "content-length" header field was set
	 \return length as set in the header or -1 if none set */
	long GetContentLength() const;

	//! the complete header information as an Anything
	Anything GetInfo() const {
		return fHeader;
	}

	struct InvalidLineException {
		String fMessage, fLine;
		InvalidLineException(String const& msg, String const& line) throw () :
			fMessage(msg), fLine(line) {
		}
		virtual const char* what() const throw () {
			return fMessage;
		}
	};
	struct SizeExceededException: MIMEHeader::InvalidLineException {
		long fMaxSize, fActualSize;
		SizeExceededException(String const& msg, String const& line, long lMaxSize, long lActualSize) throw () :
			MIMEHeader::InvalidLineException(msg, line), fMaxSize(lMaxSize), fActualSize(lActualSize) {
			fMessage.Append("; max: ").Append(fMaxSize).Append(" actual: ").Append(fActualSize);
		}
	};
	struct LineSizeExceededException: MIMEHeader::SizeExceededException {
		LineSizeExceededException(String const& msg, String const& line, long lMaxSize, long lActualSize) throw () :
			SizeExceededException(msg, line, lMaxSize, lActualSize) {
		}
	};
	struct HeaderSizeExceededException: MIMEHeader::SizeExceededException {
		HeaderSizeExceededException(String const& msg, String const& line, long lMaxSize, long lActualSize) throw () :
			SizeExceededException(msg, line, lMaxSize, lActualSize) {
		}
	};
	struct StreamNotGoodException {
		const char* what() const throw () {
			return "Stream not good";
		}
	};

protected:
	//! method to subclass if the lookup behaviour shall deviate from the standard
	/*! implementation (i.e. allow more Anys to be searched, hierarchical, etc) */
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const;
};

namespace Coast {
	namespace StreamUtils {
		char const LF = '\n';
		char const CR = '\r';
		void getLineFromStream(std::istream &in, String &line, long const maxlinelen);
	}
}

#endif
