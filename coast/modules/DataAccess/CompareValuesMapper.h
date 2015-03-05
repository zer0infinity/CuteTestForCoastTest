/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CompareValuesMapper_H
#define _CompareValuesMapper_H

#include "Mapper.h"

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
class CompareValuesMapper: public ResultMapper {
	CompareValuesMapper();
	CompareValuesMapper(const CompareValuesMapper &);
	CompareValuesMapper &operator=(const CompareValuesMapper &);
public:
	CompareValuesMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) CompareValuesMapper(fName);
	}
protected:
	//! evaluate operation and map the result if comparison evaluated to true
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);
};

#endif
