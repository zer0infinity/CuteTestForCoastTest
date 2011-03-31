/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "MapperTestDAImpl.h"
#include "StringStream.h"
#include "Dbg.h"

RegisterDataAccessImpl(MapperTestDAImpl);

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
		for (long i = 0, sz=config.GetSize(); i < sz; i++) {
			String strGetKey, strPutKey;
			Anything anyEntry = config[i];
			strGetKey = config.SlotName(i);
			if ( anyEntry.GetType() == AnyArrayType ) { //<! assume unnamed configuration entries { /getKey putKey }
				strGetKey = anyEntry.SlotName(0L);
				anyEntry = anyEntry[0L];
			}
			strPutKey = anyEntry.AsString();
			Trace("get key [" << strGetKey << "] put key [" << strPutKey << "]");
			if (strGetKey.Length()) {
				String inputStr(128L);
				bool bGetCode, bPutCode = false;
				if (strMode.Compare("int") == 0) {
					int iTestVal = 0;
					bGetCode = input->Get(strGetKey, iTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, iTestVal, context);
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << (long)iTestVal << "]");
					}
				} else if (strMode.Compare("long") == 0) {
					long lTestVal = 0L;
					bGetCode = input->Get(strGetKey, lTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, lTestVal, context);
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << lTestVal << "]");
					}
				} else if (strMode.Compare("bool") == 0) {
					bool bTestVal = false;
					bGetCode = input->Get(strGetKey, bTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, bTestVal, context);
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << (bTestVal ? "true" : "false") << "]");
					}
				} else if (strMode.Compare("float") == 0) {
					float fTestVal = 0.0F;
					bGetCode = input->Get(strGetKey, fTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, fTestVal, context);
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << fTestVal << "]");
					}
				} else if (strMode.Compare("double") == 0) {
					double dTestVal = 0.0;
					bGetCode = input->Get(strGetKey, dTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, dTestVal, context);
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << dTestVal << "]");
					}
				} else if (strMode.Compare("Anything") == 0) {
					Anything aTestVal;
					bGetCode = input->Get(strGetKey, aTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, aTestVal, context);
						StringStream Ios(inputStr);
						aTestVal.PrintOn(Ios);
						Ios << std::flush;
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << inputStr << "]");
					}
				} else if (strMode.Compare("StringStream") == 0) {
					StringStream Ios(inputStr);
					bGetCode = input->Get(strGetKey, Ios, context);
					Ios << std::flush;
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, Ios, context);
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << inputStr << "]");
					}
				} else {
					// default is String mode
					String sTestVal = "";
					bGetCode = input->Get(strGetKey, sTestVal, context);
					if (bGetCode) {
						bPutCode = output->Put(strPutKey, sTestVal, context);
						Trace("From [" << strGetKey << "] to [" << strPutKey << "]: [" << sTestVal << "]");
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
