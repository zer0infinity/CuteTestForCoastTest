/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#ifndef MSAJAXFIXFIELDLENGTHRESULTMAPPER_H_
#define MSAJAXFIXFIELDLENGTHRESULTMAPPER_H_

#include "Mapper.h"

//! Tokenize an MSAjax (UTF8 aware) string and fix potentially broken length fields, e.g. after replacements.
/*! Search regular expression \c /Pattern or \c /SimplePattern in String convertible value or stream.
 *
 * @section msarm1 Mapper configuration
@code
{
	/Fields {
		/<name>		String
		...
	}
	/FieldSeparator	String
}
@endcode
 * @par \c Fields
 * Mandatory\n
 * A list of fieldnames and a token specifying what to do with the field\n
 * The number of fields must match the structure of the ajax to be tokenized\n
 * Following tokens are allowed:\n
 * \c 0 : this designates the length field\n
 * \c * : field does not count for length calculation\n
 * \c "+" : field will be length calculated
 *
 * @par \c FieldSeparator
 * Optional, default \c "|" \n
 * Specify the delimiter to tokenize the ajax string
 *
 * @par Example1:
@code
/MSAjaxFixFieldLengthResultMapper {
	/DestinationSlot	Body
	/Fields {
		/len	0
		/id1	*
		/id2	*
		/value	"+"
	}
	/FieldSeparator	"|"
	/Content {
		/ResultMapper *
	}
}
@endcode
 * value to put with key \b Content
@code
"10|x|y|\xC3\xA4\xC3\xB6\xC3\xBC|"
@endcode
 * resulting output in TmpStore would be:
@code
/Body {
	/Content "3|x|y|\xC3\xA4\xC3\xB6\xC3\xBC|"
}
@endcode
*/
class MSAjaxFixFieldLengthResultMapper: public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	MSAjaxFixFieldLengthResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MSAjaxFixFieldLengthResultMapper(fName);
	}
protected:
	//! potentially correct the length fields of the MSAjax structure
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! potentially correct the length fields of the MSAjax structure
	/*! @copydoc ResultMapper::DoPutStream(const char *, std::istream &, Context &, ROAnything) */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);
};

#endif /* MSAJAXFIXFIELDLENGTHRESULTMAPPER_H_ */
