/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _XMLMapper_H
#define _XMLMapper_H

//---- Mapper include -------------------------------------------------
#include "Mapper.h"

//---- XMLMapper ----------------------------------------------------------
//! Mapper to use with ReadXMLFileDAImpl
/*!
<B>XMLMapper-Configuration:</B><PRE>
{
	/Delim				Mapperspec		optional, default is "."  This delim is used to compose the lookup-path
	/IndexDelim			Mapperspec		optional, default is ":". This delim is used to compose the lookup-path
	/Elements			Mapperspec		mandatory, an Anything containing element names to search for
	/IndexedPathOnly    Mapperspec		true/false, default is false.
										if on, generated LookupPathes contain only indexes e.g. :0:1:2
										by default Slotnames are used if present, eg. :0.somename.1
	The result is placed in the tmp store using an Anything. This Anyhing has to be accessed with the name of the mapper you configured
	in the mapper-hierarchy. eg. tmp["mymapper"]["ParsedXMLAsAny"], tmp["mymapper"]["LookupPathes"]

	Meaning of the Slots:	 /ParsedXMLAsAny contains the parsed XML as passed to the mapper
							 /LookupPathes contains the lookup pathes to retrieve the found elements
							  in the /ParsedXMLAsAny  content.
	If IndexedPathOnly is on, the /LookupPathes are composed with index-Only semantics. Otherwise the /LookupPath
	contains the appropriate SlotName, if present.

	Using ReadXMLFileDAImpl and XMLMapper together gives you a neat way to retrieve your XML-Elements
	from a file. For restrictions see the GenericXMLParser.
}
</PRE>
*/
class XMLMapper : public ResultMapper
{
public:
	//--- constructors
	XMLMapper(const char *name);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;
	// ignores the SelectScript-Hook, operates directly on fConfig (FIXME?)
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything);

protected:
	bool Iterate(Anything currentAny, String pathSoFar, long slotIndex, String slotName, bool bFound, Anything &result);

private:
	XMLMapper();
	XMLMapper(const XMLMapper &);
	XMLMapper &operator=(const XMLMapper &);

	char fIndexDelim;
	char fDelim;
	bool fIndexedPathOnly;
};

#endif
