/*
 * Copyright (c) 2008, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_BACKENDCONFIGLOADER_H
#define _CONFIG_BACKENDCONFIGLOADER_H

/*! \file
 * Definitions for WIN32 specific DLL builds.
 * Define the compilation flag BACKENDCONFIGLOADER_IMPL when you build the current library as DLL to enable export of all symbols.
 * When using the library as dependency and the flag BACKENDCONFIGLOADER_IMPL is therefore not defined, the symbols will be imported instead.
 */

#if defined(WIN32)
#ifdef _DLL
#include <windows.h>
#ifdef BACKENDCONFIGLOADER_IMPL
#define EXPORTDECL_BACKENDCONFIGLOADER	__declspec(dllexport)
#else
#define EXPORTDECL_BACKENDCONFIGLOADER	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_BACKENDCONFIGLOADER
#endif
#else
#define EXPORTDECL_BACKENDCONFIGLOADER
#endif

#endif
