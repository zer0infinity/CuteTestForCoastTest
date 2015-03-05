/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#ifndef StringLengthResultMapper_H_
#define StringLengthResultMapper_H_

#include "Mapper.h"

//! Calculate length of value as String and be aware of UTF8 character sequences.
/*!
 * @section slrm1 Mapper configuration
 * This ResultMapper has no specific configuration.
 *
 * @copydoc ResultMapperConfiguration
 *
 * @par Example1:
@code
/StringLengthResultMapper {
	/DestinationSlot	Body
	/Content {
	    /Content-Length { # on the fly slot rename
			/ResultMapper *
		}
	}
}
@endcode
 * value to put with key \b Content
@code
"\xC3\xA4\xC3\xB6\xC3\xBC"
@endcode
 * resulting output in TmpStore would be:
@code
/Body {
	/Content-Length 3
}
@endcode
*/
class StringLengthResultMapper: public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	StringLengthResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) StringLengthResultMapper(fName);
	}
protected:
	//! calculate string length of given value and store it away
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! calculate string length of stream and store it away
	/*! @copydoc ResultMapper::DoPutStream(const char *, std::istream &, Context &, ROAnything) */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);
};

#endif /* StringLengthResultMapper_H_ */
