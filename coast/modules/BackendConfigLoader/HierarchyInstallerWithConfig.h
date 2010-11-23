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

class HierarchyInstallerWithConfig : public HierarchyInstaller {
	ROAnything fObjectConfig;
public:
	HierarchyInstallerWithConfig(const char *cat, const ROAnything roaObjectConfig, char delimSlot = '.', char delimIdx = ':');
	virtual ~HierarchyInstallerWithConfig() {}
protected:
    virtual void DoInitializeLeaf(const char *leafName, HierarchConfNamed *& leaf);
private:
    char fSlotDelim, fIdxDelim;
};

#endif /* HIERARCHYINSTALLERWITHCONFIG_H_ */
