/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef VHOSTSERVICEDISPATCHER_H_
#define VHOSTSERVICEDISPATCHER_H_

#include "config_wdbase.h"
#include "ServiceDispatcher.h"

class EXPORTDECL_WDBASE VHostServiceDispatcher: public RendererDispatcher
{
public:
	VHostServiceDispatcher(const char *dispatcherName);
	virtual ~VHostServiceDispatcher() {
	}

	//!cloning interface
	IFAObject *Clone() const {
		return new VHostServiceDispatcher(fName);
	}

	//!uses a table to map uri prefixes to service name; renders entry found, if nothing matches, renders "DefaultService"
	virtual String FindServiceName(Context &ctx);

protected:
	void FindVHostInList(const String &requestVhost, const String &requestURI, const ROAnything &vhostList,
						 long &matchedVhost, long &matchedVhostPrefix);

private:
	// block the following default elements of this class
	// because they're not allowed to be used
	VHostServiceDispatcher();
	VHostServiceDispatcher(const VHostServiceDispatcher &);
	VHostServiceDispatcher &operator=(const VHostServiceDispatcher &);
};

#endif /* VHOSTSERVICEDISPATCHER_H_ */
