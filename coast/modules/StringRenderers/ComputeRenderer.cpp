/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "ComputeRenderer.h"
#include "Tracer.h"
#include <stdlib.h>

//---- ComputeRenderer ----------------------------------------------------------------
RegisterRenderer(ComputeRenderer);

ComputeRenderer::ComputeRenderer(const char *name) : Renderer(name)
{
}

void ComputeRenderer::RenderAll(std::ostream &reply, Context &ctx, const ROAnything &config)
{
	StartTrace(ComputeRenderer.RenderAll);
	TraceAny(config, "config");

	String n1, n2, min, max, op;
	long n = -1;
	ROAnything roaSlotConfig;

	if (config.LookupPath(roaSlotConfig, "Number1")) {
		RenderOnString(n1, ctx, roaSlotConfig);
	} else {
		Trace("Error in ComputeRenderer::RenderAll, Number1 not defined");
		Render(reply, ctx, config["Error"]);
		return;
	}
	Trace("Number1: [" << n1 << "]");

	if (config.LookupPath(roaSlotConfig, "Number2")) {
		RenderOnString(n2, ctx, roaSlotConfig);
	} else {
		Trace("Error in ComputeRenderer::RenderAll, Number2 not defined");
		Render(reply, ctx, config["Error"]);
		return;
	}
	Trace("Number2: [" << n2 << "]");

	if (config.LookupPath(roaSlotConfig, "Minimum")) {
		RenderOnString(min, ctx, roaSlotConfig);
	}
	Trace("Minimum: [" << min << "]");

	if (config.LookupPath(roaSlotConfig, "Maximum")) {
		RenderOnString(max, ctx, roaSlotConfig);
	}
	Trace("Maximum: [" << max << "]");

	if (min.Length() > 0 && max.Length() > 0 && min.AsLong(0L) > max.AsLong(0L)) {
		reply << "Invalid: Minimum > Maximum";
		return;
	}

	if (config.LookupPath(roaSlotConfig, "Operator")) {
		RenderOnString(op, ctx, roaSlotConfig);
	} else {
		Trace("Error in ComputeRenderer::RenderAll, op not defined");
		Render(reply, ctx, config["Error"]);
		return;
	}
	Trace("Operator: [" << op << "]");

	if ( op.IsEqual("+") ) {
		n = n1.AsLong(0L) + n2.AsLong(0L);
	} else if ( op.IsEqual("-") ) {
		n = n1.AsLong(0L) - n2.AsLong(0L);
	} else if ( op.IsEqual("*") ) {
		n = n1.AsLong(0L) * n2.AsLong(0L);
	} else if ( op.IsEqual("/") ) {
		if (n2.AsLong(0L) == 0) {
			reply << "Fatal: Division by Zero";
			return;
		}
		n = n1.AsLong(0L) / n2.AsLong(0L);
	} else if ( op.IsEqual("%") ) {
		if (n2.AsLong(0L) == 0) {
			reply << "Fatal: Modulo by Zero";
			return;
		}
		n = n1.AsLong(0L) % n2.AsLong(0L);
	} else if ( op.IsEqual(">") ) {
		if ( n1.AsLong(0L) > n2.AsLong(0L) ) {
			n = 1;
		} else {
			n = 0;
		}
	} else if ( op.IsEqual(">=") ) {
		if ( n1.AsLong(0L) >= n2.AsLong(0L) ) {
			n = 1;
		} else {
			n = 0;
		}
	} else if ( op.IsEqual("<") ) {
		if ( n1.AsLong(0L) < n2.AsLong(0L) ) {
			n = 1;
		} else {
			n = 0;
		}
	} else if ( op.IsEqual("<=") ) {
		if ( n1.AsLong(0L) <= n2.AsLong(0L) ) {
			n = 1;
		} else {
			n = 0;
		}
	} else if ( op.IsEqual("=") ) {
		if ( n1.AsLong(0L) == n2.AsLong(0L) ) {
			n = 1;
		} else {
			n = 0;
		}
	}

	if ( min.Length() > 0 && ( n < min.AsLong(0L) ) ) {
		n = min.AsLong(0L);;
	}

	if ( max.Length() > 0 && ( n > max.AsLong(0L) ) ) {
		n = max.AsLong(0L);;
	}

	Trace( String() << "n1 = " << n1 << "   n2 = " << n2 << "   n = " << n);

	reply << (String() << n);
}
