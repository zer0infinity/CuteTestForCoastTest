/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ThreadedStresserRunner_H
#define _ThreadedStresserRunner_H

//---- Stresser include -------------------------------------------------
#include "Stresser.h"

//---- ThreadedStresserRunner ----------------------------------------------------------
//! runs a series of stressers each in a separate thread
//! The number and types of the Stressers to be run is
//! given in the ThreadedStresserRunner's configuration
//! <PRE>
//!	{
//!		/StresserName1	Nr1
//!		/StresserName2	Nr2
//!		....
//!	}</PRE>
//! This configuration defines that Nr1 threads are run with StresserName1,
//! Nr2 threads with StresserName2 and so on.
class EXPORTDECL_PERFTEST ThreadedStresserRunner : public Stresser
{
public:
	//--- constructors
	ThreadedStresserRunner(const char *StresserName);
	~ThreadedStresserRunner();

	//--- public api
	//! runs the configured stressers
	//! \return Anything containing the collected results from all stressers
	//! the collected results are of the form
	//! <PRE>
	//!	{
	//!		/Results {
	//!			{				# Results of the first Stresser
	//!				/Nr		x	# number of calls
	//!				/Sum	x	# sum of transaction time
	//!				/Max	x	# maximum transaction time
	//!				/Min	x	# minimum transaction time
	//!				/Error	x	# number of erroneous transactions
	//!			}
	//!			{				# Results of the second Stresser
	//!				....
	//!			}	....
	//!		}
	//!		/Total {
	//!				/Time	x	# the overall time in ms until all stressers have completed
	//!				/Nr		x	# total number of calls
	//!				/Sum	x	# the total sum of transaction time
	//!				/Max	x	# maximum transaction time of all stressers
	//!				/Min	x	# minimum transaction time of all stressers
	//!				/Error	x	# total number of erroneous transactions
	//!		}
	//!	}</PRE>
	virtual Anything Run(long id);
//-- Cloning interface
	IFAObject *Clone() const {
		return new ThreadedStresserRunner(fName);
	}

protected:

	virtual long ConfigStressers(long numOfStressers, long &sz, ROAnything &stresser);

};

#endif
