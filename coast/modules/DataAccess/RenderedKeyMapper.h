/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RenderedKeyResultMapper_H
#define _RenderedKeyResultMapper_H

#include "Mapper.h"

//! Dynamically adjust the \c Put key according to Renderer specification
/*! They key used to put the value will be rendered according to the given specification prior to putting it.
 * To extend the flexibility of rendering, the value itself will be pushed into the Context to allow using entries of
 * the current value being put. Additionally the current key is also available in Context as \c MappedKey.
 * If required, the rendered key can also be stored in the Context, using the specification as given in slot \c StoreKeyAt.
 * @section rkrm Mapper configuration
\code
{
	/KeySpec		Renderer specification
	/StoreKeyAt		Renderer specification
	/Store 			String
	/<new-key>		ResultMapper specification
	/"*"			catch all ResultMapper specification
}
\endcode
 * @par \c KeySpec
 * Optional, defaults to mapped key\n
 * Renderer specification resulting in a new key to be used when putting the value Anything\n
 * The value Anything will be pushed into the Context prior to rendering. In addition, the current mapping key will also be available as \c MappedKey
 *
 * @par \c StoreKeyAt
 * Optional, default is to not store the newly rendered key\n
 * Renderer specification resulting in a slot name conforming to Anything::LookupPath semantics to store rendered key in Context
 *
 * @par \c Store
 * Optional, defaults to TmpStore\n
 * Store the rendered key in a specific store in the context: \c Role -> RoleStore, \c Session -> SessionStore, \c Request
 *
 * @par \c <new-key>
 * Optional, resulting key used to further map the value\n
 * ResultMapper specification used to further map/Put() the current value with the new key
 *
 * @par \c "*"
 * Optional, catch all specification to be used for any key not explicitly caught\n
 * ResultMapper specification used to further map/Put() the current value with the new key
 *
 */
class RenderedKeyResultMapper: public ResultMapper {
	RenderedKeyResultMapper();
	RenderedKeyResultMapper(const RenderedKeyResultMapper &);
	RenderedKeyResultMapper &operator=(const RenderedKeyResultMapper &);
public:
	//--- constructors
	RenderedKeyResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RenderedKeyResultMapper(fName);
	}

protected:
	//! Put value according to newly rendered key
	/*! @copydoc ResultMapper::DoPutAny(const char *, Anything &, Context &, ROAnything) */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! Put \c is according to newly rendered key
	/*! @copydoc ResultMapper::DoPutStream() */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything script);

	//! Loop breaker, allow on the fly key renaming and call ResultMapper::DoPutAny() with \c slotname
	/*! @copydoc ResultMapper::DoPutAnyWithSlotname() */
	virtual bool DoPutAnyWithSlotname(const char *key, Anything &value, Context &ctx, ROAnything roaScript, const char *slotname);

	//! Loop breaker, allow on the fly key renaming and call ResultMapper::DoPutStream() with \c slotname
	/*! @copydoc ResultMapper::DoPutStreamWithSlotname() */
	virtual bool DoPutStreamWithSlotname(const char *key, std::istream &is, Context &ctx, ROAnything roaScript, const char *slotname);
};

//! Dynamically adjust the \c Get key according to Renderer specification
/*! They key used to get the value will be rendered according to the given specification prior to getting it.
 * To extend the flexibility of rendering the current key is also available in Context as \c MappedKey.
 * If required, the rendered key can also be stored in the Context, using the specification as given in slot \c StoreKeyAt.
 * @section rkpm Mapper configuration
\code
{
	/KeySpec		Renderer specification
	/StoreKeyAt		Renderer specification
	/Store 			String
}
\endcode
 * @par \c KeySpec
 * Optional, defaults to mapped key\n
 * Renderer specification resulting in a new key to be used when getting the value Anything\n
 * The current mapping key will be available as \c MappedKey
 *
 * @par \c StoreKeyAt
 * Optional, default is to not store the newly rendered key\n
 * Renderer specification resulting in a slot name conforming to Anything::LookupPath semantics to store rendered key in Context
 *
 * @par \c Store
 * Optional, defaults to TmpStore\n
 * Store the rendered key in a specific store in the context: \c Role -> RoleStore, \c Session -> SessionStore, \c Request
 */
class RenderedKeyParameterMapper: public ParameterMapper {
	RenderedKeyParameterMapper();
	RenderedKeyParameterMapper(const RenderedKeyParameterMapper &);
	RenderedKeyParameterMapper &operator=(const RenderedKeyParameterMapper &);
public:
	//--- constructors
	RenderedKeyParameterMapper(const char *name) :
		ParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RenderedKeyParameterMapper(fName);
	}

protected:
	//! Get value anything according to newly rendered key
	/*! @copydoc ParameterMapper::DoGetAny(const char *, Anything &, Context &, ROAnything) */
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script);
};

#endif
