/////////////////////////////////////////////////////////////////////////
// CLIShim.cpp - C++\CLI layer is a proxy for calls into MockChannel   //
//                                                                     //
// Note:                                                               //
//   - build as a dll so C# can load                                   //
//   - link to MockChannel static library                              //
//                                                                     //
// Jim Fawcett, CSE687 - Object Oriented Design, Spring 2017 
//   revised by Yaodong Wang , Spring 2017                             //
/////////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* Implementation
*   - Note:
		Constructor: two child threads started as constructing,
			1, client's deQ msg form sending Blocking Q
				, that let it asyn to avoid blocked
			2, server's listen and recv thread that also run async
		Delivering semantic are distinct between "up" and "down'
			hand up, using deQ BlockingQ, and .net app is receiver,
				therefore could block the receiver. So app user should
				use hand-up async.
			hand down, .net user is sender, that only conser is to enQ
				and no asyn need to avoid blocking.
*  ----------------------------
*
* Build Process:
* --------------
* Required Files: CLIShim.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 29 April 2017
*	- reviesed along with socket client& server libary
*
*/

#include "CLIShim.h"
#include <iostream>

using namespace System::Threading;

//----< convert std::string to System.String >---------------------------

String^ Shim::stdStrToSysStr(const std::string& str)
{
  return gcnew String(str.c_str());
}
//----< convert System.String to std::string >---------------------------

std::string Shim::sysStrToStdStr(String^ str)
{
  std::string temp;
  for (int i = 0; i < str->Length; ++i)
	  temp += char(str[i]);
  return temp;
}
//----< Constructor sets up sender and receiver >------------------------

Shim::Shim(String^ clientId, size_t myPort, String^ mySendPath, String^ myRecvPath)
{
	String^ myEndPoint = "localhost:" + myPort.ToString();
	ClientFactory clientfactory;
	ServerFactory serverfactory;
	pClient = clientfactory.newInstance(sysStrToStdStr(clientId), sysStrToStdStr(myEndPoint), sysStrToStdStr(mySendPath));
	
	pServer = serverfactory.newInstance(sysStrToStdStr(myRecvPath));

	clientDeQThre = gcnew Thread(gcnew ThreadStart(this, &Shim::exeClientXmt));
	servrLstnEnQThre = gcnew Thread(gcnew ParameterizedThreadStart(this, &Shim::exeServerLsnAndRcv)); 
	
	clientDeQThre->Start( );
	servrLstnEnQThre->Start(myPort); // should listen to its own Address
}

//----< updating client's self port >---------------------------------------

void Shim::updateMyClientDir(String^ newDir)
{
	pClient->setClientDir(sysStrToStdStr(newDir));
}

//----< put message into channel >---------------------------------------

void Shim::handDownPostMessage(String^ msg, String^ toAddr)
{
	auto postMsg = pClient->makePostMessage(sysStrToStdStr(msg), sysStrToStdStr(toAddr));
	pClient->enBlockQ(postMsg);
}

//----< put file upload into channel >---------------------------------------

void Shim::handDownPostFileMessage(String^ filename, String^ toAddr)
{
	auto postFMsg = pClient->makePostMessage("", sysStrToStdStr(toAddr));
	postFMsg.addAttribute(HttpMessage::Attribute("file", sysStrToStdStr(filename)));
	pClient->enBlockQ(postFMsg);
}

//----< put file download request into channel >---------------------------------------

void Shim::handDownGetFileMessage(String^ filename, String^ toAddr)
{
	auto postFMsg = pClient->makeGetMessage(sysStrToStdStr(filename), sysStrToStdStr(toAddr));
	pClient->enBlockQ(postFMsg);
}

//----< retrieve message from channel >----------------------------------

Message^ Shim::handUpFileMessage()
{
	HttpMessage httpMsg = pServer->deBlockQ();
	Message^ msg = gcnew Message();
	std::string cmd = httpMsg.findValue("command"); // post, get
	std::string fileName = httpMsg.findValue("file");
	std::string body = httpMsg.bodyString();
	if (cmd != "")
	{
		msg->command = stdStrToSysStr(cmd);
		if (fileName != "") msg->filename = stdStrToSysStr(fileName);
		if (body != "") msg->msgBody = stdStrToSysStr(body);
	}
	return msg;
}

// ------< auto loop,  client's deQ of blocking send Quque out >---------------------------

void Shim::exeClientXmt()
{
	pClient->execute(100);  //  deQ every 100 millisec apart, loop forever
}

// ------< auto loop,  server's listen and enQ newcoming mesgs >---------------------------

void Shim::exeServerLsnAndRcv(Object^ serverPort)
{
	pServer->execute((size_t)serverPort); /* this loop forever to listen the PORT and rcv*/
}


#ifdef TEST_CLISHIM
//////////< test stub >//////////////
int main()
{
  
  Shim^ pShim1 = gcnew Shim(gcnew String("client A"), 8080, gcnew String("../Repository-Client-Project-3"), gcnew String("../Repository-Server-Project-3"));
  Shim^ pShim2 = gcnew Shim(gcnew String("client B"), 8081, gcnew String("../Repository-Client-Project-3"), gcnew String("../Repository-Server-Project-3"));
  
  pShim1->handDownPostFileMessage( pShim1->stdStrToSysStr("ConfigureParser.cpp") , "localhost:8081");  // enQ client's sendingQ
  pShim1->handDownGetFileMessage(pShim1->stdStrToSysStr("Request a demo file.xxx"), "localhost:8081"); 
  pShim1->handDownPostMessage(pShim1->stdStrToSysStr("quit"), "localhost:8081");
  
  while (true)
  {
	  Message^ pReply = pShim2->handUpFileMessage();
	  std::cout << "\n <CLISHIM> received message \"" << pShim2->sysStrToStdStr(pReply->command + ", " +  pReply->filename + ", "+ pReply->msgBody) << "\"";
	  std::cout << "\n\n";
  }

}
#endif
