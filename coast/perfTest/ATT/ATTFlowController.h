/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ATTFlowController_H
#define _ATTFlowController_H

//---- HttpFlowController include -------------------------------------------------
#include "config_att.h"
#include "HttpFlowController.h"

//---- forward declaration -----------------------------------------------

//---- ATTFlowController ----------------------------------------------------------
//!FlowController for the acceptance test tool
//! handles two data accesses to get the sample and the expected
class ATTFlowController : public HttpFlowController
{
public:
	//--- constructors
	ATTFlowController(const char *FlowControllerName);
	virtual ~ATTFlowController();

//-- Cloning interface
	IFAObject *Clone() const {
		return new ATTFlowController(fName);
	}

	//--- public api
	//!gets the sample and the expected HTML documents using two dataacesses
	//! the name of the data access that gets the sample is expected in ctx.GetQuery()["DataAccess"]
	//! the name of the data access that gets the expected is expected in ctx.Lookup("ControlDataAccess")
	//! \param ctx the context to use for the data accesses
	//! \param execTime out - time used by the data access to get the sample
	//! \return true if the data accesses were ok, false otherwise
	virtual bool ExecDataAccess(Context &ctx, long &execTime);

	virtual bool AnalyseReply(Context &ctx) {
		return HttpFlowController::AnalyseReply(ctx);
	};
	virtual bool AnalyseReply(Context &ctx, Anything &result);

//	virtual bool PrepareRequest(Context &ctx);
	virtual bool PrepareRequest(Context &ctx, bool &bPrepareRequestSucceeded);

protected:
	//--- subclass api
	//!gets the expected HTML documents
	//! the name of the data access defaults to GetExpected but can be pass in with ctx.Lookup("ControlDataAccess")
	//! \param ctx the context to use for the data accesses
	//! \return true if the data accesses was ok, false otherwise
	virtual bool GetExpected(Context &ctx);

	//!compares the sample and the expected HTYML Trees
	//! the HTML Trees are expected to be in the context's tempstore
	//! the sample in Mapper.HTTPBody ,
	//! the expected in GetExpected.HTTPBody
	//! \param ctx the context to use for the data accesses
	//! \param result out - comparison report gets appended to this any's CompareResults slot.
	//! \return true if there was no difference, false otherwise
	virtual bool DoCompareHTMLDocs(Context &ctx, Anything &result);
	//--- member variables declaration
	bool fError;
private:
	ATTFlowController();
	ATTFlowController(const ATTFlowController &);
	ATTFlowController &operator=(const ATTFlowController &);

};

#endif
