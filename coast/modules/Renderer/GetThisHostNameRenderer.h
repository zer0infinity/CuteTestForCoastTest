/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _GetThisHostNameRenderer_H
#define _GetThisHostNameRenderer_H

#include "Renderer.h"

//! Get DNS name of the host this program is running
/*! \par Configuration
\code
{
	/Representation		String	optional, [*Full*|HostOnly|DomainOnly]
}
\endcode */
class GetThisHostNameRenderer: public Renderer {
public:
	//--- constructors
	GetThisHostNameRenderer(const char *name) :
		Renderer(name) {
	}
protected:
	virtual void RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
