/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _LookupInterface_H
#define _LookupInterface_H

//---- baseclass include -------------------------------------------------
#include "config_wdbase.h"

class ROAnything;

//---- LookupInterface ----------------------------------------------------------
//!public members define lookup protocol for clients; protected members define implementation protocol for subclasses
//! the public members define the client api for lookupable context information. The information can be addressed by a path expression.
//! the protected method DoLookup defines the implementers api
class EXPORTDECL_WDBASE LookupInterface
{
public:
	LookupInterface() {};
	virtual ~LookupInterface() {};

	//!provide immutable context information as ROAnything
	//! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack
	//! \param result the search result as ROAnything, it is empty if nothing is found
	//! \param delim the key delimiter that defines the subparts of the key
	//! \return<i>true</i> if key was found otherwise false
	bool Lookup(const char *key, ROAnything &result, char delim = '.', char indexdelim = ':') const;

	// syntactic sugar: provide search results as return value

	//!provide immutable context information as ROAnything; overloaded API to return a ROAnything directly
	//! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack
	//! \param delim the key delimiter that defines the subparts of the key
	//! \return result the search result as ROAnything, it is empty if nothing is found
	ROAnything Lookup(const char *key, char delim = '.', char indexdelim = ':') const;

	//!provide immutable context information as const char *
	//! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack
	//! \param dflt the value returned if key is not found
	//! \param delim the key delimiter that defines the subparts of the key
	//! \return result the search result as char ptr, it is empty if nothing is found
	const char *Lookup(const char *key, const char *dflt, char delim = '.', char indexdelim = ':') const;

	//!provide immutable context information as long
	//! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack
	//! \param dflt the value returned if key is not found
	//! \param delim the key delimiter that defines the subparts of the key
	//! \return result the search result as long, it is empty if nothing is found
	long Lookup(const char *key, long dflt, char delim = '.', char indexdelim = ':') const;

	//!provide immutable context information as double
	//! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack
	//! \param dflt the value returned if key is not found
	//! \param delim the key delimiter that defines the subparts of the key
	//! \return result the search result as double, it is empty if nothing is found
	double Lookup(const char *key, double dflt, char delim = '.', char indexdelim = ':') const;

protected:
	//!subclass api to search for key in objects context
	//! method to implement in subclasses
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const = 0;
};

#endif
