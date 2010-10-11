/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

#ifndef _Socket_H
#define _Socket_H

#if defined(__370__)
#define SOCKET_ERROR sock_errno()
#else
#if !defined(WIN32)
#define SOCKET_ERROR errno
#endif
#endif

#include "config_foundation.h"
#include "Anything.h"

//--- forward declarations ------------------------------------------------------------
class EXPORTDECL_FOUNDATION AcceptorCallBack;

struct 	 sockaddr_in;
int EXPORTDECL_FOUNDATION closeSocket(int sd);

//--- Socket --------------------------------------------------------------------------
//!represents a read/write socket end point that is connected
//! it is a <B>communication end point</B> for read/write sockets.
//! it has an api that allows the manipulation of the socket and
//! the generation of an iostream which is a SocketStream.
class EXPORTDECL_FOUNDATION Socket
{
public:
	//! constructor sets the variables
	//! \param socket the socket filedescriptor for the socket endpoint
	//! \param clientInfo contains information about the other connected endpoint like ip adress and port
	//! \param doClose flag that defines the behaviour of the destructor with regard to closing the socket <BR>doClose == <B>true</B> closes the socketdescriptor in the destructor <BR>doClose == <B>false</B> leaves the socket open
	//! \param timeout timeout for socket communication over the stream (default is 5 minutes)
	//! \param a Allocator to use for allocation
	//! \pre  socket is a valid connected socket file descriptor
	//! \post  a usable socket object which allocates the stream on demand
	Socket(int socket, const Anything &clientInfo = Anything(), bool doClose = true, long timeout = 300L * 1000L /* 5 minutes */, Allocator *a = Storage::Global());

	//! destructor
	//! \pre  socket contains a valid connected socket file descriptor and potentially a stream object
	//! \post  the stream is deleted and the socketfd is closed if fDoClose is true
	virtual ~Socket();

	//! returns an iostream the iostream is allocated once and then cached
	//! \pre socket contains a valid connected socket file descriptor and potentially a stream object
	//! \post the stream is deleted and the socketfd is closed if fDoClose is true
	virtual std::iostream *GetStream();

	//! accessor to the sockets fd
	virtual long GetFd() const {
		return fSockFd;
	}

	//! blocks the calling thread until the socket is ready to accept data or the timeout expires
	//! if returning false, you can use bool HadTimeout() to determine if it was because of a timeout or an error
	virtual bool IsReadyForWriting();

	//! IsReadyForReading blocks the calling thread until the socket is ready to provide data or the timeout expires
	//! if returning false, you can use bool HadTimeout() to determine if it was because of a timeout or an error
	virtual bool IsReadyForReading();
	bool HadTimeout() {
		return fHadTimeout;
	}

	//! \deprecated API:
	//! blocks the calling thread until the socket is ready to accept data or the timeout expires
	//! \param  timeout - defines the maximum waiting time in milliseconds if tiemout is 0 it returns immediately, if -1 it blocks infinitely
	//! \param  retCode - returns the poll return code in case of false it might be a timeout (retCode == 0) or an error on socket level (retCode == -1)
	virtual bool IsReadyForWriting(long timeout, long &retCode);

	//! IsReadyForReading blocks the calling thread until the socket is ready to provide data or the timeout expires
	//! \param  timeout - defines the maximum waiting time in milliseconds if tiemout is 0 it returns immediately, if -1 it blocks infinitely
	//! \param  retCode - returns the poll return code in case of false it might be a timeout (retCode == 0) or an error on socket level (retCode == -1)
	virtual bool IsReadyForReading(long timeout, long &retCode);

	//! returns amount of bytes read from belonging iostream:
	virtual long GetReadCount() const;
	//! returns amount of bytes wrote from belonging iostream:
	virtual long GetWriteCount() const;

	//! shuts the socket down for reading and writing. After this operation it is no longer usable
	virtual bool ShutDown();

	//! shuts the reading side down. The socket can still be used for writing
	virtual bool ShutDownReading();

	//! shuts the writing side down. The socket can still be used for reading
	virtual bool ShutDownWriting();

	//! returns information about the other side of the socket like ip adress and port
	virtual const Anything &ClientInfo() {
		return fClientInfo;
	}

	//! Appends information stored in parameter to fClientInfo
	//! iterates over anythings slots for appending
	//! uses ROAnything to enforce DeepClone
	virtual void AppendToClientInfo(const char *slot, const ROAnything &);

	//! sets the fDoClose flag
	virtual void CloseOnDelete(bool doClose = true)	{
		fDoClose = doClose;
	}

	//! set the default timeout for socket communication in milliseconds
	virtual void SetTimeout(long timeout) 	{
		fTimeout = timeout;
	}

	//! get the default timeout for socket communication over streams
	virtual long GetTimeout()				{
		return fTimeout;
	}

	//! access allocator
	virtual Allocator *GetAllocator()		{
		return fAllocator;
	}

	//! allocate object with optimized allocator strategy
	static void *operator new(size_t size, Allocator *a);
	//! delete object with optimized allocator strategy
	static void operator delete(void *d);
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
	static void operator delete(void *d, Allocator *a);
#endif

#if defined(WIN32)
	//! mandatory initialization/cleanup calls for WIN32
	static void InitWSock();
	static void CleanupWSock();
#endif

	/*! Wrapper function to read from a socket filedescriptor. Win32 handles socket filedescriptors different than normal file descriptors
		\param fd socket filedescriptor to read from
		\param buf character buffer which receives the bytes read
		\param len length of the supplied character buffer buf
		\param flags special flags for WIN32
		\return number of bytes received, 0 if the connection has been gracefully closed and negative in case of an error */
	static int read(int fd, char *buf, int len, int flags = 0);

	/*! Wrapper function to write to a socket filedescriptor. Win32 handles socket filedescriptors different than normal file descriptors
		\param fd socket filedescriptor to write to
		\param buf character buffer which contains the data to be sent
		\param len number of characters to send from buf
		\param flags special flags for WIN32
		\return number of bytes sent (can be lower than the given amount if non-blocking), negative in case of an error */
	static int write(int fd, const char *buf, int len, int flags = 0);

	/*! get specified option for the given socket filedescriptor
		\note only int-types supported by this function
		\param socketFd file descriptor of socket
		\param optionName name of option to query
		\param lValue value of the specified option
		\return true if no error occured */
	static bool GetSockOptInt(int socketFd, int optionName, int &lValue);

	/*! set network buffering on socket level
		\param socketfd Filedescriptor of the socket to modify
		\param lBufSize buffer size to set, ace specifies 65535 as maximum
		\param bWriteSide set to true if the write buffer should be modified, false for the read side
		\return true if the modification was successful, check back with GetSockOptInt() to get the real buffer size */
	static bool SetSockoptBufSize(int socketfd, long lBufSize, bool bWriteSide);

	/*! set the socket option TCP_NODELAY that disables the Nagle algorithm
		\param socketfd file descriptor of socket to modify
		\param bNoDelay set to true if you want to disable the Nagle algorithm, false to enable it
		\return true in case of success */
	static bool SetNoDelay(int socketfd, bool bNoDelay = true);

	/*! member wrapper around static function
		\return true in case of success */
	bool SetNoDelay() {
		return SetNoDelay(GetFd(), true);
	}

	/*! hook for setting file descriptors to non-blocking IO, used for sockets and pipes
		\param fd the file descriptor to be set to non-blocking with fcntl
		\param dontblock allow also for reset of non-blocking
		\return true on success of operation */
	static bool SetToNonBlocking(int fd, bool dontblock = true);

	/*! set SO_REUSEADDR socket option to disable 4 minutes wait
		\param socketfd file descriptor of socket to modify
		\param bReuse set to true if you want to reuse socket address, false otherwise
		\return true in case of success */
	static bool SetSockoptReuseAddr(int socketfd, bool bReuse = true);

	//!Providing default implementation of this method to avoid runtime errors
	// if people are calling me based upon a Socket *
	virtual bool IsCertCheckPassed(ROAnything config) {
		return false;
	}

protected:
	//! DoMakeStream allocates a socket iostream, overwrite these method if you need something specific
	virtual std::iostream *DoMakeStream();

	//! IsReady bottleneck routine that calls System::DoSingleSelect
	virtual bool IsReady(bool forreading);

//@{
	//! \deprecated
	//! IsReady bottleneck routine that calls the poll system call
	virtual bool IsReady(long fd, short event, long timeout, long &retCode);
//@}

	//!bottleneck routine that calls shutdown system call
	//!	\param fd the file descriptor to be set to non-blocking with fcntl
	//! \param which defines which side of the socket is shutdown see shutdown man page
	virtual bool ShutDown(long fd, long which);

	/*! print out the last error message caused by a socket error, it will get printed on SystemLog and Trace
		\param socketfd filedescriptor of socket on which the error occured
		\param contextmessage some text explaining why it was
		\param lRetCode return code of the function which failed previously */
	static void LogError(int socketfd, const char *contextmessage, long lRetCode);

protected:
	//! the socket file descriptor used for this socket
	long fSockFd;

	//! the iostream used to read and write to the socket ( default is SocketStream )
	std::iostream *fStream;

	//! flag that determines whether the socket will be closed in the destructor
	bool fDoClose;

	//! contains information about the other side of the socket
	Anything fClientInfo;

	//!default timeout for socket communication over streams
	long fTimeout;

	//!marker showing timeout case, if not ready this indicates no-error
	bool	fHadTimeout;
	//!optimized allocation strategy
	Allocator *fAllocator;

private:
	//!  this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	Socket();

	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	Socket(const Socket &s);

	//!  this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	Socket &operator=(const Socket &s);

	friend class ConnectorTest;
};

//! common features of Connector and Acceptor
class EXPORTDECL_FOUNDATION EndPoint
{
public:
	//!constructor with ip adress, port
	//! \param ipAdr defines the ip adress of the other endpoint in dot notation like 127.0.0.1
	//! \param port defines the port number of the connection: a 16-bit integer
	EndPoint(const String &ipAdr, long port)
		: fIPAddress(ipAdr, Storage::Global()), fPort(port), fSockFd(-1), fThreadLocal(false) {}

	//! deletes the internal socket object if allocated
	virtual ~EndPoint() {}

	//!return the port number of an EndPoint
	//! useful when constructing with port 0 as the OS will assign an arbitrary portnumber
	//! in bind, it is available after a PrepareAcceptLoop
	long GetPort() {
		return fPort;
	}
	//!return the ip address of this endpoint
	virtual String GetAddress() {
		return fIPAddress;
	}
	//!obtain the file descriptor of this endpoint's socket
	int GetFd() {
		return fSockFd;
	}
	//!wrap socket() system call
	//! \return true if socket succeeded, false if failure, store file descriptor in fSockFd
	bool CreateSocket();
	//!unify error logging
	void LogError(const char *contextmessage, int sockerrno=0);
	//!obtain port bound to socket after connect or accept
	long GetBoundPort();
	//!bind endpoint to a ip address and or port
	bool BindToAddress(String &srcIpAdr, long srcPort);
	//!return true if getsockopt indicates an error
	bool SockOptGetError();
	//!obtain default timeout value of 5 seconds, for connectors the connecttimeout is default
	virtual long GetDefaultSocketTimeout() {
		return 300 * 1000L;
	}

	//!set to true to allocate socket in thread local memory pool, if available
	void SetThreadLocal(bool tl = true) {
		fThreadLocal = tl;
	}

	//! wrap inet_addr api
	//! \param ipAddr the ip address in dot notation, i.e. "127.0.0.1"
	//! \param anyipaddr set to true for client source address in connectors
	//! \return the numerical addr in network order, INADRR_NONE if an error occurs
	static unsigned long MakeInetAddr(String ipAddr, bool anyipaddr = false);

	//! prepare the C structs for inet addrs used in socket API
	//! usually called with Socket::MakeInetAddr as second param
	static bool PrepareSockAddrInet(struct sockaddr_in &sockaddr, unsigned long ipAddr, long port);

	//! define localhost representation once
	static const char *GetLocalHost() {
		return "127.0.0.1";
	}

protected:
	//!factory method for Socket creation, it creates a Socket object as default override this if you need sth. else
	//! overwrite this method if you want to provide a subclass of Socket as product
	//! \param socketfd the socket file descriptor
	//! \param clientInfo the information about other endpoint ip adress and port
	//! \param doClose flag that defines whether the generated Socket object closes the socket on delete
	virtual Socket *DoMakeSocket(int socketfd, Anything &clientInfo, bool doClose = true);
	Allocator *GetSocketAllocator();

	//!holds the ip adress in dot notation e.g. 127.0.0.1
	String fIPAddress;
	//!holds the port
	long fPort;
	//!the endpoint socket file descriptor
	int fSockFd;
	//!flag that indicates use of optimized allocation policies for DoMakeSocket
	bool fThreadLocal;

	friend class ConnectorTest;
};

//--- ConnectorArgs-------------------------------------------------------------------------
//! DataObject to hold the plenty arguments needed for Connector derived classes.
//! This class  unlike ConnectorParameters does not "hold" or "take" a Context, which is crucial
//! for independent project structures.
//! This class supports assignment and copy construction.
class EXPORTDECL_FOUNDATION ConnectorArgs
{
public:
	ConnectorArgs(const String &ipAddr, long port, long connectTimeout = 0L);

	ConnectorArgs();
	~ConnectorArgs();
	ConnectorArgs &operator=(const ConnectorArgs &ca);

	// The peer's IPAddress
	String IPAddress();
	// The peer's port
	long Port();
	String PortAsString();
	// The connect timeout in milliseconds (used for connects and for reads/writes!)
	long ConnectTimeout();

private:
	String fIPAddress;
	long fPort;
	long fConnectTimeout;
};

//--- Connector ---------------------------------------------------------------------------
//!defines the active side socket factory that establishes a socket connection
//! It holds an active end point specification for a socket
//! It connects to the server on the other side creating a socket connection that's read/writeable
class EXPORTDECL_FOUNDATION Connector: public EndPoint
{
public:
	//! Constructor with ip adress, port and socketoptions, holds specification to create socket object on demand
	/*! \param ipAdr defines the ip adress of the other endpoint in dot notation like 127.0.0.1
		\param port defines the port number of the connection: a 16-bit integer
		\param connectTimeout the time we wait until we must have a connection or we abort the try ( in milliseconds)
		\param srcIpAdr the ip adress we bind this end of the socket to
		\param srcPort the port we bind this end of the socket to
		\param threadLocal Allocate underlying Socket object in memory pool of calling Thread if set to true
		\pre ipAdr is a valid, reachable server ip adress, port is a valid port number
		\post Connector holds the socket specification, the socket is not yet connected */
	Connector(String ipAdr, long port, long connectTimeout = 0, String srcIpAdr = "", long srcPort = 0, bool threadLocal = false);
	//!same as above, but take a ConnectorArgs object which wraps ipAdr, port, connectTimeout
	Connector(ConnectorArgs &connectorArgs, String srcIpAdr = "", long srcPort = 0,  bool threadLocal = 0);

	//!  Connector destructors deletes the internal socket object if allocated
	virtual ~Connector();

	//!creates an internal Socket, Connector deletes it in the destructor;
	virtual Socket *Use();

	//!creates an internal Socket and returns its stream, Connector deletes it in the destructor;
	virtual std::iostream *GetStream();

	//!queries the set timeout. Note that this timeout is inherited by the socket instance
	//! the connector uses, 0 is blocking > 0 non-blocking
	long GetDefaultSocketTimeout() {
		return fConnectTimeout > 0 ? fConnectTimeout : EndPoint::GetDefaultSocketTimeout();
	}
	long GetTimeout() {
		return fConnectTimeout;
	}

	//!returns the information collected in the socket used by Connector
	Anything ClientInfo();

protected:
	//! creates a Socket object everytime, the caller is responsible for destruction
	//! \param  doClose - specifies for the socket to be created whether the socket will be closed on destruction
	virtual Socket *MakeSocket(bool doClose = true);

	//! establishes a connection to server on ip adress and port
	//! \return  returns true if the connection could be established, might need to closeSocket(fSocketId) if false
	//! \post  if ( retVal == true ) the socket is connected
	virtual bool 	DoConnect();
	//!checkk error status after connect() failed (errno or WSAGetLastError() resp.)
	bool ConnectWouldBlock();

	//!holds the timeout in seconds we are willing to wait for a connection if 0 we use the system timeout mechanism (about 75 seconds or more )
	long fConnectTimeout;

	//!holds the source ip adress in dot notation e.g. 127.0.0.1 (optional)
	String fSrcIPAdress;

	//!holds the source port (optional)
	long fSrcPort;

	//!internal socket object generated by the method Use or GetStream
	Socket *fSocket;

	//!for testcases only
	friend class SocketStreamTest;
	friend class ConnectorTest;
	friend class SocketTest;
	friend class ConnectorAcceptorTest;

private:
	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	Connector();

	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	Connector(const Connector &);

	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	Connector &operator=(const Connector &);
};

//--- Acceptor ---------------------------------------------------------------------------
//!represents a listener socket end point that listens on a server port for incoming connections
//! it implements the socket protocol for listener sockets and runs the accept loop
//! it is usually started in its own thread
class EXPORTDECL_FOUNDATION Acceptor : public EndPoint
{
public:
	//!constructor with listener port, backlog and callback object
	//! \param ipadress defines the logical address or ip-address of the connection
	//! \param port defines the port number of the connection: a 16-bit integer
	//! \param backlog defines the backlog paramater for tcp in the accept call
	//! \param cb call back object that is called out of the accept loop, cb is owned by the acceptor
	//! \pre port is a valid port number > 0, backlog > 0, cb != NULL
	//! \post Acceptor holds the socket specification, the acceptor is not yet listening
	Acceptor(const char *ipadress, long port, long backlog, AcceptorCallBack *cb);

	//!destructor stops the accept loop and destroys the callback object
	//! \pre fPort > 0 is a valid port number, fCallBack != NULL
	//! \post accept loop is stopped, fCallBack == NULL, fSocket is closed if it was opened
	virtual ~Acceptor();

	//!calls the socket api in the correct sequence to setup a listener socket
	//! \return returns 0 if ok, -1 if failed, the socket gets closed if any call after socket fails
	//! \pre port is a valid port number > 0, backlog > 0, cb != NULL
	//! \post holds the socket specification, the acceptor is listening but not yet accepting
	virtual int PrepareAcceptLoop();

	//!starts the accept loop which only terminates after setting fAlive to false
	//! \return returns 0 if termination ok, -1 if final accept failes
	//! \pre port is a valid port number > 0, backlog > 0, cb != NULL
	//! \post holds the socket specification, the acceptor is not yet accepting
	//! \param once forces the acceptor to stop acceptloop after first accept
	virtual int RunAcceptLoop(bool once = false);

	//!calls accept
	//! \return returns Socket *object if ok, 0 if failed
	virtual Socket *DoAccept();

	//!stops the accept loop by setting fAlive to false
	//! \param useConnect connect to localhost if Acceptor is alive (necessary to bring a multithreaded Acceptor out of the loop).
	//! \return returns true
	//!call has no effect if accept loop has already stopped or was never running
	virtual bool StopAcceptLoop(bool useConnect = true);

	//!return the ip address the acceptor is listening to
	String GetAddress();

	//!return Backlog size
	long GetBacklog();
protected:

	//!calls the creator of the acceptor back via fCallBack object
	//! \param socket communication end point created in this accept call
	//! \pre socket != NULL, fCallBack != NULL
	virtual void DoCallBack(Socket *socket);

	//!parameter for tcp that defines how many connection requests get queued in the tcp layer
	long fBackLog;

	//!callback object that calls the user of the acceptor back when an connection has succeeded
	AcceptorCallBack *fCallBack;

	//!alive flag, keeps the accept loop running
	bool fAlive;

	//!stopped flag, holds state that the accept loop is stopped
	bool fStopped;

private:
	//! this standard operator is private to block its use
	//! it is private to prohibit its use<br>
	//!if it is not declared, the C++ compiler would generate it automatically.<br>
	Acceptor();

	//! this standard operator is private to block its use
	//! it is private to prohibit its use<br>
	//!if it is not declared, the C++ compiler would generate it automatically.<br>
	Acceptor(const Acceptor &);

	//! this standard operator is private to block its use
	//! it is private to prohibit its use<br>
	//!if it is not declared, the C++ compiler would generate it automatically.<br>
	Acceptor &operator=(const Acceptor &);
};

//--- CallBackFactory ---------------------------------------------------------------------------
//!factory object that creates AcceptorCallbacks on demand
class EXPORTDECL_FOUNDATION CallBackFactory
{
public:

	//!default constructor is empty since class provides only protocol no implementation
	CallBackFactory()	{ }

	//!default destructor is empty since class provides only protocol no implementation
	virtual ~CallBackFactory() { }

	//!call back functionality provides the socket object
	virtual AcceptorCallBack *MakeCallBack() = 0;

private:
	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	CallBackFactory(const CallBackFactory &);

	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	CallBackFactory &operator=(const CallBackFactory &);
};

//--- AcceptorCallBack ---------------------------------------------------------------------------
//!callback object that calls the client of the accept loop back when a connection has suceeded
//!this class is abstract and must be subclassed to provide request handling functionality
//!CallBack provides the Socket object resulting from the accepted connection
//!it has also a locking protocol for MT Safe usage
class EXPORTDECL_FOUNDATION AcceptorCallBack
{
public:

	//!default constructor is empty since class provides only protocol no implementation
	AcceptorCallBack()	{ }

	//!default destructor is empty since class provides only protocol no implementation
	virtual ~AcceptorCallBack() { }

	//!call back functionality provides the socket object
	//! \param socket the Socket object resulting from the accepted connection
	virtual void CallBack(Socket *socket) = 0;

	//!locking callback objects may override this method to provide true locking
	virtual void Lock() { }

	//!locking callback objects may override this method to provide true unlocking
	virtual void Unlock() { }

	//!condition var callback objects may override this method to provide true condition vars
	virtual void Wait() { }

	//!condition var callback objects may override this method to provide true signalling
	virtual void Signal() { }

private:
	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	AcceptorCallBack(const AcceptorCallBack &);

	//! this standard operator is private to block its use
	//! if it is not declared, the C++ compiler generates it automatically
	//! it is not implemented don't use it
	AcceptorCallBack &operator=(const AcceptorCallBack &);
};

#endif
