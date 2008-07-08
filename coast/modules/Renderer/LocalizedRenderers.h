/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LOCALIZEDRENDERERS_H
#define _LOCALIZEDRENDERERS_H

#include "config_renderer.h"
#include "Renderer.h"

//---- LanguageSwitchRenderer --------------------------------------------------------------
//! Renders a language specific renderer specification
/*!
\par Configuration
\code
{
	/D			Rendererspec	optional, specification used for german output
	/F			Rendererspec	optional, specification used for french output
	/I			Rendererspec	optional, specification used for italian output
	/E			Rendererspec	optional, specification used for english output
	/Default 	Rendererspec	optional, used if no regular match is found
}
\endcode

LanguageSwitchRenderer selects a language specific renderer specification
to create the reply. The LocalisationUtils class is used for this. The language
name defined by context.Language() is used to look up which renderer specification
to render.
Therefore the tags in the config data passed to StringRenderer are arbitrary,
depending on the choosen representation of the languages.
*/
class EXPORTDECL_RENDERER LanguageSwitchRenderer : public Renderer
{
public:
	LanguageSwitchRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

//---- StringRenderer --------------------------------------------------------------
//! Renders a language specific string
/*!
\par Configuration
\code
{
	/D			String	optional, string used for german output
	/F			String	optional, string used for french output
	/I			String	optional, string used for italian output
	/E			String	optional, string used for english output
	/Default 	String	optional, used if no regular match is found
}
\endcode

StringRenderer is a special case of a LanguageSwitchRenderer. It only
allows simple strings where LanguageRenderer accepts full renderer
specifications. There may be a performance advantage by using a StringRenderer
instead of the more general LanguageSwitchRenderer, however a
LanguageSwitchRenderer could always be used instead of a StringRenderer:
*/
class EXPORTDECL_RENDERER StringRenderer : public Renderer
{
public:
	StringRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &config);
};

#endif
