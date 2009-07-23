/*
 * Copyright (c) 2009, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _CONFIG_COASTORACLE_H
#define _CONFIG_COASTORACLE_H

/*! \file
 * Definitions for WIN32 specific DLL builds.
 * Define the compilation flag COASTORACLE_IMPL when you build the current library as DLL to enable export of all symbols.
 * When using the library as dependency and the flag COASTORACLE_IMPL is therefore not defined, the symbols will be imported instead.
 */

#if defined(WIN32)
#include <windows.h>
#ifdef _DLL
#ifdef COASTORACLE_IMPL
#define EXPORTDECL_COASTORACLE	__declspec(dllexport)
#else
#define EXPORTDECL_COASTORACLE	__declspec(dllimport)
#endif
#else
#define EXPORTDECL_COASTORACLE
#endif
#else
#define EXPORTDECL_COASTORACLE
#endif

#endif
