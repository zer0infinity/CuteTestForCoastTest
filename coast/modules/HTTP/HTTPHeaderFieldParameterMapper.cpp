/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */
#include "HTTPHeaderFieldParameterMapper.h"
#include "Timers.h"
#include "HTTPConstants.h"
#include "StringStream.h"

namespace Coast {
	namespace HTTP {
		bool HTTPHeaderFieldParameterMapper::DoGetStream(const char *key, std::ostream &os, Context &ctx, ROAnything info) {
			StartTrace1(HTTPHeaderFieldParameterMapper.DoGetStream, "Key:<" << NotNull(key) << ">");
			Anything fieldValues;
			//! break potential loop here when our own DoGetAny is calling us
			ParameterMapper::DoGetAny(key, fieldValues, ctx, SelectScript(key, fConfig, ctx));
			String potentiallyCombinedKey = key;
			TraceAny(fieldValues, "field values available for full key [" << potentiallyCombinedKey << "]");
			String headerFieldName = potentiallyCombinedKey.SubString(potentiallyCombinedKey.StrRChr(getDelim()) + 1).ToUpper();
			Trace("field name only [" << headerFieldName << "]");
			if (!fieldValues.IsNull()) {
				Coast::HTTP::putHeaderFieldToStream(os, ctx, headerFieldName, fieldValues);
			}
			return true;
		}

		bool HTTPHeaderFieldParameterMapper::DoGetAny(const char *key, Anything &value, Context &ctx, ROAnything info) {
			StartTrace1(HTTPHeaderFieldParameterMapper.DoGetAny, "Key:<" << NotNull(key) << ">");
			String strBuf(128L);
			OStringStream osStr(strBuf);
			if (DoGetStream(key, osStr, ctx, info)) {
				osStr.flush();
				value = strBuf;
				return true;
			}
			return false;
		}
		RegisterParameterMapper(HTTPHeaderFieldParameterMapper);
	}
}
