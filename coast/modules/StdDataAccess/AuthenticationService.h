/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _AuthenticationService_H
#define _AuthenticationService_H

#include "ServiceHandler.h"

class AuthenticationService: public ServiceHandler {
	AuthenticationService();
public:
	AuthenticationService(const char *authenticationServiceHandlerName) :
		ServiceHandler(authenticationServiceHandlerName) {
	}
	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const {
		return new (a) AuthenticationService(fName);
	}

protected:
	bool DoHandleService(std::ostream &os, Context &ctx);
	bool DoCheck(Context &ctx);
	void ForwardToMainHandler(std::ostream &os, Context &ctx);
	bool AuthenticationIsOk(Context &ctx);
	void GetUserNameAndPw(Context &ctx, String &name, String &pw);
	bool AuthenticateUser(Context &ctx, String &name, String &pw);
	void Produce401Response(std::ostream &os, Context &ctx);
};

#endif
