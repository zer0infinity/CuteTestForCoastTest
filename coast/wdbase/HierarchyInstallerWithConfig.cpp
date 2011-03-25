/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "HierarchyInstallerWithConfig.h"
#include "Dbg.h"

void HierarchyInstallerWithConfig::DoInitializeLeaf(const char *leafName, HierarchConfNamed *& leaf) {
	StartTrace1(HierarchyInstallerWithConfig.DoInitializeLeaf, "current leaf [" << leafName << "]");
	ROAnything roaObjectConfig;
	SubTraceAny(TraceFullConfig, fObjectConfig, "installer config");
	if ( fObjectConfig.LookupPath(roaObjectConfig, leafName, fSlotDelim, fIdxDelim) ) {
		TraceAny(roaObjectConfig, "setting config for object [" << leafName << "]");
		leaf->SetConfig(GetCategory(), leafName, roaObjectConfig);
	}
	HierarchyInstaller::DoInitializeLeaf(leafName, leaf);
}
