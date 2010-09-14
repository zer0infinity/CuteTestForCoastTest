/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _UNSAFEFSTREAM_H
#define _UNSAFEFSTREAM_H

#if defined(__SUNPRO_CC) && !defined(__STD_ISTREAM__) && ( __SUNPRO_CC < 0x500 )
#include <fstream.h>
//! a non locking fstream implementation for SunCC
//! since every stream implementation of Sun's iostream implementation
//! is supposed to be thread-safe you have to implement some unsafe_xxxstreams
//! if you don't want that feature
class unsafe_fstream : public unsafe_fstreambase, public unsafe_iostream
{
public:
	//! empty constructor
	unsafe_fstream() : unsafe_fstreambase() { }

	//! constructor parameters see man page of fstream
	unsafe_fstream(const char *nm, int mode, int prot = filebuf::openprot) : unsafe_fstreambase(nm, mode, prot) { }

	//! constructor, parameters see man page of fstream
	unsafe_fstream(int fd) : unsafe_fstreambase(fd) { }

	//! constructor, parameters see man page of fstream
	unsafe_fstream(int _f, char *m, int i) : unsafe_fstreambase(_f, m, i) { }

	//! destructor
	~unsafe_fstream()	{}

	//! accessor method to streambuf
	filebuf *rdbuf()	{
		return unsafe_fstreambase::rdbuf();
	}

	//! opens file name and connects its file descriptor to fs
	//! further details see man page fstream
	void	open(const char *nm, int m, int p = filebuf::openprot) {
		unsafe_fstreambase::open(nm, m, p);
	}
};
#endif

#endif
