/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _OPTIONSPRINTER_H
#define _OPTIONSPRINTER_H

#include "config_renderer.h"
#include "Renderer.h"

//---- OptionsPrinter -----------------------------------------------------------
//! Renders options within a tag
/*!
\par Configuration
\code
{
	/Key    Value	Rendererspec	optional, produces attributes like KEY=VALUE
	Value			Rendererspec	optional, produces attribute like VALUE
	...
}
\endcode

The OptionsPrinter is used by many Renderers that are producing HTML Tags
to generate user-specified options within the start tag
The OptionsPrinter is not a registered Renderer! It should be instantiated direcly by its clients
It interprets its configuration as Key-Value pairs
Value is always a Renderer specification
It iterates over all slots in the config
If the slot has a name  Slotname=Render(Value)  is rendered
If the slot does not have a name  Render(Value)    is produced
*/
class EXPORTDECL_RENDERER OptionsPrinter : public Renderer
{
public:
	//! use this for direct call of this renderer
	OptionsPrinter();
	//! regular ctor for prototype incarnation in registry
	OptionsPrinter(const char *name);
	~OptionsPrinter();
	void RenderAll(std::ostream &reply, Context &c, const ROAnything &data);
};

#endif		//ifndef _OPTIONSPRINTER_H
