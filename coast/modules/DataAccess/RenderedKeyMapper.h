/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RenderedKeyMapper_H
#define _RenderedKeyMapper_H

//---- ResultMapper include -------------------------------------------------
#include "Mapper.h"

//---- RenderedKeyMapper ----------------------------------------------------------
//! <B>dynamically create Put-key according to rendererspec</B>
/*!
<B>Configuration:</B><PRE>
{
	/KeySpec		Rendererspec	mandatory, renderer specification to create new key
	/StoreKeyAt		String			optional, default not stored in context, location in LookupPath semantics to store created key value in context
}</PRE>
The value Anything to put will be temporarily pushed onto the context to allow lookups within.
*/
class RenderedKeyMapper : public ResultMapper
{
public:
	//--- constructors
	RenderedKeyMapper(const char *name);
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
	RenderedKeyMapper();
	RenderedKeyMapper(const RenderedKeyMapper &);
	RenderedKeyMapper &operator=(const RenderedKeyMapper &);
};

#endif
