/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "StreamTransferMapper.h"

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"
#include "RequestProcessor.h"
#include "Server.h"
#include "Dbg.h"

//---- StreamTransferMapper ------------------------------------------------------------------
RegisterResultMapper(StreamTransferMapper);

namespace {
	void PutResponseLineAndHeader(std::ostream &os, Context &ctx) {
		StartTrace(StreamTransferMapper.PutResponseLineAndHeader);
		Anything tmpStore(ctx.GetTmpStore());
		Anything mapinfo;
		if (tmpStore.IsDefined("Mapper")) {
			mapinfo = tmpStore["Mapper"];
		}

		if (mapinfo.IsDefined("HTTPStatus")) {
			// only create output if the body really is defined, otherwise the
			// data access already took care, if this is ok has to be determined
			Context::PushPopEntry<Anything> aEntry(ctx, "TmpHTTPStatus", mapinfo);
			RequestProcessor::RenderProtocolStatus(os, ctx);
			mapinfo.Remove("HTTPStatus");
		} else {
			Trace("no HTTPStatus");
			os << "HTTP/1.1 200 Ok" << ENDL;
		}
		if (mapinfo.IsDefined("HTTPHeader")) {
			Renderer::Render(os, ctx, mapinfo["HTTPHeader"]);
			mapinfo.Remove("HTTPHeader");
		} else {
			Trace("no HTTPHeader");
		}
		os << ENDL; // mark the end of the header
	}
}

bool StreamTransferMapper::DoPutStream(const char *key, std::istream &is, Context &ctx, ROAnything config) {
	StartTrace(StreamTransferMapper.DoPutStream);
	String cmp("HTTPBody");
	if (cmp == key) {
		std::iostream *os = ctx.GetStream();
		if (os) {
			PutResponseLineAndHeader(*os, ctx);
			(*os) << is.rdbuf();
			return true;
		}
		Trace("cannot pass stream");
		return false;
	}
	return ResultMapper::DoPutStream(key, is, ctx, config);
}
