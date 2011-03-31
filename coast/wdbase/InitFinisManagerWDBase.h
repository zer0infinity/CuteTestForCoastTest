/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _InitFinisManagerWDBase_H
#define _InitFinisManagerWDBase_H

#include "InitFinisManager.h"

//---- InitFinisManagerWDBase ----------------------------------------------------------
//! support class for initializing/cleaning things up in a deterministic sequence
/*!
This is the 'singleton' instance for foundation specific initialization and cleanup. If you want to add foundation-specific InitFinisManager based objects you have to derive from this class.
Normally you do not have to edit this object manually, just derive a class based on this one and add code to the DoInit() and DoFinis() method.
Initialization sequence can be controlled using the uiPriority parameter during construction of the object.
The advantage of not doing initialization/finalization in ctor/dtor lies within the ability to Init/Finis when global construction of objects has finished. This way round we can guarantee not to use global objects not already existing.

Sample code how to implement a specific Init/Finis object. The code should be placed where actually needed - inside an implementation file - to keep complexity at a minimum
<pre>
class SomeInitializer : public InitFinisManagerWDBase
{
public:
	SomeInitializer(unsigned int uiPriority)
	: InitFinisManagerWDBase(uiPriority)
	{}

	~SomeInitializer()
	{}

	virtual void DoInit()
	{
		SomeThing::Initialize();
	}

	virtual void DoFinis()
	{
		SomeThing::Finalize();
	}
};
...
static SomeInitializer *psgSomeInitializer = new SomeInitializer(0);
</pre>
*/
class InitFinisManagerWDBase : public InitFinisManager
{
public:
	/*! Base constructor, pass a priority number greater or equal 0 to control Init/Finis sequence
		\param uiPriority The library specific 'singleton' always sets its priority 0 because it is not used and does nothing in its DoInit()/DoFinis() methods. The lower the number, the higher the priority. */
	InitFinisManagerWDBase(unsigned int uiPriority);

	/*! Static method the get library specific instance pointer
		\return pointer to the 'singleton' instance */
	static InitFinisManagerWDBase *Instance();

private:
	/*! Method to do specific initialization
		In case of the 'singleton' object, we do not have to do anything */
	virtual void DoInit() {};

	/*! Method to do specific finalization
		In case of the 'singleton' object, we do not have to do anything */
	virtual void DoFinis() {};

	//! static 'singleton' instance pointer
	static InitFinisManagerWDBase *fgpInitFinisManagerWDBase;

	//! inhibit compiler supplied bitwise default copy ctor
	InitFinisManagerWDBase(const InitFinisManagerWDBase &);
	//! inhibit compiler supplied bitwise default assignment operator
	InitFinisManagerWDBase &operator=(const InitFinisManagerWDBase &);
};

#endif
