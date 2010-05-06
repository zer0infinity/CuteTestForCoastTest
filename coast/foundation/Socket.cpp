/*
 * Copyright (c) 2005, Peter Sommerlad and IFS Institute for Software at HSR Rapperswil, Switzerland
 * All rights reserved.
 *
 * This library/application is free software; you can redistribute and/or modify it under the terms of
 * the license that is included with this library/application in the file license.txt.
 */

//--- interface include --------------------------------------------------------
#include "Socket.h"

//--- standard modules used ----------------------------------------------------
#include "System.h"
#include "SystemLog.h"
#include "Resolver.h"
#include "SocketStream.h"
#include "Dbg.h"

//--- c-library modules used ---------------------------------------------------
#include <cstring>

#if defined (WIN32)
#include <io.h>
#else
#include <errno.h> /* PS wg. SUNCC5 */
#include <sys/socket.h>
#if !defined(__370__)
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netinet/tcp.h>		/* TCP_NODELAY */
#include <sys/poll.h>
#endif
#include <fcntl.h>
#endif

#if defined(_AIX) || defined(__SUNPRO_CC)
#include <strings.h>
#endif

#define socklen_type socklen_t

#if !defined(INADDR_NONE)
#define INADDR_NONE ((unsigned long)-1)
#endif

//--- Socket ----------------------
int closeSocket(int sd)
{
	StartTrace1(Socket.closeSocket, "fd:" << (long)sd);
#if defined(WIN32)
	return closesocket(sd);		// clean up socket structure
#else
	return close(sd);
#endif
}

Socket::Socket(int socketfd, const Anything &clientInfo, bool doClose, long timeout, Allocator *a)
	: fSockFd(socketfd)
	, fStream(0)
	, fDoClose(doClose)
	, fClientInfo(clientInfo)
	, fTimeout(timeout)
	, fAllocator((a) ? a : Storage::Global())
{
	StartTrace1(Socket.Ctor, "fd:" << GetFd() << " using allocator: [" << (long)fAllocator << "]");
	TraceAny(fClientInfo, "clientInfo");
}

Socket::~Socket()
{
	StartTrace1(Socket.Dtor, "fd:" << GetFd());
	if ( fStream ) {
		Trace("Deleting stream");
		delete fStream;
		fStream = 0;
	}
	if ( fDoClose ) {
		Trace("Closing socket: " << fSockFd);
		long lRetCode;
		if ( (fSockFd >= 0) && ( lRetCode = closeSocket(fSockFd) ) < 0 ) {
			LogError(fSockFd, "close() socket", lRetCode);
		}
	}
}

void Socket::AppendToClientInfo(const char *slot, const ROAnything &info)
{
	StartTrace(Socket.AppendToClientInfo);
	TraceAny(info, "info to append at slot:" << slot);
	if (slot) {
		fClientInfo[slot] = info.DeepClone(fClientInfo.GetAllocator());
	} else {
		fClientInfo.Append(info.DeepClone(fClientInfo.GetAllocator()));
	}
	TraceAny(fClientInfo, "after appending");
}

iostream *Socket::GetStream()
{
	StartTrace1(Socket.GetStream, "fd:" << GetFd());

	if ( !fStream ) {
		fStream = DoMakeStream();
	}
	return fStream;
}

iostream *Socket::DoMakeStream()
{
	StartTrace1(Socket.DoMakeStream, "fd:" << GetFd());

	if ( fSockFd >= 0 ) {
		return new(fAllocator) SocketStream(this, GetTimeout());
	}
	return 0;
}

void *Socket::operator new(size_t size, Allocator *a)
{
	StartTrace(Socket.operatornew);
	if (a) {
		Trace("allocating with allocator:[" << (long)a << "]");
		return a->Calloc(1, size);
	} else {
		Trace("using global new");
		return ::operator new(size);
	}
}

#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
void Socket::operator delete(void *d, Allocator *a)
{
	StartTrace(Socket.operatordelete);
	if (d) {
		if (a) {
			Trace("deleting with allocator:[" << (long)a << "]");
			a->Free(d);
		} else {
			Trace("deleting with global delete");
			::operator delete(d);
		}
	}
}
#endif

void Socket::operator delete(void *d)
{
	StartTrace(Socket.operatordelete);
	if (d) {
		Allocator *a = ((Socket *)d)->fAllocator;
#if defined(WIN32) && (_MSC_VER >= 1200) // VC6 or greater
		Socket::operator delete(d, a);
#else
		if (a) {
			Trace("deleting with allocator:[" << (long)a << "]");
			a->Free(d);
		} else {
			Trace("deleting with global delete");
			::operator delete(d);
		}
#endif
	}
}

bool Socket::IsReady(long fd, short event, long timeout, long &retCode)
{
	StartTrace1(Socket.IsReady, "fd:" << fd);
	retCode = -1;
	fHadTimeout = false;
	if ( fd >= 0 ) {
		bool bRead = (event & POLLIN) > 0, bWrite = (event & POLLOUT) > 0;
		retCode = System::DoSingleSelect(fd, timeout, bRead, bWrite);
		Trace("testing fd:" << fd << " for " << (bRead ? "reading" : "") << (bRead && bWrite ? "&" : "") << (bWrite ? "writing" : "") << " returned:" << retCode);
		if (retCode > 0) {
			return true;
		}
		if (retCode < 0) {
			LogError(fd, "select() failed", retCode);
		} else {
			Trace("timeout occurred!");
			fHadTimeout = true;
		}
	}
	return false;
}

bool Socket::IsReadyForReading(long timeout, long &retCode)
{
	return IsReady(GetFd(), POLLIN, timeout, retCode);
}

bool Socket::IsReadyForWriting(long timeout, long &retCode)
{
	return IsReady(GetFd(), POLLOUT, timeout, retCode);
}

bool Socket::IsReadyForReading()
{
	return IsReady(true);
}

bool Socket::IsReadyForWriting()
{
	return IsReady(false);
}

bool Socket::IsReady(bool forreading)
{
	StartTrace1(Socket.IsReady, "fd:" << GetFd() << (forreading ? " for reading" : " for writing"));
	bool bRet = false;
	int res = System::DoSingleSelect(GetFd(), GetTimeout(), forreading, !forreading);
	fHadTimeout = (0 == res);
	if (res < 0) {
		LogError(GetFd(), "select() failed", res);
	}
	bRet = (res > 0);
	Trace("returning" << (bRet ? " true" : " false"));
	return bRet;
}

#if !defined(__linux__)
#define SHUT_RD 0
#define SHUT_WR 1
#define SHUT_RDWR 2
#endif

bool Socket::ShutDown(long fd, long how)
{
//	how     Specifies the type of shutdown.
//	The  values are as follows:
//
//  SHUT_RD       Disables    further    receive
//                operations.
//
//  SHUT_WR       Disables further  send  opera-
//                tions.
//
//  SHUT_RDWR     Disables  further   send   and
//                receive operations.
	if ( fd >= 0 ) {
		long lRetCode;
		if ( ( lRetCode = ::shutdown(fd, how)) >= 0 ) {
			return true;
		}
		LogError(fd, "shutdown()", lRetCode);
	}
	return false;
}

bool Socket::ShutDownReading()
{
	return ShutDown(GetFd(), SHUT_RD);
}

bool Socket::ShutDownWriting()
{
	if ( fStream ) {
		fStream->flush();
	}
	return ShutDown(GetFd(), SHUT_WR);
}

bool Socket::ShutDown()
{
	return ShutDown(GetFd(), SHUT_RDWR);
}

long Socket::GetReadCount() const
{
	StartTrace1(Socket.GetReadCount, "fd:" << GetFd());
	if (fStream) {
		SocketStreamBuf *sbuf = ((SocketStream *)fStream)->rdbuf();
		if (sbuf) {
			return sbuf->GetReadCount();
		}
	}
	return 0L;
}

long Socket::GetWriteCount() const
{
	StartTrace1(Socket.GetWriteCount, "fd:" << GetFd());
	if (fStream) {
		SocketStreamBuf *sbuf = ((SocketStream *)fStream)->rdbuf();
		if (sbuf) {
			return sbuf->GetWriteCount();
		}
	}
	return 0L;
}

#if defined(WIN32)
void Socket::InitWSock()
{
	static bool once = false;
	if (!once) {
		once = true;
		WORD wVersionRequested;
		WSADATA wsaData = { 0, 0, 0, 0, 0, 0, 0 };
		int err;

		wVersionRequested = MAKEWORD( 2, 0 );

		err = WSAStartup( wVersionRequested, &wsaData );
		if ( err != 0 ) {
			/* Tell the user that we couldn't find a useable */
			/* WinSock DLL.                                  */
			return;
		}

		/* Confirm that the WinSock DLL supports 2.0.*/
		/* Note that if the DLL supports versions greater    */
		/* than 2.0 in addition to 2.0, it will still return */
		/* 2.0 in wVersion since that is the version we      */
		/* requested.                                        */

		if ( LOBYTE( wsaData.wVersion ) != 2 ||
			 HIBYTE( wsaData.wVersion ) != 0 ) {
			/* Tell the user that we couldn't find a useable */
			/* WinSock DLL.                                  */
			WSACleanup( );
			return;
		}
	}
}

void Socket::CleanupWSock()
{
	WSACleanup();
}
#endif

int Socket::read(int fd, char *buf, int len, int flags)
{
#if defined(WIN32)
	return ::recv(fd, buf, len, flags);
#else
	return ::read(fd, buf, len);
#endif
}

int Socket::write(int fd, const char *buf, int len, int flags)
{
#if defined(WIN32)
	return ::send(fd, buf, len, flags);
#else
	return ::write(fd, buf, len);
#endif
}

void Socket::LogError(int socketfd, const char *contextmessage, long lRetCode)
{
	StartTrace1(Socket.LogError, "fd:" << socketfd);
	String logMsg(contextmessage);
	logMsg << " of socket fd=" << socketfd
		   << " failed with function retCode:" << lRetCode << " (#" << (long)System::GetSystemError() << ") " << SystemLog::LastSysError();
	Trace(logMsg);
	SystemLog::Error(logMsg);
}

bool Socket::GetSockOptInt(int socketFd, int optionName, int &lValue)
{
	StartTrace(Socket.GetSockOptInt);
	bool boRet = true;
	socklen_type len = sizeof(lValue);
	if ( getsockopt(socketFd, SOL_SOCKET, optionName, (char *)&lValue, &len) < 0 ) {
		SYSWARNING("sockopt-error [" << SystemLog::LastSysError() << "]");
		boRet = false;
	}
	Trace("returned value:" << (long)lValue);
	return boRet;
}

bool Socket::SetNoDelay(int socketfd, bool bNoDelay)
{
	StartTrace(Socket.SetNoDelay);
	if ( socketfd >= 0 ) {
		// set bNoDelay to true to disable nagle algorithm, pass 1 to setsockopt
		int val = (bNoDelay ? 1 : 0);
		long lRetCode;
		// use of (char *) instead of (const char *) to make strange platforms happy (S370)
		if ( (lRetCode = setsockopt(socketfd, IPPROTO_TCP, TCP_NODELAY, (char *) &val, sizeof(val))) == 0) {
			return true;
		}
		LogError(socketfd, "setsockopt( TCP_NODELAY )", lRetCode);
	}
	return false;
}

bool Socket::SetSockoptReuseAddr(int socketfd, bool bReuse)
{
	StartTrace(Socket.SetSockoptReuseAddr);
	if ( socketfd >= 0 ) {
		// reuse socket.. (avoid 4 minutes timeout)
		int val = (bReuse ? 1 : 0);
		long lRetCode;
		if ( (lRetCode = setsockopt(socketfd, SOL_SOCKET, SO_REUSEADDR, (char *)&val, sizeof(val))) == 0) {
			return true;
		}
		LogError(socketfd, "setsockopt( SO_REUSEADDR )", lRetCode);
	}
	return false;
}

bool Socket::SetSockoptBufSize(int socketfd, long lBufSize, bool bWriteSide)
{
	StartTrace(Socket.SetSockoptBufSize);
	if ( socketfd >= 0 ) {
		int val = lBufSize;
		long lRetCode;
		if ( (lRetCode = setsockopt(socketfd, SOL_SOCKET, (bWriteSide ? SO_SNDBUF : SO_RCVBUF), (char *)&val, sizeof(val))) == 0) {
			return true;
		}
		LogError(socketfd, String("setsockopt( ") << (bWriteSide ? "SO_SNDBUF" : "SO_RCVBUF") << " )", lRetCode);
	}
	return false;
}

bool Socket::SetToNonBlocking(int fd, bool dontblock)
{
	StartTrace1(Socket.SetToNonBlocking, "fd:" << fd << (dontblock ? " dont" : "") << " block");
	if (fd < 0) {
		return false;
	}
#if defined(WIN32)
	u_long nNonBlocking = dontblock ? 1 : 0;
	Trace("argument to pass:" << nNonBlocking);
	if ( 0 == ioctlsocket(fd, FIONBIO, &nNonBlocking) ) {
		Trace("was successful");
		return true;
	}
#else
	int flags;
	if ((flags = fcntl(fd, F_GETFL, 0)) >= 0) {
		if (dontblock) {
			flags |= O_NONBLOCK;
		} else {
			flags &= ~O_NONBLOCK;
		}
		if ( fcntl(fd, F_SETFL, flags) >= 0 ) {
			return true;
		}
	}
#endif
	String logMsg;
	SYSERROR(logMsg << "error in SetToNonBlocking fcntl  (" << SystemLog::LastSysError() << ")");

	return false;
}

//#define SOCK_TRACE
#ifdef SOCK_TRACE
#include "DiffTimer.h"

class ConnectStat
{
public:
	ConnectStat(String usage);
	~ConnectStat();

	void Use() { }

	String fUsage;
	DiffTimer fTimer;
};
#endif

//--- EndPoint ---------------------------------------
void EndPoint::LogError(const char *contextmessage)
{
	StartTrace(EndPoint.LogError);
	String logMsg(contextmessage);
	logMsg << " of socket " << (long)GetFd()
		   << " with address: " << fIPAddress << " port: " << fPort
		   << " failed (#" << (long)System::GetSystemError() << ") " << SystemLog::LastSysError();
	Trace(logMsg);
	SystemLog::Error(logMsg);
}

bool EndPoint::CreateSocket()
{
	StartTrace(EndPoint.CreateSocket);
	if ((fSockFd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
		LogError("socket()");
		return false;
	}
	return true;
}

long EndPoint::GetBoundPort()
{
	StartTrace(EndPoint.GetBoundPort);
	long socketPort = 0;
	// print out the Port we got from bind()
	struct 	 sockaddr_in assignedAddr;
	socklen_type length = sizeof(assignedAddr);

	if ( getsockname(GetFd(), (sockaddr *) &assignedAddr, &length) >= 0 ) {
		socketPort = (long)ntohs(assignedAddr.sin_port);
		Trace("bound port: " << socketPort);
	} else {
		LogError("getsockname() could not retrieve assigned port");
	}
	return socketPort;
}

bool EndPoint::BindToAddress(String &srcIpAdr, long srcPort)
{
	StartTrace(EndPoint.BindToAddress);
	if (GetFd() < 0 ) {
		return false;
	}
	if ( ( srcIpAdr.Length() > 0 ) || (srcPort > 0) ) {
		Trace("creating endpoint address");
		// use defined adress or fPort as source address
		// create the source address
		struct 	 sockaddr_in ip_addr;
		if (PrepareSockAddrInet(ip_addr, MakeInetAddr(srcIpAdr, true), srcPort)) {
			if (!Socket::SetSockoptReuseAddr(GetFd())) {
				return false;
			}

			Trace("binding socket descriptor to ip");
			if (bind(GetFd(), (struct sockaddr *) &ip_addr, sizeof(struct sockaddr_in)) < 0) {
				LogError("bind()");
				return false;
			}
		}
		srcPort = GetBoundPort();
	}
	return true;
}

unsigned long EndPoint::MakeInetAddr(String ipAddr, bool anyipaddr)
{
	if (ipAddr.Length() <= 0) {
		if (anyipaddr) {
			return INADDR_ANY;
		}
		ipAddr = GetLocalHost(); // for Servers ?
	}
	unsigned long addr = inet_addr((char *)(const char *)ipAddr); // strange cast for 370 compatibility
	// workaround to simulate valid ip address when 255.255.255.255 is given
	// which corresponds to -1L (INADDR_NONE) return code
	if ( ipAddr != "255.255.255.255" || addr != ((unsigned long) - 1L) ) {
		Assert(addr != INADDR_NONE);
	}
	return addr;
}

bool EndPoint::PrepareSockAddrInet(struct sockaddr_in &socketaddr, unsigned long ipAddr, long port)
{
	StartTrace1(EndPoint.PrepareSockAddrInet, "ip:" << (long)ipAddr << " port:" << port);
#if defined(WIN32)
	// port can be 0, this means NT assigns a port
	if (ipAddr != INADDR_NONE)
#else
	if (ipAddr != INADDR_NONE && port > 0)
#endif
	{
		memset((char *) &socketaddr, 0, sizeof(socketaddr));
		socketaddr.sin_family      = AF_INET;
		socketaddr.sin_addr.s_addr = ipAddr;
		socketaddr.sin_port        = htons((unsigned short)port);
		return true;
	}
#if defined(WIN32)
	Trace("condition [ipAddr != INADDR_NONE] failed");
#else
	Trace("condition [ipAddr != INADDR_NONE && port > 0] failed");
#endif
	return false;
}

Allocator *EndPoint::GetSocketAllocator()
{
	return fThreadLocal ? Storage::Current() : Storage::Global();
}

Socket *EndPoint::DoMakeSocket(int socketfd, Anything &clientInfo, bool doClose)
{
	StartTrace(EndPoint.DoMakeSocket);
	clientInfo["HTTPS"] = false;
	Allocator *a = GetSocketAllocator();

	Trace("allocating with Storage::" << (fThreadLocal ? "Current" : "Global") << "(): [" << (long)a << "]");
	return new (a) Socket(socketfd, clientInfo, doClose, GetDefaultSocketTimeout(), a);
}

bool EndPoint::SockOptGetError()
{
	StartTrace(EndPoint.SockOptGetError);
	int error = 0;
	bool bSuccess = Socket::GetSockOptInt(GetFd(), SO_ERROR, error);
	if ( !bSuccess || error != 0 ) {
		LogError("getsockopt() ");
		SYSWARNING("SO_ERROR code:" << (long)error);
		return true;
	}
	return false;
}

//---- ConnectorArgs ----------------------------------------------
ConnectorArgs::ConnectorArgs(const String &ipAddr, long port, long connectTimeout) :
	fIPAddress(ipAddr),  fPort(port), fConnectTimeout(connectTimeout)
{
	StartTrace(ConnectorArgs.ConnectorArgs);
}

ConnectorArgs::ConnectorArgs()  :
	fIPAddress("127.0.0.1"),  fPort(80), fConnectTimeout(0)
{
}

ConnectorArgs::~ConnectorArgs()
{
	StartTrace(ConnectorArgs.~ConnectorArgs);
}

ConnectorArgs &ConnectorArgs::operator=(const ConnectorArgs &ca)
{
	StartTrace(ConnectorArgs.operator = );

	if ( this == &ca ) {
		return *this;
	}
	fIPAddress 				= ca.fIPAddress;
	fPort 					= ca.fPort;
	fConnectTimeout			= ca.fConnectTimeout;
	return *this;
}

String ConnectorArgs::IPAddress()
{
	StartTrace(ConnectorArgs.IPAddress);

	return fIPAddress.Length() > 0 ? (const char *)fIPAddress : "127.0.0.1";
}

long ConnectorArgs::Port()
{
	StartTrace(ConnectorArgs.Port);
	return fPort;
}

String ConnectorArgs::PortAsString()
{
	StartTrace(ConnectorArgs.PortAsString);
	return String().Append(fPort);
}

long ConnectorArgs::ConnectTimeout()
{
	StartTrace(ConnectorArgs.ConnectTimeout);
	return fConnectTimeout;
}

//--- Connector --------------------------------------------
Connector::Connector(String ipAdr, long port, long connectTimeout, String srcIPAdr, long srcPort, bool threadLocal) :
	EndPoint(String(Resolver::DNS2IPAddress(ipAdr), -1, threadLocal ? Storage::Current() : Storage::Global()), port),
	fConnectTimeout(connectTimeout),
	fSrcIPAdress(Resolver::DNS2IPAddress(srcIPAdr), -1, threadLocal ? Storage::Current() : Storage::Global()),
	fSrcPort(srcPort), fSocket(0)
{
	StartTrace(Connector.Connector);
	SetThreadLocal(threadLocal);
	Trace("IPAddr: " << fIPAddress << ":" << fPort);
	Trace("TimeOut: " << fConnectTimeout);
	Trace("srcIP : " << fSrcIPAdress << ":" << fSrcPort);
}

Connector::Connector(ConnectorArgs &connectorArgs, String srcIPAdr, long srcPort, bool threadLocal) :
	EndPoint(String(Resolver::DNS2IPAddress(connectorArgs.IPAddress()), -1, threadLocal ? Storage::Current() : Storage::Global()), connectorArgs.Port()),
	fConnectTimeout(connectorArgs.ConnectTimeout()),
	fSrcIPAdress(Resolver::DNS2IPAddress(srcIPAdr), -1, threadLocal ? Storage::Current() : Storage::Global()),
	fSrcPort(srcPort), fSocket(0)
{
	StartTrace(Connector.Connector);
	SetThreadLocal(threadLocal);
	Trace("IPAddr: " << fIPAddress << ":" << fPort);
	Trace("TimeOut: " << fConnectTimeout);
	Trace("srcIP : " << fSrcIPAdress << ":" << fSrcPort);
}

Connector::~Connector()
{
	if ( fSocket ) {
		delete fSocket;
	}
}

Socket *Connector::MakeSocket(bool doClose)
{
	StartTrace(Connector.MakeSocket);
	Socket *s = 0;
	if ( (fSrcPort == 0) || (!fSocket) ) {
		if ( DoConnect()) {
			Anything clientInfo;
			clientInfo["REMOTE_ADDR"] = fIPAddress;
			clientInfo["REMOTE_PORT"] = fPort;
			System::SetCloseOnExec(GetFd());
			s = DoMakeSocket(GetFd(), clientInfo, doClose);
		}
		if (GetFd() >= 0 && ! s) {
			closeSocket(GetFd());
			fSockFd = -1;
		}
	}
	return s;
}

Socket *Connector::Use()
{
	StartTrace(Connector.Use);
	if ( !fSocket ) {
		fSocket = MakeSocket();
	}
	return fSocket;
}

iostream *Connector::GetStream()
{
	StartTrace(Connector.GetStream);
	Socket *psocket = Use();
	if ( psocket ) {
		return psocket->GetStream();
	}
	return 0;
}

Anything Connector::ClientInfo()
{
	StartTrace(Connector.ClientInfo);
	if ( fSocket ) {
		return fSocket->ClientInfo();
	} else {
		return Anything();
	}
}

bool Connector::ConnectWouldBlock()
{
#if defined(WIN32)
	long winerror = WSAGetLastError ();
	//	if ( winerror != WSAEINPROGRESS )
	return ( winerror == WSAEWOULDBLOCK );
#else
	return ( errno == EINPROGRESS );
#endif
}

bool Connector::DoConnect()
{
	StartTrace(Connector.DoConnect);
	Trace("IPAddr: " << fIPAddress << ":" << fPort);
	Trace("TimeOut: " << fConnectTimeout);
	Trace("srcIP : " << fSrcIPAdress << ":" << fSrcPort);
#ifdef SOCK_TRACE
	ConnectStat cs("Connect");
	cs.Use();
#endif
	struct sockaddr_in serv_addr;
	if (PrepareSockAddrInet(serv_addr, MakeInetAddr(fIPAddress), fPort)) {
		if (!CreateSocket()) {
			return false;
		}
		Trace("socket created, descriptor: " << (long)GetFd());
		if (!BindToAddress(fSrcIPAdress, fSrcPort)) {
			return false;
		}
		if ( !Socket::SetToNonBlocking(GetFd(), (fConnectTimeout > 0) ) ) {
			LogError(String("set socket to ") << (fConnectTimeout > 0 ? "non " : "") << "blocking mode failed: ");
			return false;
		}
		Trace("connecting to: " << fIPAddress << ":" << fPort );
		if ( (connect(GetFd(), (struct sockaddr *) &serv_addr, sizeof(serv_addr)) >= 0)
			 || ( ConnectWouldBlock() &&
				  System::IsReadyForWriting(GetFd(), fConnectTimeout) &&
				  !SockOptGetError() ) ) {
			Trace("connect successfully done")
			return true;
		}
		LogError("connect() failed");
	}
	return false;
}

#ifdef SOCK_TRACE
ConnectStat::ConnectStat(String usage)
	: fUsage(usage), fTimer()
{
}

ConnectStat::~ConnectStat()
{
	SystemLog::WriteToStderr( fUsage << " " << fTimer.Diff() << " ms" << "\n" );
}
#endif

//---- Acceptor ---------------------------------------------------------
class EXPORTDECL_FOUNDATION CallBackLocker
{
public:
	CallBackLocker(AcceptorCallBack *ac) : fCallBack(ac) {
		if (fCallBack) {
			fCallBack->Lock();
		}
	}
	~CallBackLocker()	{
		if (fCallBack) {
			fCallBack->Unlock();
		}
	}

	void Use() { }
protected:
	AcceptorCallBack *fCallBack;
};

class EXPORTDECL_FOUNDATION RevCallBackLocker
{
public:
	RevCallBackLocker(AcceptorCallBack *ac) : fCallBack(ac) {
		if (fCallBack) {
			fCallBack->Unlock();
		}
	}
	~RevCallBackLocker()	{
		if (fCallBack) {
			fCallBack->Lock();
		}
	}

	void Use() { }
protected:
	AcceptorCallBack *fCallBack;
};

class EXPORTDECL_FOUNDATION CallBackSynchronizer
{
public:
	CallBackSynchronizer(AcceptorCallBack *ac) : fCallBack(ac) { }
	~CallBackSynchronizer()	{ }

	void Wait() {
		if (fCallBack) {
			fCallBack->Wait();
		}
	}
	void Signal() {
		if (fCallBack) {
			fCallBack->Signal();
		}
	}

	void Use() { }
protected:
	AcceptorCallBack *fCallBack;
};

Acceptor::Acceptor(const char *ipadress, long port, long backlog, AcceptorCallBack *cb)
	: EndPoint(Resolver::DNS2IPAddress(ipadress), port)
	, fBackLog(backlog)
	, fCallBack(cb)
	, fAlive(false)
	, fStopped(true)
{
	StartTrace(Acceptor.Ctor);
}

Acceptor::~Acceptor()
{
	StartTrace(Acceptor.Dtor);
	StopAcceptLoop();
	if (fCallBack) 		{
		delete fCallBack;
		fCallBack = 0;
	}
	if ( fSockFd >= 0 ) {
		closeSocket(fSockFd);
	}
}

int Acceptor::PrepareAcceptLoop()
{
	StartTrace(Acceptor.PrepareAcceptLoop);
	if (!CreateSocket()) {
		return -1;
	}
	if (!Socket::SetSockoptReuseAddr(GetFd())) {
		return -1;
	}
	Trace("trying to bind to [" << fIPAddress << ":" << fPort << "]");
	if (!BindToAddress(fIPAddress, fPort)) {
		return -1;
	}
	// listen on the server socket for incoming connect calls from clients
	// configure the backlog size, the calls queued in kernel, not yet processed
	// by the accept loop, the same size as the active session
	if (listen(GetFd(), fBackLog) < 0) {
		LogError("listen()");
		return -1;
	}
	//NB: linux resolves address only correct after listen!
	fPort = GetBoundPort();
	{
		CallBackLocker cb(fCallBack);
		cb.Use();
		fAlive = true;
	}
	Trace("acceptor fd:" << GetFd());
	return 0;
}

Socket *Acceptor::DoAccept()
{
	StartTrace(Acceptor.DoAccept);
	struct 	 sockaddr_in clnt_addr;
	int      newsock = -1;
	socklen_type     clilen =  sizeof(clnt_addr);
	{
		RevCallBackLocker rcl(fCallBack);
		rcl.Use();
		// accept new connections, this call blocks
		newsock = accept(fSockFd, (struct sockaddr *) &clnt_addr, &clilen);
	}
	if (newsock < 0) {
		LogError("accept()");
		// SOP: check for stopper and set fAlive to false
	} else {
		Trace("accept fd:" << (long)newsock);
		if ( fAlive ) {
			Anything clnInfo;
			clnInfo["REMOTE_ADDR"] = inet_ntoa(clnt_addr.sin_addr);
			clnInfo["REMOTE_PORT"] = (long)ntohs(clnt_addr.sin_port);
			Trace("accept on " << inet_ntoa(clnt_addr.sin_addr));
			Trace("Port: " << (long)ntohs(clnt_addr.sin_port));

			return DoMakeSocket(newsock, clnInfo);
		}
		closeSocket(newsock);
	}
	return 0;
}

int Acceptor::RunAcceptLoop(bool once)
{
	StartTrace(Acceptor.RunAcceptLoop);
	{
		CallBackLocker cb(fCallBack);
		cb.Use();
		fStopped = false;

		while ( fAlive ) {
			Socket *psocket = DoAccept();
			if ( psocket ) {
				DoCallBack(psocket);
			}
			if (once) {
				fAlive = false; // leave loop after first accept
			}
		}
	}
	fStopped = true;
	CallBackSynchronizer(fCallBack).Signal();
	return 0;
}

void Acceptor::DoCallBack(Socket *psocket)
{
	StartTrace(Acceptor.DoCallBack);
	if (fCallBack) {
		fCallBack->CallBack(psocket);
	}
}

bool Acceptor::StopAcceptLoop(bool useConnect)
{
	StartTrace(Acceptor.StopAcceptLoop);
	CallBackLocker cbl( fCallBack );
	cbl.Use();
	if (fAlive && !fStopped && useConnect) {
		Trace("fAlive: [" << fAlive << "] fStopped: [" << fStopped << "] useConnect: [" << useConnect << "]");
		fAlive = false;
		const char *ipaddress = (fIPAddress.Length() > 0) ? (const char *)fIPAddress : GetLocalHost();

		Connector(ipaddress, fPort).Use();
		// synchronize with other thread
		CallBackSynchronizer cbs(fCallBack);
		while (!fStopped) {
			cbs.Wait();
		}
	} else {
		fAlive = false;
		fStopped = true;
	}
	Trace("On return (true) fAlive: [" << fAlive << "] fStopped: [" << fStopped << "] useConnect: [" << useConnect << "]");
	return true;
}

long Acceptor::GetBacklog()
{
	return fBackLog;
}

// BIA/RUM
String Acceptor::GetAddress()
{
	if (fIPAddress.Length() == 0) {
		return "any";
	}
	return fIPAddress;
}
