/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "SimpleDataAccessService.h"

//--- standard modules used ----------------------------------------------------
#include "Timers.h"
#include "Renderer.h"
#include "DataAccess.h"
#include "RequestProcessor.h"
#include "Dbg.h"

//---- SimpleDataAccessService -----------------------------------------------------------
RegisterServiceHandler(SimpleDataAccessService);

//---- SimpleDataAccessService ----------------------------------------------------------------
SimpleDataAccessService::SimpleDataAccessService(const char *name) : ServiceHandler(name)
{
}

SimpleDataAccessService::~SimpleDataAccessService()
{
}

void SimpleDataAccessService::DoHandleService(ostream &reply, Context &ctx)
{
	StartTrace(SimpleDataAccessService.DoHandleService);
	if (!!reply ) {
		String service;
		this->GetName(service);

		Anything tmpStore(ctx.GetTmpStore());
		Trace("Executing data access: " << service);

		if ( DataAccess(service).StdExec(ctx) ) {
			MethodTimer(SimpleDataAccessService.DoHandleService, "Processing output", ctx);
		} else {
			Trace("error something wrong with service:" << service);
			Anything mapinfo = tmpStore["Mapper"];
			if (!(mapinfo.IsDefined("HTTPStatus") && mapinfo["HTTPStatus"].IsDefined("ResponseCode"))) {
				mapinfo["ResponseCode"] = 500L;
				mapinfo["ResponseMsg"] = "internal server error";

				mapinfo["HTTPBody"] = "SimpleDataAccessService::DoHandleService some error occured";
			}
		}

		Trace("Creating output.");
		if (tmpStore.IsDefined("Mapper") && tmpStore["Mapper"].IsDefined("HTTPBody")) {
			// only create output if the body really is defined, otherwise the
			// data access already took care, if this is ok has to be determined
			Anything mapinfo = tmpStore["Mapper"];

			if (mapinfo.IsDefined("HTTPStatus")) {
				const char *TEMP_KEY = "TmpHTTPStatus";
				ctx.PushStore(TEMP_KEY, mapinfo);
				RequestProcessor::RenderProtocolStatus(reply, ctx);
				String key;
				ctx.PopStore(key);
				Assert(key == TEMP_KEY);
			} else {
				Trace("no HTTPStatus");
			}
			if (mapinfo.IsDefined("HTTPHeader")) {
				String httpHeader;
				{
					OStringStream os(&httpHeader);
					Renderer::Render(os, ctx, mapinfo["HTTPHeader"]);
				}
				Trace("HTTPHeader:<" << httpHeader << ">");
				reply << httpHeader;
			} else {
				Trace("no HTTPHeader");
			}
			reply << ENDL;
			Renderer::Render(reply, ctx, mapinfo["HTTPBody"]);
		} else {
			Trace("no HTTPBody");
		}
		reply << flush;
	}
}
