/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DataAccessImpl_H
#define _DataAccessImpl_H

#include "config_dataaccess.h"
#include "Mapper.h"

class Registry;

//---- DataAccessImplsModule -----------------------------------------------------------
class EXPORTDECL_DATAACCESS DataAccessImplsModule : public WDModule
{
public:
	DataAccessImplsModule(const char *name);
	~DataAccessImplsModule();

	virtual bool Init(const Anything &config);
	virtual bool ResetFinis(const Anything &);
	virtual bool Finis();
};

//--- DataAccessImpl --------------------------------------------------
//! base class for classes implementing access to data delivery services
//! it has single api Exec that must implement the access in one call
//! if data access is conversational, the conversation taking place is done through
//! the mapper objects
class EXPORTDECL_DATAACCESS DataAccessImpl : public HierarchConfNamed
{
public:
	DataAccessImpl(const char *name);
	~DataAccessImpl();

	//!factory method to create a generic data access object, not very useful
	IFAObject *Clone() const;

	//!protocol provided to implement data access
	//! \param c The context in with the transaction takes place
	//! \param trxname the name of the data access entity
	//! \param input a mapper object that is mapping data from the client space to the data access object on request
	//! \param output a mapper object that maps the result of the access back into client space
	virtual bool Exec(Context &c, InputMapper *input, OutputMapper *output);

	//! find the InputMapper by Name, look first into own config, otherwise into
	//! registered mappers with my own name
	//! \param daName the name of the mapper sought
	//! \param context the context of this data access call
	virtual InputMapper *GetInputMapper(Context &context);

	//! find the outputMapper by Name
	//! \param daName the name of the mapper sought
	//! \param context the context of this data access call
	virtual OutputMapper *GetOutputMapper(Context &context);
	//--- Registration
	RegCacheDef(DataAccessImpl);	// FindDataAccessImpl()
protected:

	// generate the config file name (without extension, which is assumed to be any)
	// out of category and objName
	// the default implementation just takes the objName
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName);

	// load an anything and make it available by storing a reference in fConfig
	// the default implementation uses the cache handler
	// if you provide your own implementation be careful about the lifetime of
	// the loaded anything otherwise fConfig points to invalid data
	virtual bool DoLoadConfig(const char *category);

	// log error message to context and syslog
	void HandleError(Context &context, const String &mapperName, const char *file, long line, const String &msg);

	friend class DataAccessImplTest;

private:
	DataAccessImpl();
	DataAccessImpl(const DataAccessImpl &);
	DataAccessImpl &operator=(const DataAccessImpl &);
};

//--- LoopBackDAImpl ---------------------------------------------------

//	Structure of config:
//<PRE>	{
//		/transfer {					config entry to specify slots to transfer
//			/FromSlot1	ToSlot1		using InputMapper->Get to lookup FromSlot and
//									putting it using OutputMapper->Put to store it at ToSlot
//			/FromSlot2	ToSlot2		...
//			...
//		}
//		/Streaming					optional, 0L/1L/2L/3L, default 0L, defining streaming mode
//									0L or default: no streaming used, using Get(FromSlot, String, Context)
//									1L: InputMapper->Get using StringStream, eg. Get(FromSlot, ostream, Context, ROAnything)
//										OutputMapper->Put using String, eg. Put(ToSlot, String, Context)
//									2L: InputMapper->Get using String, eg. Get(FromSlot, String, Context)
//										OutputMapper->Put using StringStream, eg. Put(ToSlot, istream, Context, ROAnything)
//									3L: InputMapper->Get using StringStream, eg. Get(FromSlot, ostream, Context, ROAnything)
//										OutputMapper->Put using StringStream, eg. Put(ToSlot, istream, Context, ROAnything)
//	}
class EXPORTDECL_DATAACCESS LoopBackDAImpl: public DataAccessImpl
{
public:
	LoopBackDAImpl(const char *name);
	~LoopBackDAImpl();

	//!factory method to create a generic data access object, not very useful
	IFAObject *Clone() const;

	//!protocol provided to implement data access
	//! \param c The context in with the transaction takes place
	//! \param trxname the name of the data access entity
	//! \param input a mapper object that is mapping data from the client space to the data access object on request
	//! \param output a mapper object that maps the result of the access back into client space
	virtual bool Exec(Context &c, InputMapper *input, OutputMapper *output);

private:
	LoopBackDAImpl();
	LoopBackDAImpl(const LoopBackDAImpl &);
	LoopBackDAImpl &operator=(const LoopBackDAImpl &);
};

#define RegisterDataAccessImpl(name) RegisterObject(name, DataAccessImpl)

#endif		//not defined _DataAccessImpl_H
