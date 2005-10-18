/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ANYTHINGUTILS_H
#define _ANYTHINGUTILS_H

#include "config_wdbase.h"
#include "Context.h"

//---- StoreCopier -----------------------------------------------------------

//! Use this class to copy item from Context to an Anything
//! To use this class; create an instance and call Operate on it.
class EXPORTDECL_WDBASE StoreCopier
{
public:

	//!deprecated use static API
	StoreCopier() {};
	virtual ~StoreCopier() {};

	//! Copies slots from context to dest according to config
	//! The config Anything should have the form
	//! <PRE>{
	//!		/ContextLookupName	RendererSpec	# RendererSpec produces the Slotname in the destination Anything
	//! ...
	//! }</PRE>
	//! the method iterates over all slots in config. If context.LookupPath(ContextLookupName) returns an ROAnything,
	//! it is copied into dest[RendererSpec].<BR>
	//! Note, you can specify ContextLookupName as a dot/colon separated list of slotnames to retrieve slots from any
	//! hierarchy level (e.g fields.System). The result is always copied into toplevel slot of dest.

	//!param: c - context, source that provides the data
	//!param: dest - The Anything that is updated.
	//!param: config - Contains the list of source and destination slotnames.
	//!param: delim - delimiting character for named slots
	//!param: indexdelim - delimiting character for indexed slots
	static void Operate(Context &c, Anything &dest, const Anything &config, char delim = '.', char indexdelim = ':');
	static void Operate(Context &c, Anything &dest, const ROAnything &config, char delim = '.', char indexdelim = ':');
};

//---- StoreFinder -----------------------------------------------------------

//! Use this class to get a slot from a context store according to configuration
//! To use this class; create an instance and call Operate on it.
class EXPORTDECL_WDBASE StoreFinder
{
public:

	//!deprecated use static API
	StoreFinder() {};
	virtual ~StoreFinder() {};

	//! looks up the slot in the store, creates it if not found.
	//! The config Anything should have the form
	//! <PRE>{
	//! 	/Store	Role			# opt, The Store where the Slot is lookuped
	//!								# (Known Stores are Role,Session,Request,default TempStore)
	//! 	/Slot	Level1.Level2	# Slot that is searched - if it does not exists it is created
	//!		/Delim   				optional, default ".", first char is taken as delimiter for named slots
	//!		/IndexDelim				optional, default ":", first char is taken as delimiter for indexed slots
	//! }</PRE>
	//! You can specify the Slotname as a dot separated list of names to retrieve slots from any
	//! hierarchy level (e.g fields.System).
	//! \param c context, source that provides the data
	//! \param dest The Anything reference to the slot found
	//! \param config the configuration
	//! \post dest.IsNull() == false.
	static void Operate(Context &c, Anything &dest, const Anything &config);
	static void Operate(Context &c, Anything &dest, const ROAnything &config);

	//! Gets a specific Store by Key
	//! \param c the context that provides the stores
	//! \param storeName name of the store
	//! \return a Store from the context accroding to storeName  (Role -> RoleStore, Session-> SessionStore, Request, everything else ->TmpStore )
	static Anything &FindStore(Context &c, String &storeName);
};

//---- StorePutter -----------------------------------------------------------

//! Use this class to put a Anything into a context store using lookup-syntax
//! To use this class; create an instance and call Operate on it.
class EXPORTDECL_WDBASE StorePutter
{
public:

	//!deprecated use static API
	StorePutter() {};
	virtual ~StorePutter() {};

	//! looks up the slot in the store, creates it if not found.
	//! The config Anything should have the form
	//! <PRE>{
	//! 	/Store	Role			# opt, The Store where the Slot is lookuped
	//!								# (Known Stores are Role,Session,Request,default TempStore)
	//! 	/Slot	Level1.Level2	# Rendererspec producing the Slot that is assigned to source - if it does not exists it is created
	//!		/Append   				optional [0|1], default 0, append source to destination, overwrite otherwise
	//!		/Delim   				optional, default ".", first char is taken as delimiter for named slots
	//!		/IndexDelim				optional, default ":", first char is taken as delimiter for indexed slots
	//! }</PRE>
	//! You can specify the Slotname as a dot separated list of names to retrieve slots from any
	//! hierarchy level (e.g fields.System).
	//! If /Slot contains an empty string ("") not will happen
	//! \param source The Anything that provides the data, remains unchanged
	//! \param c the context that provides the store
	//! \param config the configuration
	//! \post c.LookupPath(config["Slot"].AsString("")) = source
	static void Operate(Anything &source, Context &c, const Anything &config);
	static void Operate(Anything &source, Context &c, const ROAnything &config);
};

#endif		//ifndef _ANYTHINGUTILS_H
