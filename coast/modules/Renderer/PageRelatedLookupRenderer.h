/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _PageRelatedLookupRenderer_H
#define _PageRelatedLookupRenderer_H

#include "config_renderer.h"
#include "LookupRenderers.h"

//---- PageRelatedLookupRenderer --------------------------------------------------------------
//! <B>LookupRenderer that uses the current page name when performing a lookup</B>
/*!
<B>Configuration:</B><PRE>
{
	/LookupName		Rendererspec	optional, specification to lookup in context in a page specific slot
}
or just
{
	"name of a renderer in context"
}
</PRE>
PageRelatedLookupRenderer is a LookupRenderer that uses the current
page name when performing a lookup: A slot which matches
the current page name is first retrieved from the context.
The purpose of PageRelatedLookupRenderer is to allow for the better structuring
of global configuration files, and thereby the reduction of global namespace
pollution (e.g. in LocalizedString.any).

Example:
--------
<PRE>
/PageRelatedLookupRenderer {
	/LookupName "Title_L"
}
</PRE>
PageRelatedLookupRenderer basically creates a page local namespace:
If the above renderer was invoked from page "Foo", the name actually retrieved
from the Context would be: "Foo.Title_L". However use of the same renderer on
page "Bar" performs a lookup of "Bar.Title_L".

Entries in the LocalizedStrings.any file might look like this:
<PRE>
/Foo {		# localized strings for page "Foo"
	/Title_L {
		/Type String
		/Default "Title of page Foo"
	}
}
/Bar {		# localized strings for page "Bar"
	/Title_L {
		/Type String
		/Default "Title of page Bar"
	}
}
</PRE>
*/
class EXPORTDECL_RENDERER PageRelatedLookupRenderer : public LookupRenderer
{
public:
	PageRelatedLookupRenderer(const char *name);

protected:
	virtual ROAnything DoLookup(Context &context, const char *name, char delim, char indexdelim);
};

#endif		//not defined _PageRelatedLookupRenderer_H
