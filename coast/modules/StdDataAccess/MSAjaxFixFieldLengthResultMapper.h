/*
 * Copyright (c) 2011, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 *
 * Author(s): m1huber
 */

#ifndef MSAJAXFIXFIELDLENGTHRESULTMAPPER_H_
#define MSAJAXFIXFIELDLENGTHRESULTMAPPER_H_

#include "Mapper.h"

class MSAjaxFixFieldLengthResultMapper: public ResultMapper {
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	MSAjaxFixFieldLengthResultMapper(const char *name) :
		ResultMapper(name) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) MSAjaxFixFieldLengthResultMapper(fName);
	}
protected:
	//! potentially correct the length fields of the MSAjax structure
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! potentially correct the length fields of the MSAjax structure
	/*! @copydoc ResultMapper::DoPutStream(const char *, std::istream &, Context &, ROAnything) */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);
};

#endif /* MSAJAXFIXFIELDLENGTHRESULTMAPPER_H_ */
