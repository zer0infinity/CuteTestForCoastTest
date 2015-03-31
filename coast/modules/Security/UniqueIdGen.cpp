/*
 * Copyright (c) 2007, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#include "UniqueIdGen.h"
#include "MD5.h"
#include "SystemBase.h"
#include "DiffTimer.h"

namespace {
	long const uniqueIdHashLength=32L;
	DiffTimer fUniqueIdTimer;
	long fHostid=::gethostid();
	long fPid=coast::system::getpid();
	long getHostId() {
		return fHostid;
	}
	long getPid() {
		return fPid;
	}
	DiffTimer::tTimeType getDiffTime() {
		return fUniqueIdTimer.RawDiff();
	}
    String hexHash(String const& s) {
		StartTrace(UniqueIdGen.HexHash);
		String hash(uniqueIdHashLength);
		MD5Signer::DoHash(s, hash);
		String buffer(uniqueIdHashLength);
		buffer.AppendAsHex(reinterpret_cast<const unsigned char*>(hash.cstr()), hash.Length());
		return buffer;
	}
}

namespace coast {
	namespace security {
		String generateUniqueId(const String &additionalToken) {
			StartTrace(UniqueIdGen.generateUniqueId);
			String buffer(128L);
			buffer.Append(getDiffTime())
					.Append('_').Append(getHostId())
					.Append('_').Append(getPid())
					.Append('_').Append(coast::security::nextRandomNumber());
			if (additionalToken.Length()) {
				buffer.Append('_').Append(additionalToken);
			}
			Trace("id before hashing [" << buffer << "]");
			String ret(uniqueIdHashLength);
			ret = hexHash(buffer);
			Trace("Resulting UniqueId: [" << ret << "]");
			return ret;
		}
	}
}
