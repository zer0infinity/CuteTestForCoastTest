/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _HttpFlowController_h_
#define _HttpFlowController_h_

#include "FlowController.h"
class Context;
class Anything;
class ROAnything;

typedef String (*funcPtr)( String );

//---- HttpFlowController -----------------------------------------------------------

class EXPORTDECL_PERFTEST HttpFlowController : public FlowController
{
public:
	HttpFlowController(const char *HttpFlowControllerName);
	virtual ~HttpFlowController();

	//!Input data for the DataAccess are placed into the context.
	//! \param ctx Context of the stress run
	//! \return true if a DataAccess is to be executed - false if the series is finished
	virtual bool PrepareRequest(Context &ctx, bool &bPrepareRequestSucceeded);

	virtual bool AnalyseReply(Context &ctx, Anything &result) {
		return FlowController::AnalyseReply(ctx, result);
	};
	virtual bool AnalyseReply(Context &ctx);

	//-- Cloning interface
	IFAObject *Clone() const {
		return new HttpFlowController(fName);
	}

protected:
	virtual void DoCommand(Context &ctx, ROAnything commandConfig);

private:
	//!Finds a slot in the array of links or forms given by information in the config slot of the current FlowController step
	//! \param aTag "Link" or "Form" as String to drive this method
	//! \param aSlotName slot to be retrieved from the found html link or form array member
	//! \param aTag ctx, how to get to all information
	//! \param aTag result as Anything
	//! \return true if Form/Link etc is defined in this step config and a Forms array member found with the defined slot number and slotname in this member found, otherwise false
	//! calls e.g. ..("Link", "href", ctx, anyResult )
	//! calls e.g. ..("Link", "title", ctx, anyResult )
	//! calls e.g. ..("Form", "method", ctx, anyResult )
	//! calls e.g. ..("Form", "action", ctx, anyResult )
	//! calls e.g. ..("Frame", "src", ctx, anyResult )
	//! was static - why (?)
	bool GetFormOrLinkInfo( const String &aTag, const String &aSlotName, Context &ctx, Anything &aResult );

	// block the following default elements of this class
	// because they're not allowed to be used
	HttpFlowController();
	HttpFlowController(const HttpFlowController &);
	HttpFlowController &operator=(const HttpFlowController &);

	bool DoRelocate(Context &ctx);
	void ProcessSetCookie(Context &ctx);
	void DoProcessSetCookie(String &cookieInfo, Context &ctx);
	bool DoProcessToken( Context &ctx, bool &boJump );
	bool DoProcessLinksFormsAndFrames(Context &ctx);
	void CheckFuzzyMatch(Anything &containerToCheck, String &strToLookFor );
	void HandleTheError(String &theMsg, Anything &tmpStore );
	bool ResolveLabels(Anything &jmpTable, const ROAnything &runConfig, Anything &tmpStore );
	long FindJumpNr(String &jmpLabel);
	long ResolveJumpNameToNr(String &jumpName, Anything &tmpStore );
	void CheckDoJump(long jumpNr, Anything &tmpStore );
	void GenerateMultipartContent( String &fieldName, ROAnything fieldConfig, ROAnything boundary, String &multipartContents, Context &ctx );
	void DoCleanupAfterStep(Context &ctx, ROAnything roaStepConfig);
	// scheduling
	bool GenericScheduling( Anything &tmpStore, Anything &scheduling, String &currentTimeDate, funcPtr conversionFunction   );
	bool SingleScheduling( Context &ctx );
	bool PeriodicalScheduling( Context &ctx );
	void SetupSSLCtx(Anything &sslModuleCfg, Context &ctx);
	void PrepareSSL(Context &ctx);

	Anything fJmpTable;
	bool fJmpTableInit;
	bool fURLAlreadyCalculated;
	bool fDoRelocate;
	friend class HTTPFlowControllerPrepareTest;
};

#endif

