/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- standard modules used ----------------------------------------------------
#include "Renderer.h"

//--- standard modules used ----------------------------------------------------
#include "Registry.h"
#include "SystemLog.h"
#include "Dbg.h"

//---- RenderersModule -----------------------------------------------------------
RegisterModule(RenderersModule);

RenderersModule::RenderersModule(const char *name) : WDModule(name)
{
}

RenderersModule::~RenderersModule()
{
}

bool RenderersModule::Init(const ROAnything config)
{
	if (config.IsDefined("Renderers")) {
		AliasInstaller ai("Renderer");
		return RegisterableObject::Install(config["Renderers"], "Renderer", &ai);
	}
	return false;
}

bool RenderersModule::ResetFinis(const ROAnything config)
{
	AliasTerminator at("Renderer");
	return RegisterableObject::ResetTerminate("Renderer", &at);
}

bool RenderersModule::Finis()
{
	return StdFinis("Renderer", "Renderers");
}

//---- Renderer ---------------------------------------------------------
RegisterRenderer(Renderer);
RegCacheImpl(Renderer);	// FindRenderer()

Renderer::Renderer(const char *name) : NotCloned(name)
{
}

Renderer::~Renderer()
{
}

void Renderer::RenderAll(ostream &reply, Context &, const ROAnything &)
{
	reply << ("Renderer::RenderAll: abstract method!");
}

void Renderer::Render(ostream &reply, Context &c, const ROAnything &info)
{
	StartTrace(Renderer.Render);
	SubTraceAny(In, info, "info");
	//----------------------------------------
	// if info contains a simple type we render
	// it directly onto the output reply
	AnyImplType aImplType = info.GetType();
	Renderer *r = 0; // this is our marker, makes logic simpler (PS)
	if ( aImplType == AnyCharPtrType || aImplType == AnyLongType || aImplType == AnyDoubleType ) {
		long len;
		const char *buf = info.AsCharPtr("", len);
		Trace( "Basic renderable leaf found--> " << NotNull(buf) );
		reply.write(buf, len);
	}
	//----------------------------------------
	// if info is an array we assume it is a
	// renderer specification
	else if ( aImplType == AnyArrayType) {
		Trace( "Array entry found" );
		// Check for old style configuration
		if (info.IsDefined("Type")) {
			//----------------------------------------
			// info has a structure like { /Type ... /Data { ... } }
			// or { /Type ... /AnyKey ... /... }
			// the type defines the renderer to be used
			Trace( "old Type style renderer" );

			const char *type = info["Type"].AsCharPtr(0);
			if (type && (r = FindRenderer(type))) { // whe have to check otherwise FindRenderer cores
				// type contains a valid string and it was a valid renderer type
				if (info.IsDefined("Data")) {
					// { /Data { .... } }
					// contains the specification for this type
					r->RenderAll(reply, c, info["Data"]);
				} else {
					// { /Type ... /AnyKey ... /... }
					// we send the whole info
					// let the renderer do the rest
					r->RenderAll(reply, c, info);
				}
			} else {
				String logMsg;
				{
					OStringStream logStream(logMsg);
					logStream << "Renderer::Render: No renderer found with name [" << type << "], config [";
					info.PrintOn(logStream, false) << "] discarding config";
				}
				SystemLog::Warning(logMsg);
			} // if (r)if (type)
		} else { // new type configuration
			// treat as a collection of renderer specifications, Sequence Renderer
			for (long i = 0, size = info.GetSize(); i < size; ++i) {
				// Check if slot has a name and if this is a renderer
				// otherwise interpret the slot content
				String slotname(info.SlotName(i));
				if ( slotname.Length() > 0 && (r = FindRenderer(slotname)) ) {
					// renderer found - let it work
					TraceAny(info[i], "found [" << slotname << "], calling with config");
					r->RenderAll(reply, c, info[i]);
				} // renderer found
				else {
					Trace((slotname.Length()?slotname:"<unnamed slot>") << " not found as renderer");
					// Not a renderername or no slotname
					// - ignore it and try to interpret the slots content
					// PS: we should consider to diversify again
					// by not rendering non-anonymous slots...
					if (slotname.Length() > 0) {
						String logMsg;
						{
							OStringStream logStream(logMsg);
							logStream << "Renderer::Render: No renderer found with name [" << slotname << "], config [";
							info.PrintOn(logStream, false) << "] calling Render with config of " << slotname << " again";
						}
						SystemLog::Warning(logMsg);
					}
					Render(reply, c, info[i]);
				}
			}
		}
	}
}

void Renderer::RenderOnString(String &s, Context &c, const ROAnything &info)
{
	OStringStream os(s);
	Render(os, c, info);
	os.flush();
}

String Renderer::RenderToString(Context &c, const ROAnything &info)
{
	String result;
	RenderOnString(result, c, info);
	return result;
}

void Renderer::RenderOnStringWithDefault(String &s, Context &c, const ROAnything &info, Anything def)
{
	ROAnything inf = info;
	if (inf.IsNull()) {
		inf = def;
	}
	RenderOnString(s, c, inf);
}

String Renderer::RenderToStringWithDefault(Context &c, const ROAnything &info, Anything def)
{
	String result;
	RenderOnStringWithDefault(result, c, info, def);
	return result;
}

void Renderer::PrintOptions(ostream &reply, const char *tag, const ROAnything &any)
{
	reply << '<' << tag;
	if (any.IsDefined("Options")) {
		PrintOptions2(reply, any);
	}
	reply << '>';
}

void Renderer::PrintOptions2(ostream &reply, const ROAnything &any)
{
	if (any.IsDefined("Options")) {
		ROAnything layout = any["Options"];
		if (layout.GetType() == AnyArrayType) {
			for (long i = 0, sz = layout.GetSize(); i < sz; ++i) {
				reply << ' ' << layout[i].AsCharPtr("");
			}
		} else {
			reply << ' ' << layout.AsCharPtr("");
		}
	}
}

void Renderer::PrintOptions3(ostream &reply, Context &c, const ROAnything &config)
{
	ROAnything opts;
	if ( config.LookupPath(opts, "Options") ) {
		Renderer *op = FindRenderer("OptionsPrinter");
		if (op) {
			op->RenderAll(reply, c, opts);
		}
	}
}
