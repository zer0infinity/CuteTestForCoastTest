/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ANYTHINGUTILS_H
#define _ANYTHINGUTILS_H

#include "Context.h"

//! Use this class to copy item from Context to an Anything
//! To use this class; create an instance and call Operate on it.
class StoreCopier
{
public:
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

private:
	//!deprecated use static API
	StoreCopier();
};

//! Use this class to get a slot from a context store according to configuration
//! To use this class; create an instance and call Operate on it.
class StoreFinder
{
public:
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
	//! \return a Store from the context according to storeName  (Role -> RoleStore, Session-> SessionStore, Request, everything else ->TmpStore )
	static Anything &FindStore(Context &c, String &storeName);

private:
	//!deprecated use static API
	StoreFinder();
};

//! Use this class to put an Anything into a context store using lookup-syntax
/*! To use this class; create an instance and call Operate on it.
 * @section storeputterconfiguration StorePutter Configuration
 * @par \c Store
 * \b optional, default TempStore, in which store shall the values be put, possible store names are\n
 * \li Role
 * \li Session
 * \li Request
 * @par \c Slot
 * \b mandatory, slotname in Anything::LookupPath semantics like \c Level1.Level2	where to store the values.\n
 * This is a Renderer specification, producing the final \c Slot argument used to store away the destination content.
 * The (nested) slot(s) will be created if they do not exist.\n
 * If \c Slot contains an empty string (""), nothing will be stored
 * @par \c Append
 * \b optional, values [0|1], default 0\n
 * Specify if we should append (1) to destination or overwrite (0)
 * @par \c Delim
 * \b optional, default "."\n
 * First character is taken as delimiter for named slots
 * @par \c IndexDelim
 * \b optional, default ":"\n
 * First character is taken as delimiter for indexed slots
 */
class StorePutter
{
public:
	/*! Store source Anything into Context using the specification given in config
	 * @param source The Anything that provides the data, remains unchanged
	 * @param c the context that provides the store
	 * @param config configuration how to put source, see \ref storeputterconfiguration for specs
	 * @post c.LookupPath(config["Slot"].AsString("")) = source */
	static void Operate(Anything &source, Context &c, const Anything &config);
	static void Operate(Anything &source, Context &c, const ROAnything &config);

	/*! puts the Anything source into context using a Anything.LookupPath() like slot specification
		\param source The Anything that provides the data, remains unchanged
		\param c Context to operate on
		\param strStoreName name of store in Context to put destination Anything into
		\param destSlotname slotname to put destination Anything into
		\param append set to true if the source anything should be appended to existing content
		\param delim LookupPath slot delimiter
		\param indexdelim LookupPath index delimiter
		\post dest.LookupPath(destSlotname, delim, delimIdx)[.Append] = source */
	static void Operate(Anything &source, Context &c, String strStoreName, String destSlotname, bool append = false, char delim = '.', char indexdelim = ':');

private:
	//!deprecated use static API
	StorePutter();
};

#endif		//ifndef _ANYTHINGUTILS_H
