/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NameUsingOutputMapper_H
#define _NameUsingOutputMapper_H

//---- Mapper include -------------------------------------------------
#include "config_dataaccess.h"
#include "Mapper.h"

//---- NameUsingOutputMapper ----------------------------------------------------------
//! Stores values to put below a configured slot or its aliased name in TempStore
/*!
\par Configuration
\code
{
	/Destination {							optional
		/Store			String				mandatory, default TempStore [Role|Session|Request|TempStore], name of the destination store
		/Slot			String				qualified slot name
		/Delim   		String				optional, default ".", first char is taken as delimiter for named slots
		/IndexDelim		String				optional, default ":", first char is taken as delimiter for indexed slots
	}
}
\endcode

Default configuration is the mappers alias name in TempStore
*/
class EXPORTDECL_DATAACCESS NameUsingOutputMapper : public EagerResultMapper
{
public:
	NameUsingOutputMapper(const char *name);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything config);
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);

protected:
	virtual Anything GetDestination(Context &ctx, ROAnything config);
	virtual ROAnything GetDestinationConfig(Context &ctx, ROAnything config );

private:
	NameUsingOutputMapper();
	NameUsingOutputMapper(const NameUsingOutputMapper &);
	NameUsingOutputMapper &operator=(const NameUsingOutputMapper &);
};

#endif
