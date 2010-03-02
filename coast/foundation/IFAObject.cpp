/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "IFAObject.h"

//--- standard modules used ----------------------------------------------------
#include "SystemLog.h"

//---- NamedObject ----------------------------------------------------------
NamedObject::NamedObject(const char *name)
	: IFAObject()
{
}

bool NamedObject::GetName(String &str) const
{
	str = GetName();
	return false;
}
