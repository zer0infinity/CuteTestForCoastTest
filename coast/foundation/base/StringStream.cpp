/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "StringStream.h"
#include "Tracer.h"

namespace NSStringStream {
	bool PlainCopyStream2Stream(std::istream *streamSrc, std::ostream &streamDest, long &copiedBytes, long lBytes2Copy) {
		StartTrace(StringStream.PlainCopyStream2Stream);
		std::streamsize szReadSize = 0;
		Trace("Bytes to copy: " << lBytes2Copy);
		String sBuf(lBytes2Copy);
		char *buf = (char *) (const char *) sBuf;
		copiedBytes = 0L;
		bool bRet = true;
		while (lBytes2Copy > 0L) {
			if (streamSrc->good()) {
				if (streamDest.good()) {
					streamSrc->read(buf, (int) lBytes2Copy);
					szReadSize = streamSrc->gcount();
					streamDest.write(buf, szReadSize);
					copiedBytes += static_cast<long>(szReadSize);
					lBytes2Copy -= static_cast<long>(szReadSize);
					Trace("Bytes copied so far: " << copiedBytes);
				} else {
					Trace("Destination stream is not good , aborting copy!");
					bRet = false;
					break;
				}
			} else {
				// test if we reached eof of streamSrc
				bRet = (streamSrc->eof() != 0);
				Trace("Source stream is not good anymore" << (bRet ? " (eof)" : "") << ", finishing copy!" );
				break;
			}
		} Trace("bytes copied this time: " << copiedBytes);
		return bRet;
	}
}
