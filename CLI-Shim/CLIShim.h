#pragma once
///////////////////////////////////////////////////////////////////////
// CLIShim.h - C++\CLI layer is a proxy for calls into MockChannel   //
//                                                                   //
// Note:                                                             //
//   - build as a dll so C# can load                                 //
//   - link to MockChannel static library                            //
//                                                                   //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2017         //
///////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* Channel Deliever semantics is declared here
*   - handDownPostMessage:  bridge general message channel from user to socket library 
	- handDownPostFileMessage:  bridge file transfering channel from user to socket lib
	- handDownGetFileMessage: bridget request message from user to socket lib
	- handUpFileMessage() : deliever native socket message upto managed app
	- exeClientXmt:  client loop that trying to send out message
	- exeServerLsnAndRcv:  server loop that keep listening and recv any connected message.
  Message defines format for .net application to receive.
*  ----------------------------
*
* Build Process:
* --------------
* Required Files: Client.h Server.h, and their static libary
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 29 April 2017
*	- reviesed along with socket client& server libary
*
*/


#include "../ChannelClient/Client.h"
#include "../ChannelServer/Server.h"
#include <string>
using namespace System;
using namespace System::Threading;

//////////////////////////////////////////////////
////   .net applicaiton message format
////     to help parse delivered native message
////////////
public ref class Message
{
public:
	property String^ command;
	property String^ msgBody;
	property String^ filename;
};

///////////////////////////////////////////////////////////
////   Shim 
////     mixed class to merge native and managed conversion
////////////
public ref class Shim
{
public:
  Shim(String^ clientId, size_t myPort, String^ mySendPath, String^ myRecvPath);
  void updateMyClientDir( String^ newDir);
  void handDownPostMessage(String^ msg, String^ toAddr);
  void handDownPostFileMessage(String^ filename, String^ toAddr);
  void handDownGetFileMessage(String^ filename, String^ toAddr);
  Message^ handUpFileMessage();
  void exeClientXmt();
  void exeServerLsnAndRcv(Object^ serverPort);
  String^ stdStrToSysStr(const std::string& str);
  std::string sysStrToStdStr(String^ str);
private:
  IClient* pClient;
  IServer* pServer;
  Thread^ clientDeQThre;
  Thread^ servrLstnEnQThre;
};


