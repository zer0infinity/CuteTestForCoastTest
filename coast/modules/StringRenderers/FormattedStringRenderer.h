/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FormattedStringRenderer_H
#define _FormattedStringRenderer_H

#include "Renderer.h"

//! Render the given value according to the specification with alignment and with
/*!
\par Configuration
\code
{
	/Value			Rendererspec	String to be rendered
	/Width			Rendererspec	Overall length of the rendered String, if the string is shorter than the field
									length the rest of the string will be filled with the String given in the slot /Filler
	/Filler			Rendererspec	string used for filling the string until reaching Length, Default " "
	/Align			Rendererspec	{ left, center, right }
	/SpacesForTab	Rendererspec	how many fillers should be inserted for a tab (\t) in value
     ...
}
\endcode
*/
class FormattedStringRenderer: public Renderer {
public:
	FormattedStringRenderer(const char *name) :
		Renderer(name) {
	}
protected:
	/*! @copydetails Renderer::RenderAll(std::ostream &, Context &, const ROAnything &) */
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);
};

#endif
