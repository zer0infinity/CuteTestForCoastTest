/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AnyToXMLRenderer_H
#define _AnyToXMLRenderer_H

#include "Renderer.h"

//---- AnyToXMLRenderer -----------------------------------------------------------
//! Renders XML and a DTD out of an Anything
/*!
\par Configuration
\code
{
	/Input	Rendererspec	mandatory, produces a lookup string used to lookup the Anything that serves as Input
}
\endcode
*/
class AnyToXMLRenderer : public Renderer
{
public:
	//! basic constructor
	//! \param name	Name of class to register
	AnyToXMLRenderer(const char *name);
	//! standard dtor
	~AnyToXMLRenderer();

	//! The well known Renderer main method
	virtual void RenderAll(std::ostream &reply, Context &c, const ROAnything &config);

private:
	//!transforms an Anything into XML
	//! \param reply where the XML gets written to
	//! \param input Anything that gets converted to XML. Must have at least one slot and this must have a slotname, which is equal to the XML's toplevel element. Further slots are neglected.
	void RenderXML(std::ostream &reply, ROAnything &input);

	//!transforms a list of named items
	//! \param reply where the XML gets written to
	//! \param list Anything that forms the list of named elements
	void RenderNamedChilds(std::ostream &reply, ROAnything &list);

	//!transfroms unnamed items of a list using the parent supplied tag
	//! \param reply where the XML gets written to
	//! \param slotname XML tag applied to each item
	//! \param list Anything that forms the list of unnamed elements
	void RenderUnnamedChilds(std::ostream &reply, String &tag, ROAnything &list);

	//!Checks if a ROAnything consists of a list of unnamed childs
	//! \param element the ROAnything that gets checked
	//! \return True if element is of type eArray and the first slot has no slotname
	bool HasUnnamedChilds(ROAnything element);
};

#endif		//ifndef _AnyToXMLRenderer_H
