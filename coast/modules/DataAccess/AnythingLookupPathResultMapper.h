/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnythingLookupPathResultMapper_H
#define _AnythingLookupPathResultMapper_H

//---- Mapper include -------------------------------------------------
#include "Mapper.h"

//---- AnythingLookupPathResultMapper ----------------------------------------------------------
//! Uses Anything::LookupPath() on entries specified in mapper configuration and calls ResultMapper::DoPutAny() using the retrieved value.
/*! Lookup values in the given Anything and map them again using the given configuration. Provide a MapperScript which extracts and stores either parts of the converted Anything or the whole Anything.
\par Configuration
\code
{
	/key {
		/Slot.To.Lookup			NewOutputKey		optional, the given slotname will be used as LookupPath on the converted Anything, if the LookupPath was successful this content gets stored in the NewOutputKey
		/NonExistingSlot		ResultsInNoOutput	optional, if the slotname does not exist in the converted Anything nothing will be output
		/Slot.To.Lookup {					optional, this is quite cool, you can let another MapperScript process the lookup'd Anything
			/AnotherOutputKey {				optional, rename the output slot on the fly
				/RootMapper	*			optional, store it in TmpStore at root level
			}
		}
	}
}
\endcode

Example output of pseudo-configuration from above assuming we got the following stream to convert <b>{ /Slot { /To { /Lookup { 123 /a bcd } } /Something 123 } }</b>
\par The output in TmpStore would be:
\code
{
	/Mapper {
		/NewOutputKey {
			123
			/a bcd
		}
	}
	/AnotherOutputKey {
		123
		/a bcd
	}
}
\endcode

Note that the slot \b ResultsInNoOutput does not exist because the LookupPath of \b NonExistingSlot resulted in a Null-Anything and therefore no output slot will be created!
 */
class AnythingLookupPathResultMapper : public ResultMapper
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	AnythingLookupPathResultMapper(const char *name)
		: ResultMapper(name) {}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) AnythingLookupPathResultMapper(fName);
	}

protected:
	//! use slotname as Anything::LookupPath() argument within value and call mapper script for slotname using the new value looked up
	/*! @copydoc ResultMapper::DoPutAnyWithSlotname() */
	virtual bool DoPutAnyWithSlotname(const char *key, Anything &value, Context &ctx, ROAnything script, const char *slotname);
};
#endif
