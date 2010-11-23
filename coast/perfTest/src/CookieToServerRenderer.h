/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CookieToServerRenderer_H
#define _CookieToServerRenderer_H

#include "Renderer.h"

//---- CookieToServerRenderer -----------------------------------------------------------
/* CookieToServerRenderer:         Renders a given HTML-Tag with the end Tag
 *
 * Configuration :
 * Slotname       Description                       Status          Remarks
 * /Tag           String defining the HTML-tag      mandatory       the end tag is constructed by
 *                without brackets(eg "PRE")                        prefixing the Tag with /
 * /Options       Key-Value pair list of            optional        refer to OptionsPrinter
 *                options rendered within                           description
 *                the start tag.
 * /Content       Rendererspecification used to     optional
 *                render the content between
 *                start and end tag.
 * /NoEndTag      If defined, suppresses the        optional
 *                renderering of the end tag
 */
class CookieToServerRenderer : public Renderer
{
public:
	CookieToServerRenderer(const char *name);
	~CookieToServerRenderer();

	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

private:
	void OutputCookies(const String &explicitDomainName, std::ostream &reply, Context &c );
};

#endif		//ifndef _CookieToServerRenderer_H
