/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _StringTokenizer2Test_H
#define _StringTokenizer2Test_H

#ifndef _StringTokenizer2_h_
#define _StringTokenizer2_h_

#include "TestCase.h"

//---- forward declaration -----------------------------------------------
#include "config_foundation.h"	// for definition of EXPORTDECL_FOUNDATION
#include "ITOString.h"
//---- StringTokenizer -----------------------------------------------------------
//!testcases for StringTokenizer2
class StringTokenizer2Test : public TestCase
{
protected:
	String fShort;
	String fLong;

public:
	StringTokenizer2Test (TString name); // : TestCase (name) {}

	virtual void			setUp ();
	static Test				*worksuite ();
	static Test				*suite ();

	void			constrMethodsAll ();
	void constrMethods0 ();
	void constrMethods1 ();
	void constrMethods2 ();
	void constrMethods3 ();
	void constrMethods4 ();
	void constrMethods5 ();
	void constrMethods6 ();
	void constrMethods7 ();
	void constrMethods8 ();
	void constrMethods9 ();
	void constrMethods10 ();
	void constrMethods11 ();
	void constrMethods12 ();
	void constrMethods13 ();
	void constrMethods14 ();
	void constrMethods15 ();
	void constrMethods16 ();
	void constrMethods17 ();
	void constrMethods18 ();
	void constrMethods19 ();
	void constrMethods20 ();
	void constrMethods21 ();

	void getRemainder();
};

#endif
#endif		//not defined _StringTokenizer2Test_H
