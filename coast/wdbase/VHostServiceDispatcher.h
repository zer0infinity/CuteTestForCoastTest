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

	ServiceHandler *FindServiceHandler(Context &ctx);

private:
	ServiceHandler *findServiceBasedOnLongestURIPrefixMatch(Context &ctx, String& requestURI);

	// block the following default elements of this class
	// because they're not allowed to be used
	VHostServiceDispatcher();
	VHostServiceDispatcher(const VHostServiceDispatcher &);
	VHostServiceDispatcher &operator=(const VHostServiceDispatcher &);
};

#endif /* VHOSTSERVICEDISPATCHER_H_ */
