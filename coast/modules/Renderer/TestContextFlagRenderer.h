/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TestContextFlagRenderer_H
#define _TestContextFlagRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- TestContextFlagRenderer ----------------------------------------------------------
//! Context.Lookup() result of rendered \c FlagName slot as long value and either render \c WhenSet (!=0) or \c WhenNotSet (==0) Renderer specification
/*! @deprecated Use either ConditionalRenderer or SwitchRenderer which are even more flexible
\par Configuration
\code
{
	/FlagName		Rendererspec	mandatory, rendered result will be used to lookup value in Context
	/WhenSet		Rendererspec	optional, gets evaluated when lookup of rendered FlagName slot evaluates to != 0
	/WhenNotSet		Rendererspec	optional, gets evaluated when lookup of rendered FlagName slot evaluates to == 0
}
\endcode
Is equivalent to
\code
/SwitchRenderer { /ContextLookupName <FlagName> /Case { /1 <IsSet> } /Default <NotSet> }
\endcode
or also
\code
/ConditionalRenderer { /ContextCondition <FlagName> /True <IsSet> /False <NotSet> }
\endcode
*/
class EXPORTDECL_RENDERER TestContextFlagRenderer : public Renderer
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	TestContextFlagRenderer(const char *name);
	~TestContextFlagRenderer();

	//! Render output onto \em reply based on \c long lookup of rendered \c FlagName slot
	/*! @copydetails Renderer::RenderAll(ostream &, Context &, const ROAnything &) */
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
