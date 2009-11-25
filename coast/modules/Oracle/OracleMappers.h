/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _OracleMappers_H
#define _OracleMappers_H

//---- ParameterMapper include -------------------------------------------------
#include "config_coastoracle.h"
#include "Mapper.h"

//---- OracleParameterMapper ----------------------------------------------------------
//! <B>Mapper able to handle Oracle specific stored procedure/function parameters</B>
/*!
 * This mapper can be used for any OracleDAImpl specific query but it is required for PL/SQL queries. To separate
 * between Parameter values and others, all Parameter values need to be placed below slot \c /Params in the parameter
 * mapper configuration.
 * If a requested parameter key is not found using the full Params.key it will be searched using key only. This was
 * implemented to simplify usage under some circumstances. Please note that all slot names in the Params section need
 * to be in upper case letter as oracle is internally only uses upper case names.
 *
 * @section opms1 OracleParameterMapper configuration
 *
\code
{
	/Name
	/Params {
		/<PARAMNAME>
		...
	}
}
\endcode
 *
 * @par \c Name
 * \b mandatory \n
 * Name of stored procedure/function to execute
 * @par \c Params \c .PARAMNAME
 * \b mandatory if IN or INOUT parameter values are used in call\n
 * IN or INOUT parameter name and its value. The name must match with the parameter name of the declared stored
 * procedure/function and must be written in all upper case letters!
*/
class EXPORTDECL_COASTORACLE OracleParameterMapper : public ParameterMapper
{
public:
	/*! Default registering ctor using a unique name to register mapper with
	 * @param name Mapper gets registered using this name
	 */
	OracleParameterMapper(const char *name);

protected:
	/*! major hook method for subclasses, default does script interpretation
	 * @param key Used to lookup a value in the Context
	 * @param value Returned value
	 * @param ctx The context used to perform input mapping
	 * @param script to be interpreted if any, for subclasses this is the config to use
	 * @return returns true if the mapping was successful otherwise false
	 */
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	/*! Clone interface implementation
	 * @return Pointer to IFAObject base class
	 */
	IFAObject *Clone() const;
private:
	OracleParameterMapper();
	OracleParameterMapper(const OracleParameterMapper &);
	OracleParameterMapper &operator=(const OracleParameterMapper &);
};

//---- OracleResultMapper ----------------------------------------------------------
//! <b>Mapper used to process Oracle specific output mappings</b>
/*!
 * This mapper should be used for any OracleDAImpl.
 *
 * The main feature of this mapper is to provide non-prefixed mapping if the prefixed key is not available
 * in an OracleResultMapper derived configuration. The non-prefixed slot can then be seen as if it was
 * a default mapping specification when there is not a specialized one.
 *
 * @section orms1 OracleResultMapper Configuration
 *
\code
{
	/key			<== non-prefixed key
	...
	/<PARAMNAME> {
		/key		<== PARAMNAME prefixed key
		...
	}
}
\endcode
 *
 * @par \c key
 * Optional, mapper specification to execute when key was used in ResultMapper::Put()\n
 * If a prefixed key was used, separator used is \c Delim configuration entry or '.' as default, the prefix
 * part in the key gets removed and DoPut() using the modified/stripped key will get called again.
 *
 * @par \c PARAMNAME \c .key
 * Optional, specialized MapperSpec for prefixed key
*/
class EXPORTDECL_COASTORACLE OracleResultMapper : public ResultMapper
{
public:
	/*! Default registering ctor using a unique name to register mapper with
	 * @param name Mapper gets registered using this name
	 */
	OracleResultMapper(const char *name);

protected:
	/*! Major hook for subclasses that want to do something with their config passed as script. The default is to interpret the script and put a value for every script item used. Recursion will be stopped by DoFinalPutAny which places its value under slot key below given DoGetDestinationSlot()
	 * @param key the key usually defines the associated kind of output-value
	 * @param value the value to be mapped
	 * @param ctx The context to put values into
	 * @param script current mapper configuration as ROAnything
	 * @return returns true if the mapping was successful otherwise false */
	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

	//! Looks up the Anything at key in Context using SlotFinder
	/*!	\param key the key usually defines the associated kind of output-value
		\param targetAny Anything reference into TmpStore to finally put values at. It uses DestinationSlot and key to get the correct location in Context.
		\param ctx the context of the invocation */
	virtual void DoGetDestinationAny( const char *key, Anything &targetAny, Context &ctx );

	/*! Clone interface implementation
	 * @return Pointer to IFAObject base class
	 */
	IFAObject *Clone() const;

private:
	OracleResultMapper();
	OracleResultMapper(const OracleResultMapper &);
	OracleResultMapper &operator=(const OracleResultMapper &);
};

#endif
