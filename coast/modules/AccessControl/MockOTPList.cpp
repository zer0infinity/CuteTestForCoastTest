/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "MockOTPList.h"

//--- project modules used -----------------------------------------------------

//--- standard modules used ----------------------------------------------------
#include "Dbg.h"
#include "Registry.h"

//--- c-modules used -----------------------------------------------------------

// ---------------------- MockOTPList ---------------------------------

RegisterOTPList(MockOTPList);

bool MockOTPList::Verify(String tokenId, String otp, long window, TokenDataAccessController *)
{
	StartTrace(MockOTPList.Verify);

	// if no config is present, then return distance 1 always
	if ( fConfig.IsNull() ) {
		Trace("No config present.");
		Trace("(Mocked) Success.");
		return true;
	}

	long dist = IsValid(tokenId, otp, window, 0);
	if (dist >= 0) {
		Trace("old count: " << (long)GetCount(tokenId));
		SetCount(tokenId, GetCount(tokenId) + dist + 1);
		Trace("new count: old + " << (dist + 1) << " = " << (long)GetCount(tokenId));
	}
	return (dist >= 0);
}

long MockOTPList::IsValid(String tokenId, String otp, long window, TokenDataAccessController *)
{
	StartTrace(MockOTPList.IsValid);

	// if no config is present, then return distance 1 always
	if ( fConfig.IsNull() ) {
		Trace("No config present.");
		Trace("(Mocked) Match. Distance is: 0");
		return 0L;
	}

	Trace("token id: " << tokenId);
	Trace("otp code: " << otp);
	Trace("window  : " << window);

	ROAnything otpList = fConfig[tokenId]["OtpList"];
	if ( !otpList.IsNull() ) {
		long count = GetCount(tokenId);
		for (long i = count; i < otpList.GetSize() && i < count + window; i++) {
			if ( otpList[i].AsString().IsEqual(otp) ) {
				Trace("Match within window. Distance is: " << (i - count));
				return (i - count);
			}
		}
	}
	Trace("No match.");
	return -1L;
}

long MockOTPList::GetCount(String tokenId)
{
	StartTrace(MockOTPList.GetCount);
	// lazy copy
	if ( !fCount.IsDefined(tokenId) ) {
		fCount[tokenId] = fConfig[tokenId]["Count"].AsLong(0);
	}
	return fCount[tokenId].AsLong();
}

void MockOTPList::SetCount(String tokenId, long newCount)
{
	StartTrace(MockOTPList.SetCount);
	fCount[tokenId] = newCount;
}
