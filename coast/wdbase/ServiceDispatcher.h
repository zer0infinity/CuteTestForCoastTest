/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ServiceDispatcher_h_
#define _ServiceDispatcher_h_

#include "config_wdbase.h"
#include "Context.h"
#include "WDModule.h"

class ServiceHandler;

//---- ServiceDispatchersModule -----------------------------------------------------------
class EXPORTDECL_WDBASE ServiceDispatchersModule : public WDModule
{
public:
	ServiceDispatchersModule(const char *);
	~ServiceDispatchersModule();

	virtual bool Init(const ROAnything config);
	virtual bool ResetFinis(const ROAnything );
	virtual bool Finis();
};

//---- ServiceDispatcher -----------------------------------------------------------
//!dispatches handling of the request to a service handler using context information
//!standard implementation looks for "DefaultService" entry in Context and uses "WebAppService" if nothing is found<br>
//!no rendering takes place for efficiency reason
class EXPORTDECL_WDBASE ServiceDispatcher : public HierarchConfNamed
{
public:
	//!standard named object constructor
	ServiceDispatcher(const char *serviceDispatcherName);
	~ServiceDispatcher();

	//!registry interface
	RegCacheDef(ServiceDispatcher);	// FindServiceDispatcher()
	//!cloning interface
	IFAObject *Clone() const {
		return new ServiceDispatcher(fName);
	}

	//!dispatches control to a service handler
	virtual void Dispatch2Service(ostream &reply, Context &ctx);

	//!finds service
	virtual ServiceHandler *FindServiceHandler(Context &ctx);

	//!finds service name in context it looks up the "DefaultService" tag in the context; no rendering is used
	virtual String FindServiceName(Context &ctx);

private:
	//! block the following default elements of this class because they're not allowed to be used
	ServiceDispatcher();
	//! block the following default elements of this class because they're not allowed to be used
	ServiceDispatcher(const ServiceDispatcher &);
	//! block the following default elements of this class because they're not allowed to be used
	ServiceDispatcher &operator=(const ServiceDispatcher &);
};

//---- RendererDispatcher -----------------------------------------------------------
//!uses a table to map uri prefixes to rendered service names;
//!a table driven mapping is used to render service names<br>
//!the uri prefix of the request is mapped to a service name using a map defined in the context with the tag <b>URIPrefix2ServiceMap</b><br>
//!the service name can be a renderer specification<br>
//!if nothing matches the <b>DefaultService</b> Entry is used
class EXPORTDECL_WDBASE RendererDispatcher : public ServiceDispatcher
{
public:
	RendererDispatcher(const char *RendererDispatcherName);
	~RendererDispatcher();

	//!cloning interface
	IFAObject *Clone() const {
		return new RendererDispatcher(fName);
	}

	//!uses a table to map uri prefixes to service name; renders entry found, if nothing matches, renders "DefaultService"
	virtual String FindServiceName(Context &ctx);

protected:
	//!Workhorse of FindServiceName
	long FindURIPrefixInList(const String &requestURI, const ROAnything &uriPrefixList);
	void FindVlanInList(const String &requestVhost, const String &requestURI, const ROAnything &vhostList, long *matchedVhost, long *matchedVhostPrefix);
private:
	// block the following default elements of this class
	// because they're not allowed to be used
	RendererDispatcher();
	RendererDispatcher(const RendererDispatcher &);
	RendererDispatcher &operator=(const RendererDispatcher &);
};

#define RegisterServiceDispatcher(name) RegisterObject(name, ServiceDispatcher)

#endif
