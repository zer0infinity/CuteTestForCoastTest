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
#include "Dbg.h"
#include "ITOStorage.h"
#include "ITOString.h"

//---- FinalCleaner ----------------------------------------------------------
static FinalCleaner fgCleanerManager;

FinalCleaner::FinalCleaner()
	: fNext(0)
{
	if ( this != &fgCleanerManager ) {
		Add(this);
	}
}

FinalCleaner::~FinalCleaner()
{
	if ( this == &fgCleanerManager ) {
		Storage::SetHooks(0);
		TerminateTracer();
		while (fNext) {
			FinalCleaner *ch = fNext;
			fNext = fNext->fNext;
			delete ch;
		}
	}
}

void FinalCleaner::Add(FinalCleaner *obj)
{
	FinalCleaner *t = fgCleanerManager.fNext;
	fgCleanerManager.fNext = obj;
	obj->fNext = t;
}

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
