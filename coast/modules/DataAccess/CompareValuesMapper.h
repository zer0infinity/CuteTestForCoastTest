/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CompareValuesMapper_H
#define _CompareValuesMapper_H

//---- ResultMapper include -------------------------------------------------
#include "config_dataaccess.h"
#include "Mapper.h"

//---- CompareValuesMapper ----------------------------------------------------------
//! Mapper to conditionally put a result based on a configurable comparison operation
/*!
<B>Configuration for configured Mapper in OutputMapperMeta.any:</B><PRE>
{
	/LeftValue		Rendererspec	mandatory, how to obtain the left comparison value from Context including 'pushed' value to put
	/RightValue		Rendererspec	mandatory, how to obtain the right comparison value from Context including 'pushed' value to put
	/Operator		Rendererspec	mandatory, ['==','!=','<','>','<=','>='], operator to compare left with right value
}</PRE>

Example:
<br><i>OutputMapperMeta.any contains</i>
<pre>
/SomeConfiguredMapperForADataAccess {
	...
	/QueryResult {
		/CompareValuesMapper *
	}
	...
}
...
/CompareValuesMapper {
	/PutPolicy	Append				# <== base mapper feature
	/LeftValue	5
	/RightValue { /Lookup Value }
	/Operator ">="
}</pre>

Results after DataAccess without using this Mapper:
<pre>
/QueryResult {
	{
		/Value "5"
	}
	{
		/Value "3"
	}
	{
		/Value "12"
	}
	{
		/Value "7"
	}
}</pre>

Results after DataAccess using this Mapper:
<pre>
/QueryResult {
	{
		/Value "5"
	}
	{
		/Value "12"
	}
	{
		/Value "7"
	}
}</pre>
*/
class EXPORTDECL_DATAACCESS CompareValuesMapper : public ResultMapper
{
public:
	//--- constructors
	CompareValuesMapper(const char *name);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

protected:
	//! Major hook for subclasses that want to do something with their config passed as script. The default is to interpret the script and put a value for every script item used. Recursion will be stopped by DoFinalPutAny which places its value under slot key below given DoGetDestinationSlot()
	/*! \param key the key usually defines the associated kind of output-value
		\param value the value to be mapped
		\param ctx the context of the invocation
		\param script current mapper configuration as ROAnything
		\return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

private:
	CompareValuesMapper();
	CompareValuesMapper(const CompareValuesMapper &);
	CompareValuesMapper &operator=(const CompareValuesMapper &);
};

#endif
