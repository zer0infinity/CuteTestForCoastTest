/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RENDERER_H
#define _RENDERER_H

#include "config_wdbase.h"
#include "StringStream.h"
#include "Context.h"
#include "WDModule.h"

//---- RenderersModule -----------------------------------------------------------
class EXPORTDECL_WDBASE RenderersModule : public WDModule
{
public:
	RenderersModule(const char *name);
	~RenderersModule();

	virtual bool Init(const Anything &config);
	virtual bool ResetFinis(const Anything &);
	virtual bool Finis();
};

//---- Renderer -----------------------------------------------------------
//!rendering interpreter for configurable generation of output
//!Renderer implements a render script interpreter to generate (mostly but necessarily html) output in a flexible, configurable way
class EXPORTDECL_WDBASE Renderer : public NotCloned
{
public:
	//!standard named object constructor
	//! \param name object name for registry
	Renderer(const char *name);
	//!destructor does nothing
	~Renderer();

	//!rendering hook; overwrite this method in subclasses
	//!generates output on reply driven by config using the context given
	//! \param reply stream to generate output on
	//! \param c Context to be used for output generation
	//! \param config configuration which drives the output generation
	virtual void RenderAll(ostream &reply, Context &c, const ROAnything &config);

	//!script interpreter that generates output interpreting info as rendering script
	//!generates output on reply driven by the script info using the context given
	//! \param reply stream to generate output on
	//! \param c Context to be used for output generation
	//! \param info script which drives the output generation
	static void Render(ostream &reply, Context &c, const ROAnything &info);

	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param s string to generate output into
	//! \param c Context to be used for output generation
	//! \param info script which drives the output generation
	static void RenderOnString(String &s, Context &c, const ROAnything &info);
	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param s string to generate output into
	//! \param c Context to be used for output generation
	//! \param info script which drives the output generation
	static String RenderToString(Context &c, const ROAnything &info);

	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param s string to generate output into
	//! \param c Context to be used for output generation
	//! \param info script which drives the output generation
	//! \param def script which drives the output generation if info is empty
	static void RenderOnStringWithDefault(String &s, Context &c, const ROAnything &info, Anything def = "");
	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param s string to generate output into
	//! \param c Context to be used for output generation
	//! \param info script which drives the output generation
	//! \param def script which drives the output generation if info is empty
	static String RenderToStringWithDefault(Context &c, const ROAnything &info, Anything def = "");

	//!utility method to print html options easily legacy implementation
	static void PrintOptions(ostream &reply, const char *tag, const ROAnything &any);
	//!utility method to print html options easily legacy implementation
	static void PrintOptions2(ostream &reply, const ROAnything &any);
	//!utility method to print html options easily using OptionsPrinter
	static void PrintOptions3(ostream &reply, Context &c, const ROAnything &config);

	RegCacheDef(Renderer);	// FindRenderer()
};
#define RegisterRenderer(name) RegisterObject(name, Renderer)

#endif		//ifndef _RENDERER_H
