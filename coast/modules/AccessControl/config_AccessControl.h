/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_ACCESSCONTROL_H
#define _CONFIG_ACCESSCONTROL_H

// WIN32 settings for Windows NT
#if defined(WIN32)
#include <windows.h>
#ifdef _DLL
#ifdef ACCESSCONTROL_IMPL
#define EXPORTDECL_ACCESSCONTROL	__declspec(dllexport)
#else
#define EXPORTDECL_ACCESSCONTROL	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_ACCESSCONTROL
#endif
#else
#define EXPORTDECL_ACCESSCONTROL
#endif

#endif
