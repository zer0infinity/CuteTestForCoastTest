/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _SortedTagsHTMLMapper_h_
#define _SortedTagsHTMLMapper_h_

#include "config_perftest.h"
#include "Mapper.h"

//---- SortedTagsHTMLMapper -----------------------------------------------------------

class EXPORTDECL_PERFTEST SortedTagsHTMLMapper : public ResultMapper
{
public:
	SortedTagsHTMLMapper(const char *SortedTagsHTMLMapperName) : ResultMapper(SortedTagsHTMLMapperName) {};
	~SortedTagsHTMLMapper() {}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) SortedTagsHTMLMapper(fName);
	}

	//! reads an HTML document from istream
	//! The HTML is parsed and the tags are put into an Anything, which is then stored in [MapperName][key]
	//! \param key defines the target slotname under tmp.<MapperName>
	//! \param is the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param config ignored
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, istream &is, Context &ctx, ROAnything config);

	//! reads an HTML document out of a string
	//! creates a stream out of the string and calls DoPutStream (see above)
	virtual bool Put(const char *key, const String &value, Context &ctx);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	SortedTagsHTMLMapper();
	SortedTagsHTMLMapper(const SortedTagsHTMLMapper &);
	SortedTagsHTMLMapper &operator=(const SortedTagsHTMLMapper &);

};

#endif
