/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegExpSearchReplaceResultMapper_H
#define _RegExpSearchReplaceResultMapper_H

#include "Mapper.h"

//! Search and replace by regular expression a String convertible value or stream.
/*! Search regular expression \c /Pattern or \c /SimplePattern in String convertible value or stream.
 *
 * @section resrrm1 Mapper configuration
\code
{
	/Expressions {
		{	# optional if only one expression used
			/Pattern			# either
			/SimplePattern		# or
			/MatchFlags
			/Replacement
			/ReplaceAll
		}
		...
	}
}
\endcode
 * @par \c Expressions
 * Mandatory\n
 * One expression specification or list of expressions to apply on put value in sequence of list
 *
 * @par \c Pattern
 * Mandatory, if \c SimplePattern not specified, this one has precedence\n
 * Regular expression pattern used to search within contents of value. Simple String or Renderer specification supported.
 *
 * @par \c SimplePattern
 * Mandatory, if \c Pattern not specified\n
 * Simple Regular expression pattern used to search within contents of value. The pattern will be filtered through RE::SimplePatternToFullRegularExpression()
 * to create a valid regular expression string. Simple String or Renderer specification supported.
 *
 * @par \c MatchFlags
 * Optional, default 0 (RE::MATCH_NORMAL which is case sensitive matching), check \ref RE::eMatchFlags for valid values
 *
 * @par \c Replacement
 * Mandatory\n
 * Simple String or Renderer specification resulting in a string to be used as replacement for the matching regular expression.\n
 * \note Group specific replacement strings like \\1 are not supported.
 *
 * @par \c ReplaceAll
 * Optional, default 1 (true)\n
 * Default is to replace all regular expression matches within value. Set to 0 if you want to only replace the first occurrence.
 *
 * @par Example1:
\code
/RegExpSearchReplaceResultMapper {
	/DestinationSlot "MyDestination"
	/Expressions {
		{
			/Pattern "repl\w+ "
			/Replacement ""
		}
		{
			/SimplePattern " blu*"
			/Replacement ""
		}
	}
	/putkey {
		/ResultMapper *
	}
}
@endcode
 * value to put with key \b putkey
@code
Search replace BlUb in this blubby string
@endcode
 * resulting output in TmpStore would be:
@code
/MyDestination {
	/putkey	"Search BlUb in this"
}
@endcode
*/
class RegExpSearchReplaceResultMapper: public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	RegExpSearchReplaceResultMapper(const char *name) :
		ResultMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RegExpSearchReplaceResultMapper(fName);
	}

protected:
	//! regular expression replacement for simple anything types convertible to String
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! regular expression replacement for streams
	/*! @copydoc ResultMapper::DoPutStream() */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script);
};
#endif
