/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SlotnameOutputMapper_H
#define _SlotnameOutputMapper_H

//---- Mapper include -------------------------------------------------
#include "NameUsingOutputMapper.h"

//---- SlotnameOutputMapper ----------------------------------------------------------
//! Stores values below a configured slot and groups the output using the SlotnameSlots specification -> see example
/*!
<b>Configuration in OutputMapperMeta.any:</b><pre>
{
	/Destination {							optional
		/Store			String				mandatory, default TempStore [Role|Session|Request|TempStore], name of the destination store
		/Slot			String				qualified slot name
		/Delim   		String				optional, default ".", first char is taken as delimiter for named slots
		/IndexDelim		String				optional, default ":", first char is taken as delimiter for indexed slots
	}
	/SlotnameSlots {						optional
		TopLevelSlot						optional,defines a slot of the result set whose value will be taken to group the results by
		SecondLevelSlot						optional,defines a slot of the result set whose value will be taken to sub-group the results by
		...
	}
} </pre>
Default configuration is the mappers alias name in TempStore
<b>Example:</b><pre>
/MyMapper {
	/Destination {
		/Slot	MySlot
	}
	/SlotnameSlots {
		Category
		SubCategory
	}
}

/ResultSetToProcess {
	{
		/Category		Games
		/SubCategory	Adventure
		/SomeValue		Myst
	}
	{
		/Category		Food
		/SubCategory	Beverage
		/SomeValue		Coke
	}
	{
		/Category		Tools
		/SubCategory	Electrical
		/SomeValue		Whatever
	}
	{
		/Category		Food
		/SubCategory	Beverage
		/SomeValue		Water
	}
	{
		/Category		Games
		/SubCategory	Shooter
		/SomeValue		Doom
	}
}
</pre>
The result which gets stored in TempStore will then look like this:
<pre>
/MySlot {
	/Games {
		/Adventure {
			{
				/Category		Games
				/SubCategory	Adventure
				/SomeValue		Myst
			}
		}
		/Shooter {
			{
				/Category		Games
				/SubCategory	Shooter
				/SomeValue		Doom
			}
		}
	}
	/Food {
		/Beverage {
			{
				/Category		Food
				/SubCategory	Beverage
				/SomeValue		Coke
			}
			{
				/Category		Food
				/SubCategory	Beverage
				/SomeValue		Water
			}
		}
	}
	/Tools {
		/Electrical {
			{
				/Category		Tools
				/SubCategory	Electrical
				/SomeValue		Whatever
			}
		}
	}
}
</pre>
*/
class SlotnameOutputMapper : public NameUsingOutputMapper
{
public:
	SlotnameOutputMapper(const char *name) : NameUsingOutputMapper(name) {};
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SlotnameOutputMapper(fName);
	};

	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything config);

private:
	SlotnameOutputMapper();
	SlotnameOutputMapper(const SlotnameOutputMapper &);
	SlotnameOutputMapper &operator=(const SlotnameOutputMapper &);
};

#endif
