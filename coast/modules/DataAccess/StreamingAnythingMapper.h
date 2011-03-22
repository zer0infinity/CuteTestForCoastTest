/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StreamingAnythingMapper_H
#define _StreamingAnythingMapper_H

#include "AnythingLookupPathResultMapper.h"

//---- StreamToAnythingMapper ----------------------------------------------------------
//! converts a stream into an Anythings and puts it into the context using an optional mapper script
/*!
Reads in an Anything from the client provided stream and stores it in the context's TmpStore under [MapperName][key] by default.

To further process the converted Anything one can provide a MapperScript which extracts and stores either parts of the converted Anything or the whole Anything.
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
class StreamToAnythingMapper : public AnythingLookupPathResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	StreamToAnythingMapper(const char *name)
		: AnythingLookupPathResultMapper(name) {}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) StreamToAnythingMapper(fName);
	}

protected:
	//! reads an Anything from istream and puts it according to key
	/*! @copydoc ResultMapper::DoPutStream() */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script);
};

//---- AnythingToStreamMapper ----------------------------------------------------------
//! streams Anythings out from context to a Stream
/*!
looks up 'key' in context and streams it on the client provided stream
 */
class AnythingToStreamMapper : public ParameterMapper
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	AnythingToStreamMapper(const char *name)
		: ParameterMapper(name) {}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) AnythingToStreamMapper(fName);
	}

protected:
	//! serialize an Anything retrieved from key to the output stream
	/*! The key is looked up in the context and the Anything found is streamed to os
		Clients use this method to write to the stream anythings associated with the key
		Key may be of the form Foo.Bar.Sequence (i.e. a path) */
	/*! @copydoc ResultMapper::DoPutAnyWithSlotname() */
	virtual bool DoFinalGetStream(const char *key, std::ostream &os, Context &ctx);
};
#endif
