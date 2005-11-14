/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "OptionsPrinter.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------

RegisterRenderer(OptionsPrinter);
//---- OptionsPrinter ---------------------------------------------------------

OptionsPrinter::OptionsPrinter(const char *name) : Renderer(name)
{
}

OptionsPrinter::~OptionsPrinter()
{
}

void OptionsPrinter::RenderAll(ostream &reply, Context &c, const ROAnything &config)
{
	StartTrace(OptionsPrinter.Render);
	TraceAny(config, "config");

	for ( int i = 0, sz = config.GetSize(); i < sz; ++i ) {
		String name = config.SlotName(i);
		reply << ' ';
		if ( name.Length() > 0 ) {
			reply  << name ;
			// render option value
			reply << "=\"";
			TraceAny(config[i], "config of option [" << name << "]");
			Render(reply, c, config[i]);  // value is rendererd
			reply << '\"';
		}       // if name is ok
		else {
			TraceAny(config[i], "config of unnamed option");
			Render(reply, c, config[i]);  // value is rendererd
		}
	} // loop
}
