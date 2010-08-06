/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _NullParameterMapper_H
#define _NullParameterMapper_H

#include "config_coastoracle.h"
#include "Mapper.h"

//---- NullParameterMapper ----------------------------------------------------------
//! Converts the value of a mapped key to AnyNullImpl if its original value is listed in the \c TreatAsNull slot
/*!
 * This mapper can be used whenever a value conversion to AnyNullImpl is needed based on the String value of a mapped key.
 * Specify values to be treated as Null in the slot \em TreatAsNull .
 *
 * @section NullParameterMapperConfiguration Configuration
 *
\code
{
	/TreatAsNull {
		"ThisEvaluatesToAnyNull"
		...
	}
}
\endcode
 *
 * @par \c TreatAsNull
 * \b optional \n
 * List of String entries to be converted to AnyNullImpl as result of the ParameterMapper::Get() call
 *
 * @section NullParameterMapperExample Example
 *
\code
/MyNullMapper {
	/TreatAsNull {
		"null"
		"NULL"
		""
	}
}
\endcode
 *
 */
class EXPORTDECL_COASTORACLE NullParameterMapper : public ParameterMapper
{
public:
	/*! Default registering ctor using a unique name to register mapper with
	 * @param name Mapper gets registered using this name
	 */
	NullParameterMapper(const char *name);

protected:
	//! Set returned value to AnyNullImpl if value retrieved using ParameterMapper::DoFinalGetAny() matches any value in \c TreatAsNull list.
	/*! @copydetails ParameterMapper::DoFinalGetAny()
	 */
//	virtual bool DoFinalGetAny(const char *key, Anything &value, Context &ctx);
	virtual bool DoGetAny( const char *key, Anything &value, Context &ctx, ROAnything script );

	/*! Clone interface implementation
	 * @return Pointer to IFAObject base class
	 */
	IFAObject *Clone() const;
private:
	NullParameterMapper();
	NullParameterMapper(const NullParameterMapper &);
	NullParameterMapper &operator=(const NullParameterMapper &);
};

#endif
