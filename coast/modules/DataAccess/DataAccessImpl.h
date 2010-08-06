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
//! WDModule to initialize, configure and register DataAccessImpl objects
/*! @section DataAccessImplsModuleDescription Description
 * To initialize DataAccessImpl objects, this module should be listed in the \c /Modules section
 * of your main configuration file.
 * Module specific entries are expected within the subslot as defined in DataAccessImpl::gpcConfigPath.
 *
 * @subsection DataAccessImplsModulePurpose Purpose
    Definition of a root class for data access implementation.
    It manages all the implementation subclasses in a registry.
    This class defines the protocol for getting data in and out of a
    persistency layer.
 *
 * @subsection DataAccessImplsModuleConcurrency Concurrency
	DataAccessImpls are used in a MT-Environment. One thread per request.
    If a DataAccessImpl holds state in order to process a single request,
    it is not thread safe.
    Therefore it has to be allocated and deleted on a request basis.
 *
 * @subsection DataAccessImplsModuleCollaborations Collaborations
    An DataAccessImpl is called from the DataAccess object.
    It calls mapper in order to move the data in and out.
 *
 * @subsection DataAccessImplsModuleErrorhandling Errorhandling
    Errorhandling is implicit e.g. via parameters moved in and out by the mappers
 *
 * @subsection DataAccessImplsModuleAssumptions Assumptions
    The transaction name is unique in the system.
 */
class EXPORTDECL_DATAACCESS DataAccessImplsModule : public WDModule
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	DataAccessImplsModule(const char *name);

	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything );
	virtual bool Finis();
};

//--- DataAccessImpl --------------------------------------------------
//! Base class for implementing access to data delivery services
/*! Concrete classes need to implement the Exec() function and perform the access in one call.
 * Parameters needed to configure the data access operation should be retrieved using the supplied ParameterMapper,
 * results should be stored away using the supplied ResultMapper. */
class EXPORTDECL_DATAACCESS DataAccessImpl : public HierarchConfNamed
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	DataAccessImpl(const char *name);

	static const char* gpcCategory;
	static const char* gpcConfigPath;
	//! Name of the configuration Anything file
	static const char* gpcConfigFileName;

	/*! @copydoc IFAObject::Clone() */
	IFAObject *Clone() const;

	//!protocol provided to implement data access
	/*! @param ctx The context in which the transaction takes place
	 * @param input ParameterMapper object that is mapping data from the client space to the data access object on request
	 * @param output ResultMapper object that maps the result of the access back into client space */
	virtual bool Exec(Context &ctx, ParameterMapper *input, ResultMapper *output);

	//--- Registration
	RegCacheDef(DataAccessImpl);	// FindDataAccessImpl()

protected:
	//! Name of the file in which configurations for DataAccessImpl objects are stored, \ref gpcConfigFileName
	/*! @copydetails ConfNamedObject::DoGetConfigName() */
	virtual bool DoGetConfigName(const char *category, const char *objName, String &configFileName) const;

	//! DataAccessImpl objects will get initialized using their named configuration from within the common configuration file
	/*! @copydetails ConfNamedObject::DoLoadConfig() */
	virtual bool DoLoadConfig(const char *category);

private:
	DataAccessImpl();
	DataAccessImpl(const DataAccessImpl &);
	DataAccessImpl &operator=(const DataAccessImpl &);
};

//--- LoopBackDAImpl ---------------------------------------------------
//! Simple loop back DataAccessImpl to transfer String or StringStream values from input to output
/*! LoopBackDAImpl can be useful when wanting to test ParameterMapper or ResultMapper implementations.
@code
{
	/transfer {					config entry to specify slots to transfer
		/FromSlot1	ToSlot1		using ParameterMapper->Get to lookup FromSlot and
								putting it using ResultMapper->Put to store it at ToSlot
		/FromSlot2	ToSlot2		...
		...
	}
	/Streaming					optional, 0L/1L/2L/3L, default 0L, defining streaming mode
								0L or default: no streaming used, using Get(FromSlot, String, Context)
								1L: ParameterMapper->Get using StringStream, eg. Get(FromSlot, ostream, Context, ROAnything)
									ResultMapper->Put using String, eg. Put(ToSlot, String, Context)
								2L: ParameterMapper->Get using String, eg. Get(FromSlot, String, Context)
									ResultMapper->Put using StringStream, eg. Put(ToSlot, istream, Context, ROAnything)
								3L: ParameterMapper->Get using StringStream, eg. Get(FromSlot, ostream, Context, ROAnything)
									ResultMapper->Put using StringStream, eg. Put(ToSlot, istream, Context, ROAnything)
}
@endcode
 */
class EXPORTDECL_DATAACCESS LoopBackDAImpl: public DataAccessImpl
{
public:
	/*! @copydoc RegisterableObject::RegisterableObject(const char *) */
	LoopBackDAImpl(const char *name) : DataAccessImpl(name) {}

	/*! @copydoc IFAObject::Clone() */
	IFAObject *Clone() const;

	//! Loopback specific implementation of the data access
	/*! @copydetails DataAccessImpl::Exec() */
	virtual bool Exec(Context &ctx, ParameterMapper *input, ResultMapper *output);

private:
	LoopBackDAImpl();
	LoopBackDAImpl(const LoopBackDAImpl &);
	LoopBackDAImpl &operator=(const LoopBackDAImpl &);
};

#define RegisterDataAccessImpl(name) RegisterObject(name, DataAccessImpl)

#endif		//not defined _DataAccessImpl_H
