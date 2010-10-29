/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServiceHandler_h_
#define _ServiceHandler_h_

#include "config_wdbase.h"
#include "WDModule.h"
#include "Context.h"

//---- ServiceHandlersModule -----------------------------------------------------------
class EXPORTDECL_WDBASE ServiceHandlersModule : public WDModule
{
public:
	ServiceHandlersModule(const char *);
	virtual ~ServiceHandlersModule();

	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything );
	virtual bool Finis();
};

//---- ServiceHandler -----------------------------------------------------------
//!handles requests of a certain kind by providing a service
class EXPORTDECL_WDBASE ServiceHandler : public HierarchConfNamed
{
public:
	//!default named object constructor
	ServiceHandler(const char *serviceHandlerName);

	//!destructor does nothing
	virtual ~ServiceHandler();

	//!registry interface
	RegCacheDef(ServiceHandler);	// FindServiceHandler()

	//!handles requested service
	bool HandleService(std::ostream &os, Context &ctx);

	static const char* gpcCategory;
	static const char* gpcConfigPath;

protected:
	//!subclass hook to implement service handling
	virtual bool DoHandleService(std::ostream &os, Context &ctx) = 0;

private:
	//! block the following default elements of this class because they're not allowed to be used
	ServiceHandler();
	//! block the following default elements of this class because they're not allowed to be used
	ServiceHandler(const ServiceHandler &);
	//! block the following default elements of this class because they're not allowed to be used
	ServiceHandler &operator=(const ServiceHandler &);
};

#define RegisterServiceHandler(name) RegisterObject(name, ServiceHandler)

#endif
