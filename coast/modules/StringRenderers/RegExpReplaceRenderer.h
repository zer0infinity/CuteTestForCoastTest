/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RegExpReplaceRenderer_H
#define _RegExpReplaceRenderer_H

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

//---- RegExpReplaceRenderer ----------------------------------------------------------
//! Search string within another string.
/*! Search \c /Contains within \c /String and either render the position where the \c /Contains string was found within \c /String or render \c /True or \c /False spec if given.
 * In case of empty \c /Contains and/or  \c /String slots, render \c /Error contents.
 * @section cosr1 Renderer configuration
\code
{
	/Text
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
 * @par \c Text
 * Mandatory\n
 * Simple String or Renderer specification used to search/replace within
 *
 * @par \c Expressions
 * Mandatory\n
 * One expression specification or list of expressions to apply on \c Text in sequence of list
 *
 * @par \c Pattern
 * Mandatory, if \c SimplePattern not specified, this one has precedence\n
 * Regular expression pattern used to search within contents of \c Text. Simple String or Renderer specification supported.
 *
 * @par \c SimplePattern
 * Mandatory, if \c Pattern not specified\n
 * Simple Regular expression pattern used to search within contents of \c Text. The pattern will be filtered through RE::SimplePatternToFullRegularExpression() to create a valid regular expression string. Simple String or Renderer specification supported.
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
 * Default is to replace all regular expression matches within \c Text. Set to 0 if you want to only replace the first occurence.
 *
 * @par Example1:
\code
{ /RegExpReplaceRenderer {
	/Text	"Try to match anything within this string"
	/Expressions {
		/Pattern		"any"
		/Replacement	"some"
	}
} }
\endcode

Renders : "Try to match something within this string"

\par Example2:
\code
/TmpStore {
	/blub	anything
	/replacewith some
}
{ /RegExpReplaceRenderer {
	/Text {
		"Try to match "
		/Lookup blub
		" within this string"
	}
	/Expressions {
		{
			/Pattern		"any"
			/Replacement { /Lookup replacewith }
		}
	}
} }
\endcode

Renders : "Try to match something within this string"
*/
class RegExpReplaceRenderer: public Renderer {
	RegExpReplaceRenderer();
public:
	/*! Default constructor for NamedObject
	 * @param name defines the name of the Renderer */
	RegExpReplaceRenderer(const char *name) :
		Renderer(name) {
	}

protected:
	/*! Renders the corresponding result based on a String::Contains() check
	 * @param reply stream to generate output on
	 * @param ctx Context used for output generation
	 * @param config configuration which drives the output generation
	 */
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
