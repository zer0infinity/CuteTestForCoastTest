/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _ConnectorDAImpl_H
#define _ConnectorDAImpl_H

//--- superclass interface ---
#include "config_stddataaccess.h"
#include "DataAccessImpl.h"

class Connector;

//---- ConnectorDAImpl ----------------------------------------------------------
//! Protocol less connector to send and receive from a remote host
/*!
Use a specific Mapper to implement a protocol. For example you can use a AnythingToStreamMapper or a StreamToAnythingMapper to use an Anything as transport packet
\par Configuration
\code
{
	/Address				String		mandatory, address of the other side in IP notation
	/Port					long		mandatory, port of the other side
	/CloseSocket			long		optional, default 1, if set to 1 we not not close the socket filedescriptor that it can be re-used in a latter call, also the params will be stored at slot "SocketParams" in the tempstore to be reused
	/ConnectTimeout			long		optional, default 10, how long should we wait until we can connect to the other party
	/UseSSL					long		optional, default 0, if set to 1 a SSLSocket will be created, otherwise we use a regular Socket
	/SocketTimeout			long		optional, default 300,socket time out after which the socket gets closed, 300s is also the system default
	/SocketStreamTimeout	long		optional, default 30, max time to wait on reading characters from stream
	/CheckReadyTimeout		long		optional, default 30, time to wait until characters are available on stream
	/TcpNoDelay				long		optional, default 1, if set to 1, the TCP-stack does not wait until it has a complete packet to send, it sends immediately
}
\endcode

*/
class EXPORTDECL_STDDATAACCESS ConnectorDAImpl: public DataAccessImpl
{
public:
	/*! \param name defines the name of the data access */
	ConnectorDAImpl(const char *name);
	~ConnectorDAImpl();

	/*! @copydoc IFAObject::Clone(Allocator *) */
	IFAObject *Clone(Allocator *a) const;

	/*! executes the transaction
		\param c The context of the transaction */
	virtual bool Exec(Context &c, ParameterMapper *, ResultMapper *);

protected:
	virtual bool DoExec(Socket *pSocket, Context &context, ParameterMapper *, ResultMapper *);
	virtual bool SendInput(iostream *Ios, Socket *s, long lCheckReadyTimeout, long lSocketStreamTimeout, Context &context, ParameterMapper *, ResultMapper *);
	virtual bool HandleError(Socket *s, const char *streamText, Context &context, ParameterMapper *, ResultMapper *);

private:
	Socket *DoConnect(Connector *csc, Context &context, ParameterMapper *, ResultMapper *);
	Connector *DoMakeConnector( Context &context, ParameterMapper *, ResultMapper *);
	Socket *RecreateSocket(Anything &anyParams, Context &context, ParameterMapper *, ResultMapper *);

	// constructor
	ConnectorDAImpl();
	ConnectorDAImpl(const ConnectorDAImpl &);
	// assignement
	ConnectorDAImpl &operator=(const ConnectorDAImpl &);

	friend class ConnectorDAImplTest;
};

#endif		//not defined _ConnectorDAImpl_H
