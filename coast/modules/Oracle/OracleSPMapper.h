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
 * @par Description
 * --
 * @par Configuration
\code
{
	/Name				MapperSpec		mandatory, name of stored procedure/function to execute
	/Params {							mandatory if IN or INOUT parameter values are used in call
		/<PARAMNAME>	MapperSpec		mandatory, IN or INOUT parameter name and its value. The name must match with
											the parameter name of the declared stored procedure/function and must be
											written in all upper case letters!
		...
	}
}
\endcode
\note Parameter names need to be written in all upper case letters, otherwise the implemented mapping will not work -
 an appropriate message will be written into the Messages slot.
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

private:
	OracleSPMapper();
	OracleSPMapper(const OracleSPMapper &);
	OracleSPMapper &operator=(const OracleSPMapper &);
};

//---- OracleResultMapper ----------------------------------------------------------
//! <B>provide OracleDAImpl with stored procedure/function specific output mapping</B>
/*!
 * @par Description
 * --
 * @par Configuration
\code
{
	/QueryResult 		MapperSpec			optional, specify default behavior when results get put,
												when a stored procedure parameter gets put - usually as PARAMNAME.QueryResult -
												and it is not specialized like below, this MapperSpec will get used
	/QueryTitles		MapperSpec			optional, as above
	/QueryCount	 		MapperSpec			optional, as above

	/<PARAMNAME> {							optional, specialize MapperSpec for named stored procedure/function parameter
		/QueryResult 	MapperSpec			optional, MapperSpec to use when PARAMNAME.QueryResult gets put
		/QueryTitles	MapperSpec			optional, as above
		/QueryCount	 	MapperSpec			optional, as above
	}
}
\endcode
\note Parameter names need to be written in all upper case letters, otherwise the implemented mapping will not work.
*/
class EXPORTDECL_COASTORACLE OracleResultMapper : public ResultMapper
{
public:
	//--- constructors
	OracleResultMapper(const char *name);
	//--- support for prototype
	IFAObject *Clone() const;

protected:
	// decide which hook to overwrite
	//! Major hook for subclasses that want to do something with their config passed as script. The default is to interpret the script and put a value for every script item used. Recursion will be stopped by DoFinalPutAny which places its value under slot key below given DoGetDestinationSlot()
	/*! \param key the key usually defines the associated kind of output-value
		\param value the value to be mapped
		\param ctx the context of the invocation
		\param script current mapper configuration as ROAnything
		\return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

private:
	OracleResultMapper();
	OracleResultMapper(const OracleResultMapper &);
	OracleResultMapper &operator=(const OracleResultMapper &);
};

#endif
