/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _OracleSPMapper_H
#define _OracleSPMapper_H

//---- ParameterMapper include -------------------------------------------------
#include "config_coastoracle.h"
#include "Mapper.h"

//---- OracleSPMapper ----------------------------------------------------------
//! <B>provide OracleDAImpl with stored procedure/function specific parameters</B>
/*!
\par Configuration
\code
{
	/Name		MapperSpec				mandatory, name of stored procedure/function to execute
	/Params {							mandatory if IN or INOUT parameter values are used in call
		/<PARAMNAME>	MapperSpec		mandatory, IN or INOUT parameter name and its value. The name must match with the parameter name of the declared stored procedure/function and must be written in all uppercase letters!
		...
	}
}
\endcode
\note Parameter names need to be written in all uppercase letters, otherwise the implemented mapping will not work - an appropriate message will be written into the Messages slot.
*/
class EXPORTDECL_COASTORACLE OracleSPMapper : public ParameterMapper
{
public:
	//--- constructors
	OracleSPMapper(const char *name);
	//--- support for prototype
	IFAObject *Clone() const;

protected:
// decide which hook(s) you need to overwrite
	//! major hook method for subclasses, default does script interpretation
	/*! \param key the name defines kind of value to get or the slot in the script to use
		\param value collects data within script
		\param ctx the context of the invocation
		\param script to be interpreted if any, for subclasses this is the config to use
		\return returns true if the mapping was successful otherwise false */
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! major hook method for subclasses, default does script interpretation
	/*! \param key the name defines kind of values to write or the slot in the script to use
		\param os the stream to be mapped on
		\param ctx the context of the invocation
		\param script to be interpreted if any, for subclasses this is the config to use
		\return returns true if the mapping was successful otherwise false */
//	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything script);

	//! hook for recursion stopper in Mapper script interpretation, looks for value in context using the key and appends it to value
	/*! \param key the name defines kind of values to write or the slot in the script to use
		\param value container to put returned value into
		\param ctx the context of the invocation
		\return returns true if the mapping was successful otherwise false */
//	virtual bool DoFinalGetAny(const char *key, Anything &value, Context &ctx);

	//! Hook for recursion stopper in Mapper script interpretation. The default is to write value from context found via key to os. If the value is a composite anything the anything is Printed on os, otherwise the value is rendered literally as a string
	/*! \param key the name defines kind of values to write or the slot in the script to use
		\param os stream to put returned value into
		\param ctx the context of the invocation
		\return returns true if the mapping was successful otherwise false */
//	virtual bool DoFinalGetStream(const char *key, ostream &os, Context &ctx);

private:
	OracleSPMapper();
	OracleSPMapper(const OracleSPMapper &);
	OracleSPMapper &operator=(const OracleSPMapper &);
};

#endif
