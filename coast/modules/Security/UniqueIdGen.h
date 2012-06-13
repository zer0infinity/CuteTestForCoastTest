/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UniqueIdGen_H
#define _UniqueIdGen_H

#include "ITOString.h"

namespace Coast {
	namespace Security {
		//! Generate a Unique ID
		/*!
		 Returns a String containing a unique id.
		 */
		String generateUniqueId(const String &additionalToken = "NoAdditionaTokenWasSet");
	}
}

#endif
