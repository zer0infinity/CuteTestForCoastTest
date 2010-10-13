/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MapperTestDAImpl.h"

//--- standard modules used ----------------------------------------------------
#include "StringStream.h"
#include "Dbg.h"

#if defined(ONLY_STD_IOSTREAM)
using namespace std;
#endif

RegisterDataAccessImpl(MapperTestDAImpl);

MapperTestDAImpl::MapperTestDAImpl(const char *name) : DataAccessImpl(name)
{
}

IFAObject *MapperTestDAImpl::Clone(Allocator *a) const
{
	return new (a) MapperTestDAImpl(fName);
}

bool MapperTestDAImpl::Exec( Context &context, ParameterMapper *input, ResultMapper *output)
{
	StartTrace1(MapperTestDAImpl.Exec, fName);
	Anything config, anyDataType;
	TraceAny(fConfig, "fConfig: ");
	String strMode;
	if ( input->Get("DataType", anyDataType, context) ) {
		strMode = anyDataType.AsString();
	} else {
		strMode = "String";
	}
	Trace("DataType is [" << strMode << "]");

	if ( input->Get("transfer", config, context) ) {
		TraceAny(config, "Config: ");
		for (long i = 0; i < config.GetSize(); i++) {
			const char *slotname = config.SlotName(i);
			if (slotname) {
				String inputStr(128L);
				bool bGetCode, bPutCode = false;
				if (strMode.Compare("int") == 0) {
					int iTestVal = 0;
					bGetCode = input->Get(slotname, iTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), iTestVal, context);
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << (long)iTestVal << "]");
					}
				} else if (strMode.Compare("long") == 0) {
					long lTestVal = 0L;
					bGetCode = input->Get(slotname, lTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), lTestVal, context);
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << lTestVal << "]");
					}
				} else if (strMode.Compare("bool") == 0) {
					bool bTestVal = false;
					bGetCode = input->Get(slotname, bTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), bTestVal, context);
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << (bTestVal ? "true" : "false") << "]");
					}
				} else if (strMode.Compare("float") == 0) {
					float fTestVal = 0.0F;
					bGetCode = input->Get(slotname, fTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), fTestVal, context);
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << fTestVal << "]");
					}
				} else if (strMode.Compare("double") == 0) {
					double dTestVal = 0.0;
					bGetCode = input->Get(slotname, dTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), dTestVal, context);
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << dTestVal << "]");
					}
				} else if (strMode.Compare("Anything") == 0) {
					Anything aTestVal;
					bGetCode = input->Get(slotname, aTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), aTestVal, context);
						StringStream Ios(inputStr);
						aTestVal.PrintOn(Ios);
						Ios << flush;
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << inputStr << "]");
					}
				} else if (strMode.Compare("StringStream") == 0) {
					StringStream Ios(inputStr);
					bGetCode = input->Get(slotname, Ios, context);
					Ios << flush;
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), Ios, context);
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << inputStr << "]");
					}
				} else {
					// default is String mode
					String sTestVal = "";
					bGetCode = input->Get(slotname, sTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(config[i].AsCharPtr(), sTestVal, context);
						Trace("From [" << slotname << "] to [" << config[i].AsCharPtr() << "]: [" << sTestVal << "]");
					}
				}
				Trace("GetCode: " << (bGetCode ? "true" : "false") << " PutCode: " << (bPutCode ? "true" : "false"));
			}
		}
		Trace("ret: true");
		return true;
	}
	Trace("ret: false");
	return false;
}
