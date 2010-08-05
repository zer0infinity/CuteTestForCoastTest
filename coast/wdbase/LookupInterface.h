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
//! Define client API for lookupable context information
/*! public members define the lookup protocol for clients, protected members define implementation protocol for subclasses
 * The information can be addressed by a Anything::LookupPath() path expression. */
class EXPORTDECL_WDBASE LookupInterface
{
public:
	LookupInterface() {};
	virtual ~LookupInterface() {};

	//! Provide immutable context information as ROAnything and report if the lookup was successful
	/*! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack:0.yeah
		\param delim the key delimiter that defines the subparts of the key
		\param indexdelim the index delimiter that defines indexed subparts
		\param result the search result as ROAnything, it is empty if nothing is found
		\return true if key was found
		\return false otherwise */
	bool Lookup(const char *key, ROAnything &result, char delim = '.', char indexdelim = ':') const;

	//! Provide immutable context information as ROAnything; return a ROAnything directly
	/*! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack:0.yeah
		\param delim the key delimiter that defines the subparts of the key
		\param indexdelim the index delimiter that defines indexed subparts
		\return result the search result as ROAnything, it is empty if nothing is found */
	ROAnything Lookup(const char *key, char delim = '.', char indexdelim = ':') const;

	//! Provide immutable context information as const char *
	/*! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack:0.yeah
		\param delim the key delimiter that defines the subparts of the key
		\param indexdelim the index delimiter that defines indexed subparts
		\param dflt the value returned if key is not found
		\return result the search result as char ptr, it is empty if nothing is found */
	const char *Lookup(const char *key, const char *dflt, char delim = '.', char indexdelim = ':') const;

	//! Provide immutable context information as long value
	/*! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack:0.yeah
		\param delim the key delimiter that defines the subparts of the key
		\param indexdelim the index delimiter that defines indexed subparts
		\param dflt the value returned if key is not found
		\return result the search result as long, it is empty if nothing is found */
	long Lookup(const char *key, long dflt, char delim = '.', char indexdelim = ':') const;

	//! Provide immutable context information as double value
	/*! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack:0.yeah
		\param delim the key delimiter that defines the subparts of the key
		\param indexdelim the index delimiter that defines indexed subparts
		\param dflt the value returned if key is not found
		\return result the search result as double, it is empty if nothing is found */
	double Lookup(const char *key, double dflt, char delim = '.', char indexdelim = ':') const;

protected:
	//! Subclass hook to implement real work
	/*! \param key the search key; it can be segmented into subparts delimited by delim eg. what.a.hack:0.yeah
		\param delim the key delimiter that defines the subparts of the key
		\param indexdelim the index delimiter that defines indexed subparts
		\param result the search result as ROAnything, it is empty if nothing is found
		\return true if key was found
		\return false otherwise */
	virtual bool DoLookup(const char *key, ROAnything &result, char delim, char indexdelim) const = 0;
};

#endif
