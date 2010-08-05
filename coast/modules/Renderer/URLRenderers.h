/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLRenderers_H
#define _URLRenderers_H

#include "config_renderer.h"
#include "Renderer.h"

// ---- introduction ----------------------------------------------------
/*! \page URLRenderers URLRenderer behavior
This file provides several renderers used for the creation of URLs.
URLs are the main link of generated pages back to the current
Coast session. Unfortunately some browsers (MSIE) cannot
handle URLs of arbitrary length. In addition, pages with many
URLs are common, therefore flexible mechanisms are needed
to shorten the generated URLs and thus the generated pages:

As well as the usual full URLs the following renderers therefore
support the HTML base URL feature, i.e. a full URL is defined
once and subsequent URLs on the same page then only contain the
difference to the base url.

Another issue is, that the syntax of URLs are slightly
dependend on the place where they are used within a MIME output
For all these reasons (and some more) the URLPrinter hierarchy
was introduced. Specific subclasses are defined to
generate exactly the amount of state within the URLs that is
required in a given HTML context (see explanations below).
*/

//---- URLPrinter -------------------------------------------------------
//! Base utility class to abstract from concrete URL Format
/*!
\par Configuration
\code
{
	/BaseAddr	Rendererspec	optional, protokoll and server part of the URL
	/Action 	Rendererspec	optional, action token
	/Params {
		/nameofparam1 valueOfFirstParam		Rendererspec	optional, name and value of param 1
		/nameofparam2 valueOfSecondParam	Rendererspec	optional, name and value of param 2
		...
	}
}
\endcode

It contains hook-methods to be overwritten by derived classes.
*/
class EXPORTDECL_RENDERER URLPrinter : public Renderer
{
public:
	URLPrinter(const char *name);
	URLPrinter(const char *name, char cmdSep, char argSep, char entrySep);
	void RenderAll(ostream &reply, Context &c, const ROAnything &data);

protected:
	//! template method called by RenderAll
	/*! calls GetState wich collects the link state from the context then RenderPublicState and then RenderPrivateState
	*/
	virtual void RenderState(ostream &reply, Context &c, const ROAnything &config);

	//! render public and private information seperately
	void RenderScriptName(ostream &reply, Context &context);
	//! renders "beginning" of URL depending on some general configuration setting ( UseBaseURL, BaseAddress, ServicePrefix )
	/*! The specific configuration slot BaseAddr and content of the environment passed by the HTTP server also generates adr= and port= if given in the "linkstate"
	*/
	virtual void RenderPublicState(ostream &reply, Context &c, const ROAnything &config, Anything &state);
	void RenderPublicPartOfURL(ostream &reply, Context &c, const ROAnything &config, Anything &state);
	//! renders the "private" and usually encoded part of the state put into the URL
	/*! Different link generation contexts require different names of the parameter (X, X1 and X2) these parameter names are known by the Coast gateway program, that decodes them. the default implemenation is empty.
	*/
	virtual void RenderPrivateState(ostream &reply, Context &c, const ROAnything &config, Anything &state);

	//! build up additional private state per link
	/*! It uses the following data structure to derive what parts of the context should be put into the state, and what kind of action should be triggered by the generated URL. The /Params tag allows either explicitely or anonymously to generate paramter string values by arbitrary renderer specifications anonymous parameters are named /P# within the state where # is the position within the /Params list. The /Params mechanism allows to add in detail the amount of state passed via a specific URL and specify this locally at the place where the URL is defined.
	*/
	virtual void BuildPrivateState(ostream &reply, Context &c, const ROAnything &config,  Anything &state);

	//! encodes state and appends under argName
	void AppendEncodedState(ostream &reply, Context &c, const Anything &state, const char *argName);

	//! seperators for different kind of URLs
	char ArgSep();
	//! seperators for different kind of URLs
	char CmdSep();
	//! seperators for different kind of URLs
	char EntrySep();

	void GetState(Anything &state, Context &c);
private:
	char fArgSep;
	char fCmdSep;
	char fEntrySep;
};

//---- FullURLPrinter -------------------------------------------------------
//! Generates complete absolute URLs using a single form variable
/*!
The FullURLPrinter is used to generate complete absolute URLs with just a
single Form Variable named "X". Its command separator is '?' a la CGI,
its argument separator '&' and the parameter separator ' '
following the CGI conventions.
*/
class EXPORTDECL_RENDERER FullURLPrinter : public URLPrinter
{
public:
	FullURLPrinter(const char *name);

protected:
	void RenderPrivateState(ostream &reply, Context &c, const ROAnything &config, Anything &state);
};

//---- BaseURLPrinter -------------------------------------------------------
//! Generate the HREF part of a HTML-BASE tag
/*!
The BaseURLPrinter is used to generate the HREF in the \<BASE\> tag of
the HTML header this significantly reduces page sizes, when many
links to the current Coast session are generated on a page.
Because the base tag does only allow '/' as a separator every element
is separated by a '/' the private state is denoted by "X1="
*/
class EXPORTDECL_RENDERER BaseURLPrinter : public URLPrinter
{
public:
	BaseURLPrinter(const char *name);

protected:
	void RenderPrivateState(ostream &reply, Context &c, const ROAnything &config, Anything &state);
	void RenderPublicState(ostream &reply, Context &c, const ROAnything &config, Anything &state);
};

//---- BaseURLRenderer -------------------------------------------------------
//! Generate the HTML-BASE tag using BaseURLPrinter to render the HREF
/*!
The BaseURLRenderer is used to generate \<BASE\> tag in the HTML header
It uses the BaseURLPrinter to generate the necessary URL
*/
class EXPORTDECL_RENDERER BaseURLRenderer : public BaseURLPrinter
{
public:
	BaseURLRenderer(const char *name);
	void RenderAll(ostream &reply, Context &c, const ROAnything &data);
};

//---- SimpleURLPrinter -------------------------------------------------------
//! Used for pages where Base-URL is used - only renders private state
/*!
The SimpleURLPrinter is used on pages generated with the \<BASE\> tag using
the BaseURLPrinter. It just emits the private state of the URL
that is relative to the BASE URL. Again parameter separation is
only possible via '/' and the encoded state is named "X2="
*/
class EXPORTDECL_RENDERER SimpleURLPrinter : public URLPrinter
{
public:
	SimpleURLPrinter(const char *name);

protected:
	void RenderState(ostream &reply, Context &c, const ROAnything &config);
	void RenderPrivateState(ostream &reply, Context &c, const ROAnything &config, Anything &state);
};

//---- URLRenderer -------------------------------------------------------
//! Renders an full or simple URL depending on the page or server setting
/*!
\par Configuration
\code
{
	/Action 	Rendererspec	optional, action token
	/Params {
		/nameofparam1 valueOfFirstParam		Rendererspec	optional, name and value of param 1
		/nameofparam2 valueOfSecondParam	Rendererspec	optional, name and value of param 2
		...
	}
}
\endcode

The URLRenderer renders just the URL (e.g. for placing it
in an imagemap etc. Depending on the over-all configuration it
uses either a FullURLPrinter (without BASE tag) or the
SimpleURLPrinter when a \<BASE\> tag is used for a page.
the structure of the possible date is explained at
the URLPrinter base class:
*/
class EXPORTDECL_RENDERER URLRenderer : public Renderer
{
public:
	URLRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &data);

	//! create base-URL safe intra page link to 'id'
	//! \note a temporary slot named 'ABSOLUTE_URL' is created in tmpStore
	static String CreateIntraPageLink(Context &c, String id);
};

//---- LinkRenderer -------------------------------------------------------
//! Renders a link
/*!
\par Configuration
\code
{
	/Action 	Rendererspec	optional, action token
	/Label		Rendererspec	optional, displayed name of the link
	/Params {
		/nameofparam1 valueOfFirstParam		Rendererspec	optional, name and value of param 1
		/nameofparam2 valueOfSecondParam	Rendererspec	optional, name and value of param 2
		...
	}
	/Options {	Anything		optional, options to the A tag
		/Key	Value			optional, key - value pairs
		SimpleValue				optional, simple non-value tag
		...
	}
}
\endcode

The LinkRenderer forms a HTTP HREF statement with a Label and
an URL (see URLRenderer, URLPrinter,
SimpleURLPrinter, FullURLPrinter) The Indirection to the URLRenderer
specified in the context is necessary, because different
application environments have different requirements on the
format of the URLs (with BASE tag, encryption, etc.)
Therefore the complete config of LinkRenderer is passed to
the called URLRenderer. See URLPrinter for additional
data elements besides /Label. The additional data is used to
generate the "private" part of the link.
*/
class EXPORTDECL_RENDERER LinkRenderer : public Renderer
{
public:
	LinkRenderer(const char *name);

	void RenderAll(ostream &reply, Context &c, const ROAnything &data);
};

//--- inlines -------------------------
inline char URLPrinter::ArgSep()
{
	return fArgSep;
}
inline char URLPrinter::CmdSep()
{
	return fCmdSep;
}
inline char URLPrinter::EntrySep()
{
	return fEntrySep;
}

#endif		//not defined _URLRenderers_H
