#pragma once
/////////////////////////////////////////////////////////////////////
// Client.h - Socket Client to connect other endpoint              //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* Interface of Client semantics is declared here
*   - the endPoint is set as "IP:Port", e.g. "localhost:8082"
*   - construct httpMsg, including get post
*   - It is sued by shim and object factory
*  ----------------------------
*
* Build Process:
* --------------
* Required Files: HttpMessage.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 29 April 2017
*	- first release: meet reqs
*  ver 1.1 : 1 May 2017
*   - add specification as update "local path"
*
*/

#ifdef IN_DLL
#define DLL_DECL __declspec(dllexport)
#else
#define DLL_DECL __declspec(dllimport)
#endif

#include "../HttpMessage/HttpMessage.h"
using EndPoint = std::string;

///////////////////////////////////////////////////////////////////
// MsgClient Interface
// - for exposing to Cli Shim  
struct IClient {
	virtual std::string getClientId() = 0;
	virtual void setClientDir(std::string newDir) = 0;
	virtual EndPoint getClientEndPoint(const std::string ip, size_t port) = 0;
	virtual void setServerEndPoint(const std::string &ip, size_t port) = 0;
	virtual void setServerEndPoint(EndPoint &serverEndPoint) = 0;
	virtual EndPoint getServerEndPoint() = 0;
	virtual std::string getServerIp() = 0;
	virtual size_t getServerPort() = 0;
	virtual HttpMessage makeGetMessage(const std::string &body, const EndPoint& toAddr) = 0;
	virtual HttpMessage makePostMessage(const std::string& body, const EndPoint& toAddr) = 0;

	virtual void enBlockQ(HttpMessage &msg) = 0;
	virtual void execute(const size_t TimeBetweenMessages) = 0;
};


///////////////////////////////////////////////////////////////////
// factory to concrete instance
//
struct ClientFactory
{
	IClient* newInstance(const std::string &clientId, const EndPoint &myAddr, const std::string &localpath);
};




 

