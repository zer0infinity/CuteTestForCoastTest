/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SetLanguageRenderer_H
#define _SetLanguageRenderer_H

//---- Renderer include -------------------------------------------------
#include "config_renderer.h"
#include "Renderer.h"

//---- SetLanguageRenderer ----------------------------------------------------------
//! <B>Renders a localized value using a given temporary language setting</B>
/*!
<B>Configuration:</B><PRE>
{
	/TempLang	String			optional, temporary language setting
	/Content {	Anything
		...		Rendererspec
	}
}
</PRE>
Renderer sets given language in the context and translates a given input
anything respresenting string (temporary!).
<B>Example:</B>
# MasterLanguage is D
/Lookup InfoText_L # -> text is in german
<PRE>
/Switch {
	/SetLanguageRenderer {
		/TempLang F
		/Content {
			/Lookup InfoText_L  # now this text is in french
		}
	}
}
</PRE>
*/
class EXPORTDECL_RENDERER SetLanguageRenderer : public Renderer
{
public:
	//--- constructors
	SetLanguageRenderer(const char *name);
	~SetLanguageRenderer();

	//!Renders ?? on <I>reply </I>
	//! \param reply out - the stream where the rendered output is written on.
	//! \param c the context the renderer runs within.
	//! \param config the configuration of the renderer.
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
