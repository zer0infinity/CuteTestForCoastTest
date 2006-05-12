/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FilterFieldsMapper_H
#define _FilterFieldsMapper_H

//---- ResultMapper include -------------------------------------------------
#include "config_hikups.h"
#include "Mapper.h"

//---- FilterFieldsMapper ----------------------------------------------------------
//! <B>Mapper to filter out unwanted named slots from within the value Anything to put</B>
/*!
<B>Configuration:</B><PRE>
{
	/FieldList	{				optional, default all slots, define subset of slots to put with mapper
		...			String
	}
}</PRE>
*/
class EXPORTDECL_HIKUPS FilterFieldsMapper : public ResultMapper
{
public:
	//--- constructors
	FilterFieldsMapper(const char *name);
	//--- support for prototype
	IFAObject *Clone() const;

protected:
	//! Major hook for subclasses that want to do something with their config passed as script. The default is to interpret the script and put a value for every script item used. Recursion will be stopped by DoFinalPutAny which places its value under slot key below given DoGetDestinationSlot()
	/*! \param key the key usually defines the associated kind of output-value
		\param value the value to be mapped
		\param ctx the context of the invocation
		\param script current mapper configuration as ROAnything
		\return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

private:
	FilterFieldsMapper();
	FilterFieldsMapper(const FilterFieldsMapper &);
	FilterFieldsMapper &operator=(const FilterFieldsMapper &);
};

#endif
