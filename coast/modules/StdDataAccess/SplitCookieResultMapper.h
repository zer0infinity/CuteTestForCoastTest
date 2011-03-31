/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef SPLITCOOKIERESULTMAPPER_H_
#define SPLITCOOKIERESULTMAPPER_H_

#include "Mapper.h"

//! Split up set-cookie Strings and store them away using the cookie name as the new key to put.
/*!
 *
 * @section scrm1 Mapper configuration
 * No specific configuration, basic configuration: \ref ResultMapperConfiguration
 *
 * @par Example1:
@code
/SplitCookieResultMapper {
	/Delim ","
	/HTTPHeader {
		/SET-COOKIE {
			/RenderedKeyResultMapper *
		}
	}
}
/RenderedKeyResultMapper {
	/DestinationSlot "MultiCookies"
	/PutPolicy Append
	/Delim ","
	/KeySpec {
		/ContextLookupRenderer {
			MappedKey
		}
	}
}

@endcode
 * values to put with key \b HTTPHeader.SET-COOKIE
@code
{
	".ASPXAUTH=1D465C16E0F200E2D948F64072EF5762F896AA8F37F22A628F9E67...; path=/"
	".ASPXAUTH=0B26FEB5F5C9DD90554CC9F6884B1AC57C010401C04A9F1EA6703D...; path=/; secure; HttpOnly"
}
@endcode
 * resulting output in TmpStore would be:
@code
/MultiCookies {
	/.ASPXAUTH {
		{
			/_value_ "1D465C16E0F200E2D948F64072EF5762F896AA8F37F22A628F9E67..."
			/_attrs_ {
				/path "/"
			}
		}
		{
			/_value_ "0B26FEB5F5C9DD90554CC9F6884B1AC57C010401C04A9F1EA6703D..."
			/_attrs_ {
				/path "/"
				"secure"
				"HttpOnly"
			}
		}
	}
}
@endcode
*/
class SplitCookieResultMapper : public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	SplitCookieResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SplitCookieResultMapper(fName);
	}
protected:
	//! Split cookie(s) and Put
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);
};

#endif /* SPLITCOOKIERESULTMAPPER_H_ */
