/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LOOKUPRENDERERS_H
#define _LOOKUPRENDERERS_H

#include "Renderer.h"

//! Abstract base class for LookupRenderers
/*!
 \par Configuration
 \code
{
	/LookupName			Rendererspec	mandatory, name to lookup
	/Default			Rendererspec	optional, default if LookupName name slot was not found
	/Delim   			String			optional, default ".", first char is taken as delimiter for lookup path
	/IndexDelim			String			optional, default ":", first char is taken as index-delimiter for lookup path
}
 \endcode
 or just
 \code
{
	"name" "default renderer specification" "delim" "indexdelim"
}
 \endcode

 LookupRenderer is the abstract super class for renderers that use a lookup name and the Context to retrieve an Anything that is then used as a renderer specification (see Renderer::Render() for valid renderer specifications):

 The LookupRenderer provides indirection of rendering via a renderer
 specification or some simple data that resides somewhere within the current
 Context. (That data is usually put in the Context by the page's preprocessing or
 the previous page's postprocessing action. A subclass of LookupRenderer may then
 be used to dynamically create some page content using the data.)

 -# By using the two explicit tags /LookupName and /Default
 -# Or by just listing two entries in the input 'config' data. The first entry is then used as the lookup name and the second one is used as the default.

 \note LookupName and Default may contain any renderer specification. The default may also be omitted. Additional entries in the input data are ignored by the LookupRenderer.

 Subclasses of LookupRenderer are supposed to override the method DoLookup() to define how the data is actually retrieved.
 */
class LookupRenderer: public Renderer {
public:
	/*! standard named object constructor
	 \param name name of concrete class */
	LookupRenderer(const char *name) :
		Renderer(name) {
	}

	/*! common rendering hook
	 \param reply out - the stream where the rendered output is written on.
	 \param c the context the renderer runs within.
	 \param config the configuration of the renderer. */
	void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

protected:
	/*! DoLookup to be implemented by subclasses
	 \param context the context the renderer runs within.
	 \param name the path to lookup
	 \param delim a character specifying the named slot delimiter
	 \param indexdelim a character specifying the unnamed slot delimiter (array indices) */
	virtual ROAnything DoLookup(Context &context, const char *name, char delim, char indexdelim) = 0;
};

//! Concrete Renderer to lookup and renderer things from Context
/*!
 \par Configuration
 \code
{
	/LookupName			Rendererspec	mandatory, name to lookup
	/Default			Rendererspec	optional, default if LookupName name slot was not found
	/Delim   			String			optional, default ".", first char is taken as delimiter for lookup path
	/IndexDelim			String			optional, default ":", first char is taken as index-delimiter for lookup path
}
 \endcode
 or just
 \code
{
	"name" "default renderer specification" "delim" "indexdelim"
}
 \endcode
 */
class ContextLookupRenderer: public LookupRenderer {
public:
	/*! standard named object constructor
	 \param name name of concrete class */
	ContextLookupRenderer(const char *name) :
		LookupRenderer(name) {
	}

protected:
	/*! overriden DoLookup implementation
	 \param context the context the renderer runs within.
	 \param name the path to lookup
	 \param delim a character specifying the named slot delimiter
	 \param indexdelim a character specifying the unnamed slot delimiter (array indices) */
	ROAnything DoLookup(Context &context, const char *name, char delim, char indexdelim);
};

//! Concrete Renderer to lookup and renderer things from TempStore
/*!
 \par Configuration
 \code
 {
	 /LookupName			Rendererspec	mandatory, name to lookup
	 /Default			Rendererspec	optional, default if LookupName name slot was not found
	 /Delim   			String			optional, default ".", first char is taken as delimiter for lookup path
	 /IndexDelim			String			optional, default ":", first char is taken as index-delimiter for lookup path
 }
 \endcode
 or just
 \code
 {
 	 "name" "default renderer specification" "delim" "indexdelim"
 }
 \endcode
 */
class StoreLookupRenderer: public LookupRenderer {
public:
	/*! standard named object constructor
	 \param name name of concrete class */
	StoreLookupRenderer(const char *name) :
		LookupRenderer(name) {
	}
protected:
	/*! overriden DoLookup implementation
	 \param context the context the renderer runs within.
	 \param name the path to lookup
	 \param delim a character specifying the named slot delimiter
	 \param indexdelim a character specifying the unnamed slot delimiter (array indices) */
	ROAnything DoLookup(Context &context, const char *name, char delim, char indexdelim);
};

//! Concrete Renderer to lookup and renderer things from Query
/*!
 \par Configuration
 \code
 {
	 /LookupName			Rendererspec	mandatory, name to lookup
	 /Default			Rendererspec	optional, default if LookupName name slot was not found
	 /Delim   			String			optional, default ".", first char is taken as delimiter for lookup path
	 /IndexDelim			String			optional, default ":", first char is taken as index-delimiter for lookup path
 }
 \endcode
 or just
 \code
 {
 	 "name" "default renderer specification" "delim" "indexdelim"
 }
 \endcode
 \note The query is not included using an ordinary ContextLookupRenderer */
class QueryLookupRenderer: public LookupRenderer {
public:
	/*! standard named object constructor
	 \param name name of concrete class */
	QueryLookupRenderer(const char *name) :
		LookupRenderer(name) {
	}

protected:
	/*! overriden DoLookup implementation
	 \param context the context the renderer runs within.
	 \param name the path to lookup
	 \param delim a character specifying the named slot delimiter
	 \param indexdelim a character specifying the unnamed slot delimiter (array indices) */
	ROAnything DoLookup(Context &context, const char *name, char delim, char indexdelim);
};

#endif
