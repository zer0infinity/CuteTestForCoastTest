/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SortedTagsHTMLMapper_h_
#define _SortedTagsHTMLMapper_h_

#include "Mapper.h"

class SortedTagsHTMLMapper: public ResultMapper {
	SortedTagsHTMLMapper();
	SortedTagsHTMLMapper(const SortedTagsHTMLMapper &);
	SortedTagsHTMLMapper &operator=(const SortedTagsHTMLMapper &);
public:
	SortedTagsHTMLMapper(const char *SortedTagsHTMLMapperName) :
		ResultMapper(SortedTagsHTMLMapperName) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SortedTagsHTMLMapper(fName);
	}
protected:
	//! The HTML is parsed and the tags are put into an Anything, which is then stored in [MapperName][key]
	/*! @copydoc ResultMapper::DoPutAny() */
	virtual bool DoPutAny(const char *key, Anything &value, Context &ctx, ROAnything script);

	//! The HTML is parsed and the tags are put into an Anything, which is then stored in [MapperName][key]
	/*! @copydoc ResultMapper::DoPutStream() */
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);
};

#endif
