/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _DecimalFormatRenderer_H
#define _DecimalFormatRenderer_H

//---- Renderer include -------------------------------------------------
#include "Renderer.h"

//---- DecimalFormatRenderer ----------------------------------------------------------
//! <B>really brief class description</B>
/*!
<B>Configuration:</B><PRE>
{
	/Slot1		Rendererspec	[mandatory|optional], ...
	...
}</PRE>
further explanation of the purpose of the class
this may contain <B>HTML-Tags</B>
*/
class DecimalFormatRenderer : public Renderer
{
public:
	//--- constructors
	/*! \param name defines the name of the renderer */
	DecimalFormatRenderer(const char *name);
	~DecimalFormatRenderer();

	/*! Renders ?? on <I>reply </I>
		\param reply the stream where the rendered output is written on.
		\param ctx the context the renderer runs within.
		\param config the configuration of the renderer.
	*/
	virtual void RenderAll(ostream &reply, Context &ctx, const ROAnything &config);
	bool ReadConfig( Context &ctx, const ROAnything &config, String &sString, String &sScale, String &sDecSeparator);
	bool FormatNumber( String &sString, String &sScale, String &sDecSeparator );
	void InsertFiller( String &sScale, String &strDecPlaces );

};

#endif
