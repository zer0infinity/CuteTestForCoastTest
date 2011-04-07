/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServiceHandler_h_
#define _ServiceHandler_h_

#include "WDModule.h"
#include "Context.h"
#include "Registry.h"

class ServiceHandlersModule: public WDModule {
public:
	ServiceHandlersModule(const char *name) :
		WDModule(name) {
	}
	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything);
	virtual bool Finis();
};

//!handles requests of a certain kind by providing a service
class ServiceHandler: public HierarchConfNamed {
	ServiceHandler();
	ServiceHandler(const ServiceHandler &);
	ServiceHandler &operator=(const ServiceHandler &);
public:
	//!default named object constructor
	ServiceHandler(const char *serviceHandlerName);
    //!registry interface
	RegCacheImplInline(ServiceHandler);

	//!handles requested service
	bool HandleService(std::ostream &os, Context &ctx);

	static const char* gpcCategory;
	static const char* gpcConfigPath;
protected:
	//!subclass hook to implement service handling
	virtual bool DoHandleService(std::ostream &os, Context &ctx) = 0;
};

#define RegisterServiceHandler(name) RegisterObject(name, ServiceHandler)

#endif
