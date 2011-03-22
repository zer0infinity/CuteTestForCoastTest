/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _MimeHeaderResultMapper_H
#define _MimeHeaderResultMapper_H

#include "Mapper.h"

//---- MimeHeaderResultMapper ----------------------------------------------------------
//! Parses header fields from stream and calls DoPutAny() to allow further processing
/*! MimeHeaderResultMapper internally uses MIMEHeader to parse the input stream
 * @section rffrm Mapper configuration
\code
{
	/Normalize		String
}
\endcode
 *
 * @par \c Normalize
 * Optional, default 0 (Coast::URLUtils::eUpshift)\n
 * Specify if and how header field names should be handled, default is to convert all field names to upper case.\n
 * Check \ref Coast::URLUtils::NormalizeTag for valid values
 *
 */
class MimeHeaderResultMapper: public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	MimeHeaderResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MimeHeaderResultMapper(fName);
	}

protected:
	//! Reads from std::istream the MIME headers and Put them with the given key
	/*! @copydoc ResultMapper::DoPutStream() */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script);
};

#endif
