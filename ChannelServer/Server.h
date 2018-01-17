#pragma once
/////////////////////////////////////////////////////////////////////
// Server.h - Socket Server to listen connection request            //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* Interface of socket server semantics is declared here
*   - the endPoint is set as "IP:Port", e.g. "localhost:8082"
*   - construct httpMsg, including get post
	- Two semantics are declare here
*	-   1 exec is the main runing that keep listening new coming connect
*   -   2 deQ that let use to retrieve message async.
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
*
*/
#include "../HttpMessage/HttpMessage.h"


///////////////////////////////////////////////////////////////////
// MsgClient Interface
// - for exposing to Cli Shim 
struct IServer
{
	virtual void execute(size_t listenport) = 0; // self loop
	virtual HttpMessage deBlockQ() = 0;
};

///////////////////////////////////////////////////////////////////
// factory to concrete instance
//
struct ServerFactory
{
	IServer* newInstance(const std::string &repositoryPath);
};


