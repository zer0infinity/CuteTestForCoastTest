/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _TimeStampRenderer_H
#define _TimeStampRenderer_H

#include "config_wdbase.h"

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

//---- TimeStampRenderer ----------------------------------------------------------
//! Renders a TimeStamp to the stream
/*!
\par Configuration
\code
{
	/UnixTime 	Rendererspec		optional, default current time, unix time to use for timestamp
}
\endcode
or just
\code
{
	Rendererspec	optional, default current time, unix time to use for timestamp
}
\endcode

This renderer simply creates a TimeStamp object from current system time or given unix time value and puts the human readable String onto the reply stream.
The output format is fixed to the format returned from the object.
Currently it is (in strftime format): "\%Y\%m\%d\%H\%M\%S"
*/
class EXPORTDECL_WDBASE TimeStampRenderer : public Renderer
{
public:
	//--- constructors
	/*! \param name defines the name of the renderer */
	TimeStampRenderer(const char *name);
	virtual ~TimeStampRenderer();

	/*! Renders the timestamp to reply
		\param reply the stream where the rendered output is written on.
		\param ctx the context the renderer runs within.
		\param config the configuration of the renderer. */
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
};

#endif
