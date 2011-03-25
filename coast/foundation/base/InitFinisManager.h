/*
 * Copyright (c) 2006, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _InitFinisManager_H
#define _InitFinisManager_H

//---- InitFinisManager ----------------------------------------------------------
//! support class for initializing/cleaning things up in a deterministic sequence
/*!
Subclasses can define initialize/clean up code in DoInit() / DoFinis() methods. Objects are automatically added to the cleanup list
using the given priority to set the sequence of initialization and destruction. */
class InitFinisManager
{
public:
	/*! Base constructor, pass a priority number greater than 0 to control Init/Finis sequence
		\param uiPriority The library specific 'singleton' always sets its priority 0 because it is not used and does nothing in its DoInit()/DoFinis() methods. The lower the number, the higher the priority. */
	InitFinisManager(unsigned int uiPriority);

	//! destructor deletes all cleaner objects of the list in reverse order
	virtual ~InitFinisManager();

	//! Method used to initialize the added objects in the order given by the priority number
	void Init();

	//! Method used to finalize the added objects in the reversed order given by the priority number
	void Finis();

	/*! method which can be used to trace something based on the setting of environment variable COAST_TRACE_INITFINIS
		\param pMsg message to print, including newline character if needed */
	static void IFMTrace(const char *pMsg);

protected:
	//! Method to do specific initialization
	virtual void DoInit() = 0;

	//! Method to do specific finalization
	virtual void DoFinis() = 0;

	/*! used to build the internal priority list
		\param pManager Pointer to the library specific singleton object, internally used to distinguish singleton from 'real' objects to work on
		\param pCleaner Pointer of object to work on */
	void Add(InitFinisManager *pManager, InitFinisManager *pCleaner);

private:
	//! simple pointer 'list' of objects to manage
	InitFinisManager *fNext;

	//! priority of object
	unsigned int fPriority;

	//! inhibit compiler supplied bitwise default copy ctor
	InitFinisManager(const InitFinisManager &);
	//! inhibit compiler supplied bitwise default assignment operator
	InitFinisManager &operator=(const InitFinisManager &);
};

#endif
