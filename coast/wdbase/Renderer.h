/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RENDERER_H
#define _RENDERER_H

#include "StringStream.h"
#include "Context.h"
#include "WDModule.h"

//---- RenderersModule -----------------------------------------------------------
class RenderersModule: public WDModule {
public:
	RenderersModule(const char *name) :
		WDModule(name) {
	}

	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything);
	virtual bool Finis();
};

//---- Renderer -----------------------------------------------------------
//!rendering interpreter for configurable generation of output
//!Renderer implements a render script interpreter to generate (mostly but necessarily html) output in a flexible, configurable way
class Renderer : public NotCloned
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	Renderer(const char *name) :
		NotCloned(name) {
	}

	/*! Main Rendering hook; overwrite this method in subclasses
	 * Generates output on reply driven by configuration using the Context
	 * @param reply stream to generate output on
	 * @param ctx Context to be used for output generation
	 * @param config configuration which drives the output generation */
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);

	//!script interpreter that generates output interpreting info as rendering script
	//!generates output on reply driven by the script info using the context given
	//! \param reply stream to generate output on
	//! \param ctx Context to be used for output generation
	//! \param info script which drives the output generation
	static void Render(std::ostream &reply, Context &ctx, const ROAnything &info);

	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param s string to generate output into
	//! \param ctx Context to be used for output generation
	//! \param info script which drives the output generation
	static void RenderOnString(String &s, Context &ctx, const ROAnything &info);
	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param ctx Context to be used for output generation
	//! \param info script which drives the output generation
	static String RenderToString(Context &ctx, const ROAnything &info);

	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param s string to generate output into
	//! \param ctx Context to be used for output generation
	//! \param info script which drives the output generation
	//! \param def script which drives the output generation if info is empty
	static void RenderOnStringWithDefault(String &s, Context &ctx, const ROAnything &info, Anything def = "");
	//!utility method to generate the contents of a string using renderer scripts
	//!generates output using the strings memory
	//! \param ctx Context to be used for output generation
	//! \param info script which drives the output generation
	//! \param def script which drives the output generation if info is empty
	static String RenderToStringWithDefault(Context &ctx, const ROAnything &info, Anything def = "");

	//!utility method to print html options easily legacy implementation
	static void PrintOptions(std::ostream &reply, const char *tag, const ROAnything &any);
	//!utility method to print html options easily legacy implementation
	static void PrintOptions2(std::ostream &reply, const ROAnything &any);
	//!utility method to print html options easily using OptionsPrinter
	static void PrintOptions3(std::ostream &reply, Context &ctx, const ROAnything &config);

	RegCacheDef(Renderer);	// FindRenderer()
};
#define RegisterRenderer(name) RegisterObject(name, Renderer)

#endif		//ifndef _RENDERER_H
