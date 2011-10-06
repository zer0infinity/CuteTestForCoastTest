/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "SystemLog.h"
#include <cstring>

namespace InitFinisManager {
	void IFMTrace(const char *pMsg) {
		static const char *pEnvVar = getenv("COAST_TRACE_INITFINIS");
		static bool bTrace = (pEnvVar != 0 && strcmp(pEnvVar, "1") == 0);
		if (bTrace) {
			SystemLog::WriteToStderr(pMsg);
		}
	}
}
