/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_ACTIONS_H
#define _CONFIG_ACTIONS_H

//! DLL specific settings for Windows NT
#if defined(WIN32)
#ifdef _DLL
#include <windows.h>
#ifdef ACTIONS_IMPL
#define EXPORTDECL_ACTIONS	__declspec(dllexport)
#else
#define EXPORTDECL_ACTIONS	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_ACTIONS
#endif
#else
#define EXPORTDECL_ACTIONS
#endif

#endif
