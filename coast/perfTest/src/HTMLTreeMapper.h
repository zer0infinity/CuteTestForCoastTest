/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HTMLTreeMapper_h_
#define _HTMLTreeMapper_h_

#include "Mapper.h"

//---- HTMLTreeMapper -----------------------------------------------------------

class HTMLTreeMapper : public ResultMapper
{
public:
	HTMLTreeMapper(const char *name) : ResultMapper(name) {};
	~HTMLTreeMapper() {};

//-- Cloning interface
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTMLTreeMapper(fName);
	}

	//! reads an HTML document from istream
	//! The HTML is parsed and the tags are put into an Anything, which is then stored in Mapper.HTTPBody
	//! \param key - ignored, will default to HTTPBody
	//! \param is the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param config - ignored
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);

private:
	HTMLTreeMapper();
	HTMLTreeMapper(const HTMLTreeMapper &);
	HTMLTreeMapper &operator=(const HTMLTreeMapper &);

};
//---- HTMLTreeAndSortedTagsMapper -----------------------------------------------------------

class HTMLTreeAndSortedTagsMapper : public ResultMapper
{
public:
	HTMLTreeAndSortedTagsMapper(const char *name) : ResultMapper(name) {};
	~HTMLTreeAndSortedTagsMapper() {};

//-- Cloning interface
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) HTMLTreeAndSortedTagsMapper(fName);
	}

	//! reads an HTML document from istream
	//! The HTML is parsed and the tags are put into an Anything, which is then stored
	//! in MapperName.HTTPBody and MapperName.Output
	//! \param key - ignored
	//! \param is the stream to be mapped
	//! \param ctx the thread context of the invocation
	//! \param config - ignored
	//! \return returns true if the mapping was successful otherwise false
	virtual bool DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config);

private:
	HTMLTreeAndSortedTagsMapper();
	HTMLTreeAndSortedTagsMapper(const HTMLTreeAndSortedTagsMapper &);
	HTMLTreeAndSortedTagsMapper &operator=(const HTMLTreeAndSortedTagsMapper &);
};

#endif
