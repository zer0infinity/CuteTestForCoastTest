/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataMapper_H
#define _DataMapper_H

#include "config_dataaccess.h"
#include "Mapper.h"

//---- ListDataMapper -------------------------------------------------------------------
//! FIXME: somehow strange behavior, inconsistent with new semantics, very special case
class EXPORTDECL_DATAACCESS ListDataMapper : public ResultMapper
{
// special list mapper
//	prepares output into context tmpstore data as list of records
//  {
//		/MapperName {
//			{
//				/key0	value0
//				/key1	value1
//				...
//			}
//			{
//				/key0	value0
//				/key1	value1
//				...
//			}
//			...
//		}
//	}
public:
	ListDataMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	virtual bool DoPutAny(const char *key, Anything value, Context &ctx, ROAnything script);

private:
	ListDataMapper();
	ListDataMapper(const ListDataMapper &);
	ListDataMapper &operator=(const ListDataMapper &);
};

//---- FixedSizeMapper -------------------------------------------------------------------
class EXPORTDECL_DATAACCESS FixedSizeMapper : public EagerParameterMapper
{
// special mapper generating a fixed width input
public:
	FixedSizeMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything info);

protected:
	virtual void Pad(String &value, long length, char pad);
private:
	FixedSizeMapper();
	FixedSizeMapper(const FixedSizeMapper &);
	FixedSizeMapper &operator=(const FixedSizeMapper &);
};

//---- UpperCaseMapper -------------------------------------------------------------------
//! renders value in uppercase on stream
class EXPORTDECL_DATAACCESS UpperCaseMapper : public EagerParameterMapper
{
public:
	UpperCaseMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything config);

private:
	UpperCaseMapper();
	UpperCaseMapper(const UpperCaseMapper &);
	UpperCaseMapper &operator=(const UpperCaseMapper &);
};

//---- LookupMapper -------------------------------------------------------------------
class EXPORTDECL_DATAACCESS LookupMapper : public EagerParameterMapper
{

//! special mapper with a strange behavior, not really needed any more with modern mapper scripting
//! uses slot /LookupName "aMapperName" and delegates to mapper aMapperName if found
//! new scripting mechanics allows almost the same with /key { /aMapperName literalconfig }
public:
	LookupMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything config);

private:
	LookupMapper();
	LookupMapper(const LookupMapper &);
	LookupMapper &operator=(const LookupMapper &);
};
//------ ResultLookupMapper (splittet "OutputMapper" from LookupMapper
//! legacy mapper, no longer very useful with new mapper scripting
//! semantics can be achieved via /key { /OtherMapperName othermapperconfig }
class EXPORTDECL_DATAACCESS ResultLookupMapper : public EagerResultMapper
{

//
public:
	ResultLookupMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	virtual bool DoPutStream(const char *key, istream &is, Context &ctx, ROAnything config);

private:
	ResultLookupMapper();
	ResultLookupMapper(const ResultLookupMapper &);
	ResultLookupMapper &operator=(const ResultLookupMapper &);
};

//---- RendererMapper -------------------------------------------------------------------
class EXPORTDECL_DATAACCESS RendererMapper : public EagerParameterMapper
{

public:
	RendererMapper(const char *name);

	IFAObject *Clone() const;			// support for prototype

	//needed, because we are non-eager when retrieving an Anything (why?)
	virtual bool Get(const char *key, Anything &value, Context &ctx);
	virtual bool DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything info);
	virtual bool DoGetStream(const char *key, ostream &os, Context &ctx, ROAnything info);

private:
	RendererMapper();
	RendererMapper(const RendererMapper &);
	RendererMapper &operator=(const RendererMapper &);

};

#endif
