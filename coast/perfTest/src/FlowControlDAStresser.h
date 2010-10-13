/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FlowControlDAStresser_h_
#define _FlowControlDAStresser_h_

#include "Stresser.h"

//---- FlowControlDAStresser -----------------------------------------------------------
/*! stresser that runs a serie of DataAccesses.
 * The Input for each request is provided by an FlowController component
 * \code
 * {
 *		/DataAccess			Name of the DataAccess to use
 *		/FlowController		Name of the FlowController component
 * }
 * \endcode
 */
class EXPORTDECL_PERFTEST FlowControlDAStresser : public Stresser
{
public:
	FlowControlDAStresser(const char *FlowControlDAStresserName);
	~FlowControlDAStresser();

	virtual Anything Run(long id);

//-- Cloning interface
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) FlowControlDAStresser(fName);
	}
private:
	inline void CheckCopyErrorMessage(Anything &result, Anything &tmpStore, long lStepNumber, bool bWasError) {
		if ( tmpStore["result"].IsDefined("ErrorMessage" ) ) { // error msgs
			if (bWasError) {
				Anything conv = lStepNumber;
				String indexStr = conv.AsString("");
				result["ErrorMessageCtr"][indexStr] = tmpStore["result"]["ErrorMessage"];
			}
			tmpStore["result"].Remove("ErrorMessage");
		}
	}
};

#endif
