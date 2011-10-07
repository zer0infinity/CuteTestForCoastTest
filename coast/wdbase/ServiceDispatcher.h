/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServiceDispatcher_h_
#define _ServiceDispatcher_h_

#include "Context.h"
#include "WDModule.h"
#include "Registry.h"

class ServiceDispatchersModule: public WDModule {
public:
	ServiceDispatchersModule(const char *name) :
		WDModule(name) {
	}
	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything);
	virtual bool Finis();
};

class ServiceHandler;
//!dispatches handling of the request to a service handler using context information
//!standard implementation looks for "DefaultService" entry in Context and uses "WebAppService" if nothing is found<br>
//!no rendering takes place for efficiency reason
class ServiceDispatcher: public HierarchConfNamed {
	ServiceDispatcher();
	ServiceDispatcher(const ServiceDispatcher &);
	ServiceDispatcher &operator=(const ServiceDispatcher &);
public:
	//!standard named object constructor
	ServiceDispatcher(const char *serviceDispatcherName) :
		HierarchConfNamed(serviceDispatcherName) {
	}
	//!registry interface
	RegCacheDef(ServiceDispatcher);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) ServiceDispatcher(fName);
	}

	//!dispatches control to a service handler
	virtual bool Dispatch2Service(std::ostream &reply, Context &ctx);

	//!finds service
	virtual ServiceHandler *FindServiceHandler(Context &ctx);

	//!finds service name in context it looks up the "DefaultService" tag in the context; no rendering is used
	virtual String FindServiceName(Context &ctx);
};

//!uses a table to map uri prefixes to rendered service names;
//!a table driven mapping is used to render service names<br>
//!the uri prefix of the request is mapped to a service name using a map defined in the context with the tag <b>URIPrefix2ServiceMap</b><br>
//!the service name can be a renderer specification<br>
//!if nothing matches the <b>DefaultService</b> Entry is used
class RendererDispatcher: public ServiceDispatcher {
	RendererDispatcher();
	RendererDispatcher(const RendererDispatcher &);
	RendererDispatcher &operator=(const RendererDispatcher &);
public:
	RendererDispatcher(const char *rendererDispatcherName) :
		ServiceDispatcher(rendererDispatcherName) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RendererDispatcher(fName);
	}
	//!uses a table to map uri prefixes to service name; renders entry found, if nothing matches, renders "DefaultService"
	virtual String FindServiceName(Context &ctx);
protected:
	//!Workhorse of FindServiceName
	long FindURIPrefixInList(const String &requestURI, const ROAnything &uriPrefixList);
};

#define RegisterServiceDispatcher(name) RegisterObject(name, ServiceDispatcher)

#endif
