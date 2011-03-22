/*
 * Copyright (c) 2010, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef HIERARCHYINSTALLERWITHCONFIG_H_
#define HIERARCHYINSTALLERWITHCONFIG_H_

#include "Policy.h"

class HierarchyInstallerWithConfig: public HierarchyInstaller {
	ROAnything fObjectConfig;
	char fSlotDelim, fIdxDelim;
public:
	HierarchyInstallerWithConfig(const char *cat, const ROAnything roaObjectConfig, char delimSlot = '.', char delimIdx = ':') :
		HierarchyInstaller(cat), fObjectConfig(roaObjectConfig), fSlotDelim(delimSlot), fIdxDelim(delimIdx) {
	}
protected:
	virtual void DoInitializeLeaf(const char *leafName, HierarchConfNamed *& leaf);
};

#endif /* HIERARCHYINSTALLERWITHCONFIG_H_ */
