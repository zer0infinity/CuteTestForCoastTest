/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataMapper_H
#define _DataMapper_H

#include "Mapper.h"

//---- FixedSizeMapper -------------------------------------------------------------------
class FixedSizeMapper : public EagerParameterMapper
{
// special mapper generating a fixed width input
public:
	FixedSizeMapper(const char *name);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything info);

protected:
	virtual void Pad(String &value, long length, char pad);
private:
	FixedSizeMapper();
	FixedSizeMapper(const FixedSizeMapper &);
	FixedSizeMapper &operator=(const FixedSizeMapper &);
};

//---- UpperCaseMapper -------------------------------------------------------------------
//! renders value in uppercase on stream
class UpperCaseMapper : public EagerParameterMapper
{
public:
	UpperCaseMapper(const char *name);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything config);

private:
	UpperCaseMapper();
	UpperCaseMapper(const UpperCaseMapper &);
	UpperCaseMapper &operator=(const UpperCaseMapper &);
};

//---- LookupMapper -------------------------------------------------------------------
//! @deprecated special mapper with a strange behavior, not really needed any more with modern mapper scripting
/*! uses slot /LookupName "aMapperName" and delegates to mapper aMapperName if found
 * new scripting mechanics allows almost the same with /key { /aMapperName literalconfig }
 */
class LookupMapper : public EagerParameterMapper
{
public:
	LookupMapper(const char *name);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything config);

private:
	LookupMapper();
	LookupMapper(const LookupMapper &);
	LookupMapper &operator=(const LookupMapper &);
};

//---- RendererMapper -------------------------------------------------------------------
class RendererMapper : public EagerParameterMapper
{
public:
	RendererMapper(const char *name);

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	//needed, because we are non-eager when retrieving an Anything (why?)
	virtual bool Get(const char *key, Anything &value, Context &ctx);
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything info);
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything info);

private:
	RendererMapper();
	RendererMapper(const RendererMapper &);
	RendererMapper &operator=(const RendererMapper &);

};

#endif
