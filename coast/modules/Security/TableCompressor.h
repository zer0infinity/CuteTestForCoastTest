/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TableCompressor_h_
#define _TableCompressor_h_

#include "config_security.h"
#include "SecurityModule.h"

//---- TableCompressor -----------------------------------------------------------
class TableCompressorTest;

//!compressor class that converts input anythings into streamed out output anythings and replaces keys and values table driven
class EXPORTDECL_SECURITY TableCompressor :  public Compressor
{
public:
	TableCompressor(const char *name);
	~TableCompressor();

	//!support for prototypes
	IFAObject *Clone() const {
		return new TableCompressor(fName);
	}

	//!initializes the internal fMap with the mapping, reversemapping, and value mapping tables
	//!config is normally the Config.any bootstrap configuration
	//!configuration is in SecurityModule.TableCompressor
	//! it is possible to add a shortcut in Val2UriTags like { /Expand Roles }, which will search
	//! /Roles in Config.any and expand it into values
	//!	{
	//!		/Key2UriTags { SomeLongKeyName SomeOtherLongKeyName }
	//!		/ValMapTags  { Page Role Action } # some well known stable value ranges
	//!		/Val2UriTags { {/Expand Roles} {/Expand Pages} {/Expand Actions} }
	//! }
	virtual bool Init(ROAnything config);

	//!compresses dataIn into an streamed Anything representation
	//!the keys of dataIn are converted by Key2UriMap
	//!the values of dataIn are converted by Val2UriMap if they are values of ValMapTags e.g. Rolenames, Pagenames, Actionnames
	virtual void DoCompress(String &scrambledText, const Anything &dataIn);

	//!expands scrambledText which is expected to be an streamed Anything representation into dataOut
	//!the keys of scrambledText are converted by Uri2KeyMap
	//!the values of scrambledText are converted by Uri2ValMap if they are values of ValMapTags e.g. Rolenames, Pagenames, Actionnames
	virtual bool DoExpand(Anything &dataOut, const String &scrambledText);

protected:

	//!accesories
	ROAnything GetValMapTags() const;
	ROAnything GetKey2UriMap() const;
	ROAnything GetVal2UriMap() const;
	ROAnything GetUri2KeyMap() const;
	ROAnything GetUri2ValMap() const;
	ROAnything GetMap(const char *tag) const;

	//!utilities
	//!:constructs a mapping table of key/value pairs of the form /SomeLongName "a" /SomeOtherLongName "b" into fMap[tag]
	//!the keys have to be in state as a list { "SomeLongName" "SomeOtherLongName" }
	void MakeTable(ROAnything state, const char *tag, ROAnything config);

	//!constructs the reverse mapping of MakeTable /"a" SomeLongName /"b" SomeOtherLongName
	void MakeReverseTable(ROAnything state, const char *tag, const char *reverseTag);

	//!expands state at index with information found at other places in config
	void ExpandConfig(long index, Anything &state, ROAnything config);

	//!installs a part of found config in state if part is AnyArrayType it descents recursively
	void InstallConfig(long index, Anything &state, ROAnything part);

	//!calculates a short key "a" "ab" out of the position in the list
	String CalcKey(long index);

	//!maps 0 <= index < 52 into a-z,A-Z
	char KeyAt(int index);

	//!holds all mapping tables
	Anything fMap;

private:
	TableCompressor();
	TableCompressor(const TableCompressor &);
	TableCompressor &operator=(const TableCompressor &);

	friend class TableCompressorTest;
};

#endif
