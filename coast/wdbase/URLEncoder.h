/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _URLEncoder_h_
#define _URLEncoder_h_

#include "SecurityModule.h"

//---- URLEncoder -----------------------------------------------------------
class EXPORTDECL_WDBASE URLEncoder : public Encoder
{
public:
	URLEncoder(const char *name);
	virtual ~URLEncoder();
	virtual IFAObject *Clone() const {
		return new URLEncoder(fName);
	}

	virtual void DoEncode(String &encodedText, const String &cleartext) const;
	virtual bool DoDecode(String &cleartext, const String &scrambledText) const;

protected:

private:
	URLEncoder();
	URLEncoder(const URLEncoder &);
	URLEncoder &operator=(const URLEncoder &);
};

#endif
