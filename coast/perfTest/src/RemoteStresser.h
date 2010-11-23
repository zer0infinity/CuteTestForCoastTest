/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _RemoteStresser_H
#define _RemoteStresser_H

//---- Stresser include -------------------------------------------------
#include "Stresser.h"

//---- forward declaration -----------------------------------------------

//---- RemoteStresser ----------------------------------------------------------
//! connects to a StressServer and lets it stress
//! The RemoteStresser's configuration look like this
//! <PRE>
//!	{
//!		/DataAccess		AStressServer						# The DataAccess used to connect to the stress server
//!		/Input 		"{ /StresserName TStresserRunner }"		# The input for the stress server
//!	}</PRE>
//! The configuration is search in the file StresserMeta.any
class RemoteStresser : public Stresser
{
public:
	//--- constructors
	RemoteStresser(const char *StresserName);
	~RemoteStresser();

	//--- public api
	//! does the work (connect, send input, collect result)
	//! \return an Anything containing collected data
	virtual Anything Run(long id);

//-- Cloning interface
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) RemoteStresser(fName);
	}

};

#endif
