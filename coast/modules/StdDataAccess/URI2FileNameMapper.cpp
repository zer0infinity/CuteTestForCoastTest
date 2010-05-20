/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "URI2FileNameMapper.h"

//--- project modules used -----------------------------------------------------
#include "System.h"

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"

//--- c-modules used -----------------------------------------------------------

//---- URI2FileNameMapper ------------------------------------------------------------------
RegisterParameterMapper(URI2FileNameMapper);

bool URI2FileNameMapper::DoFinalGetAny(const char *key, Anything &val, Context &ctx)
{
	StartTrace(URI2FileNameMapper.DoFinalGetAny);
	String strKey(key);
	if (strKey == "FileName") {
		String value;
		String uri;
		Get("REQUEST_URI", uri, ctx);
		TrimUriArguments(uri, ctx);
		Trace("RequestURI: <" << uri << ">");

		String documentRoot;
		Get("DocumentRoot", documentRoot, ctx);
		Trace("DocumentRoot: <" << documentRoot << ">");
		String path = documentRoot;
		if (!System::IsAbsolutePath(documentRoot)) {
			// document root is not set or relative
			// set path to WD_ROOT
			path = System::GetRootDir();
			if (documentRoot.Length() > 0) {
				// document root is given and seems to be a relative path
				// check if we have to insert a '/' between path and documentRoot
				if ( path[long(path.Length() - 1)] != '/' && documentRoot[0L] != '/' ) {
					// no trailing or leading slash detected append one
					path << '/';
				} else if ( path[long(path.Length() - 1)] == '/' && documentRoot[0L] == '/' ) {
					path.Trim(path.Length() - 1);
				}
#if defined(WIN32)
				// very special case, because the following path: [d:config] is relative on WIN32
				// but changes to a specific drive we have to add some special logic here
				// at this point we know that documentRoot is not absolute
				char drive;
				if ( System::GetDriveLetter(documentRoot, drive) ) {
					// now that we have a drive letter we can use documentRoot as new root
					path = "";
				}
#endif
				path << documentRoot;
			}
		} else {
			// document root is absolute path
			path = documentRoot;
		}

		// trim / at end of document root since browser always
		// sends / at beginning
		if ( path[long(path.Length() - 1)] == '/'  && uri[0L] == '/') {
			path.Trim(path.Length() - 1);
		}

		// create absolute pathname
		value = path;
		value << uri;
		val = value;
		Trace("value :" << value);
		Trace("path  :" << path);
		if ( System::IsRegularFile(value) ) {
			// do nothing yet
		} else if ( System::IsDirectory(value) ) {
			if ( value[long(value.Length() - 1)] != '/' || uri.Length() == 0 ) {
				Anything tmpStore(ctx.GetTmpStore());
				if (uri.Length() == 0) {
					// set bad request error
					tmpStore["HTTPError"] = 400L;
					tmpStore["HTTPResponse"] = "Bad Request";
					return false;
				} else {
					// set relocate error
					tmpStore["HTTPError"] = 301L;
					tmpStore["HTTPResponse"] = "Permanently moved";

					value << '/';
					val = value;
					tmpStore["HTTPLocation"] = value;
					Trace("relocated name is: [" << value << "]");
					return false;
				}
			} else {
				// append default file
				value << ctx.Lookup("DirectoryIndex", "index.html");
				val = value;
			}
		} else {
			// no regular file and no directory set the error status
			// hook for special CGI treatment
			bool result = ResolveInvalidFile(path, uri, value, ctx);
			val = value;
			return result;
		}
	} else {
		Trace( "Key[" << key << "]<" << val.AsCharPtr() << ">" );
		return ParameterMapper::DoFinalGetAny(strKey, val, ctx);
	}
	Trace("resolved name is: [" << val.AsCharPtr() << "]");
	return true;
}

bool URI2FileNameMapper::ResolveInvalidFile(String &path, String &uri, String &value, Context &ctx)
{
	StartTrace(URI2FileNameMapper.ResolveInvalidFile);

	Anything tmpStore(ctx.GetTmpStore());
	//!@FIXME might be more intelligent with resolving unknown files

	// no regular file and no directory set the error status
	tmpStore["HTTPError"] = 404;
	tmpStore["HTTPResponse"] = "Not Found";

	Trace("resolved invalid name is: [" << value << "]");
	return false;
}

void URI2FileNameMapper::TrimUriArguments(String &filename, Context &ctx)
{
	StartTrace(URI2FileNameMapper.TrimUriArguments);

	long tail = filename.StrChr('?');
	if (tail >= 0) {
		filename.Trim(tail);
	}
	Trace("filename = <" << filename << ">");
}
