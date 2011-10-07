/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _FlowController_h_
#define _FlowController_h_

#include "WDModule.h"
#include "Context.h"

class FlowControllersModule: public WDModule {
public:
	FlowControllersModule(const char *name) :
		WDModule(name) {
	}
	virtual bool Init(const ROAnything config);
	virtual bool Finis();
};

//---- FlowController -----------------------------------------------------------
/*! base class for flow controlling components that
 * prepare the context for a DataAccess according to configuration and/or
 * previous requests results.
 * The configuration looks like this:
 * \code
 *	{
 *		/PreRun	{				# Inputconfiguration for preparing
 *			{ InputConfig }		# requests (only run once)
 *			{ InputConfig }
 *		}
 *		/NumberOfRuns	x		# Number of times the series is run
 *		/Run	{				# Inputconfiguration for the series of
 *			{ InputConfig }		# requests (run NumberOfRuns times)
 *			{ InputConfig }
 *		}
 *	}
 * \endcode
 */
class FlowController: public ConfNamedObject {
	FlowController();
	FlowController(const FlowController &);
	FlowController &operator=(const FlowController &);

	//! Utility method that copies the slots from runConfig to dest
	void DoPrepare(Anything &dest, const ROAnything &runConfig);
public:
	FlowController(const char *FlowControllerName);
	//!Input data for the DataAccess are placed into the context.
	//! \param ctx Context of the stress run
	//! \return true if a DataAccess is to be executed - false if the series is finished
	virtual bool PrepareRequest(Context & ctx);
	//!Input data for the DataAccess are placed into the context.
	//! \param ctx Context of the stress run
	//! \param bPrepareRequestSucceeded status of the request preparation
	//! \return true if a DataAccess is to be executed - false if the series is finished
	virtual bool PrepareRequest(Context & ctx, bool & bPrepareRequestSucceeded);
	virtual bool ExecDataAccess(Context & ctx, long & execTime);
	virtual bool AnalyseReply(Context & ctx);
	virtual bool AnalyseReply(Context & ctx, Anything & result);
	virtual void CleanupAfterStep(Context & ctx);
	virtual void Init(Context & ctx);
	RegCacheDef(FlowController);
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) FlowController(fName);
	}
protected:
	virtual long GetRequestNr(Context &ctx);
	virtual ROAnything GetStepConfig(Context &ctx);
	virtual void DoCleanupAfterStep(Context &ctx, ROAnything roaStepConfig);
};

#define RegisterFlowController(name) RegisterObject(name, FlowController)

#endif
