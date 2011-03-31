/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "AccessController.h"
#include "Dbg.h"
#include "Registry.h"

// ------ UserDataAccessController -----------------------------------

RegCacheImpl(UserDataAccessController);		// creates FindUserDataAccessController()

// ------ TokenDataAccessController -----------------------------------

RegCacheImpl(TokenDataAccessController);	// creates FindTokenDataAccessController()

// ------ EntityDataAccessController -----------------------------------

RegCacheImpl(EntityDataAccessController);	// creates FindEntityDataAccessController()
