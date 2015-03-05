/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegExpFilterFieldsParameterMapper_H
#define _RegExpFilterFieldsParameterMapper_H

#include "Mapper.h"

//! Calls ParameterMapper::DoGetAny() on named value entries matched by regular expressions. Positive and negative filtering can be applied.
/*! Lookup values in the Context and map them using the given configuration.
 * Provide a MapperScript which selects slots from within the initial key position based on a regular expression.
 * @section rffrm Mapper configuration
\code
{
	/MatchFlags				long
	/ComposeKey				long
	/NoMatchReturnValue		long
	/key {
		/"regular-expr"		MapperSpec	# positive filter
		/"regular-expr"		-			# negative filter
		...
	}
}
\endcode
 * @par \c MatchFlags
 * Optional, default 0 (RE::MATCH_NORMAL which is case sensitive matching)\n
 * Check \ref RE::eMatchFlags for valid values
 *
 * @par \c NoMatchReturnValue
 * Optional, default 1 (true)\n
 * return value of mapping if no matching slot found
 *
 * @par \c /regular-expression \c MapperSpec
 * Optional\n
 * Positive regular expression rule filter, ParameterMapper::DoGetAny() will be called with the contents of the matched slot of value.
 *
 * @par \c /regular-expression \c -
 * Optional\n
 * Negative regular expression rule filter, the matched slot of value will be ignored.
 *
 * @par Example1:
 * @code
{ /FilterSomeHeaderFields {
	/DestinationSlot SomeWhere
	/MatchFlags		1
	/CodeExample {
		/"^content"	{			# execute Mapper script for entries starting with "content"
			/RootMapper	*
		}
		/"^server"	"-"			# do not map this entry
		/"."		*			# catch all others and do default putting
	}
} }
@endcode
 * value to put:
@code
/CodeExample {
	/connection "close"
	/date "Thu, 11 Nov 2010 08:30:36 GMT"
	/server "Microsoft-IIS/6.0"
	/x-powered-by "ASP.NET"
	/x-aspnet-version "2.0.50727"
	/cache-control "private"
	/content-type "text/html; charset=utf-8"
	/content-length "4611"
}
@endcode
 * resulting output in TmpStore would be:
@code
/SomeWhere {
	/CodeExample {
		/connection "close"
		/date "Thu, 11 Nov 2010 08:30:36 GMT"
		/x-powered-by "ASP.NET"
		/x-aspnet-version "2.0.50727"
		/cache-control "private"
	}
}
/CodeExample {
	/content-type "text/html; charset=utf-8"
	/content-length "4611"
}
@endcode */
class RegExpFilterFieldsParameterMapper: public ParameterMapper {
	template<typename ValueType> bool getValueWithSlotname(const char *key, String const &strSlotname, Context & ctx, ValueType & value, ROAnything script);
	template<typename ValueType> bool getValueWithSlotnameCommon(const char *key, String const &slotname, Context & ctx, ValueType & value, ROAnything script, Anything &availableContent);
	template<typename ValueType> bool intInterpretMapperScript(const char *key, ValueType &value, Context &ctx, ROAnything script);

public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	RegExpFilterFieldsParameterMapper(const char *name) :
		ParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RegExpFilterFieldsParameterMapper(fName);
	}

protected:
	//! Override default implementation and implement RE based value getting
	/*! @copydoc ParameterMapper::DoGetAny(const char *, Anything &, Context &, ROAnything) */
	virtual bool interpretMapperScript(const char *key, Anything &value, Context &ctx, ROAnything script);
	//! Override default implementation and implement RE based value getting
	/*! @copydoc ParameterMapper::DoGetStream(const char *, std::ostream &, Context &, ROAnything) */
	virtual bool interpretMapperScript(const char *key, std::ostream &value, Context &ctx, ROAnything script);
};
#endif
