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

class FixedSizeMapper: public EagerParameterMapper {
	FixedSizeMapper();
	FixedSizeMapper(const FixedSizeMapper &);
	FixedSizeMapper &operator=(const FixedSizeMapper &);
// special mapper generating a fixed width input
public:
	FixedSizeMapper(const char *name) :
			EagerParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) FixedSizeMapper(fName);
	}

protected:
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything info);
};

//! renders value in uppercase on stream
class UpperCaseMapper: public EagerParameterMapper {
	UpperCaseMapper();
	UpperCaseMapper(const UpperCaseMapper &);
	UpperCaseMapper &operator=(const UpperCaseMapper &);
public:
	UpperCaseMapper(const char *name) :
			EagerParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) UpperCaseMapper(fName);
	}

protected:
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything config);
};

//! @deprecated special mapper with a strange behavior, not really needed any more with modern mapper scripting
/*! uses slot /LookupName "aMapperName" and delegates to mapper aMapperName if found
 * new scripting mechanics allows almost the same with /key { /aMapperName literalconfig }
 */
class LookupMapper: public EagerParameterMapper {
	LookupMapper();
	LookupMapper(const LookupMapper &);
	LookupMapper &operator=(const LookupMapper &);
public:
	LookupMapper(const char *name) :
			EagerParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) LookupMapper(fName);
	}

protected:
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything config);
};

class RendererMapper: public EagerParameterMapper {
	RendererMapper();
	RendererMapper(const RendererMapper &);
	RendererMapper &operator=(const RendererMapper &);
public:
	RendererMapper(const char *name) :
			EagerParameterMapper(name) {
	}

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RendererMapper(fName);
	}

	//needed, because we are non-eager when retrieving an Anything (why?)
	virtual bool Get(const char *key, Anything &value, Context &ctx);
protected:
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything info);
	virtual bool DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything info);
};

#endif
