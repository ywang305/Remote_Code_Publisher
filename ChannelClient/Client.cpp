/////////////////////////////////////////////////////////////////////////
// Client.cpp - C++ Client wrapper for Win32 socket api                    //
// ver 1.0                                                                 //
// ----------------------------------------------------------------------- //
// copyright  Yaodong Wang, 2017                                           //
// All rights granted provided that this notice is retained                //
// ----------------------------------------------------------------------- //
// Language:    Visual C++, Visual Studio 2015                             //
// Platform:    HP-Q5F5, Core i3, Windows 10                               //
// Build req:   static library                                             //
// Application: Project #4, CSE687 - Object Oriented Design, S2017         //
// Author:      Yaodong Wang , Syracuse University                         //
//              ywang305@syr.edu                                           //
///////////////////////////////////////////////////////////////////////////*/
/*
* Package Operations :
*------------------ -
*  implementation of interface as MsgClient. It response to get http message
    from sending queue, and connect to the remote addreass, then sends bytes
	format stream.
*   -the endPoint is set as "IP:Port", e.g. "localhost:8082"
*   -construct httpMsg, including get/post. The format of HttpMessage
	- is like [command mode toAddr fromAddr ontent-lenght body]
	-general message has no attribute of "file"
	-attr "file" is only designed to use for post a file
	-any custom type message can be defined in the HttpMessage body
	-   Specific custom message:
	-   post , "quit"  : to tell other end  to terminiate this connection 
	-   post ,  "list:xxx" : send a file list item
	-   get,   "category:all":  request all cateory and file list
	-   get,   "category:xxx":   request xxx category and related file list
*   -It is sued by shim and object factory
*  ----------------------------
*	
*
*
* Build Process:
* --------------
* Required Files: HttpMessage.h, Client.h, Socket.h, Logger, FileSystem., Utilities.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 29 April 2017
*	- first release: meet reqs
*  ver 1.1 : 1 May 2017
*   - add specification as update "local path"
*
*/

#include "Client.h"
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../Logger/Logger.h"
#include "../FileSystem/FileSystem.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <thread>
using Show = Logging::StaticLogger<1>;
using namespace Utilities;
using Utils = StringHelper;



/////////////////////////////////////////////////////////////////////
// MsgClient class
// - was created as a class so more than one instance could be 
//   run on child thread
//
class MsgClient : public IClient
{
public:
	MsgClient(const std::string &clientId, const EndPoint &myAddr, const std::string &testpath)
		: clientId(clientId), myEndPoint(myAddr), testPath(testpath) {}
	MsgClient(const std::string &clientId, const std::string myIp, size_t myPort, const std::string &testpath)
		: clientId(clientId), myEndPoint( getClientEndPoint(myIp, myPort) ), testPath(testpath) {}
	EndPoint getClientEndPoint(const std::string ip, size_t port);
	void setServerEndPoint(const std::string &ip, size_t port);
	void setServerEndPoint(EndPoint &serverEndPoint);
	EndPoint getServerEndPoint();
	std::string getServerIp();
	size_t getServerPort();
	HttpMessage makeGetMessage(const std::string &body, const EndPoint& toAddr);
	HttpMessage makePostMessage(const std::string& body, const EndPoint& toAddr);
	void enBlockQ(HttpMessage &msg);                // to enQ new msg
	void execute(const size_t TimeBetweenMessages);  // start SC, loop deQ, this should call once
	std::string getClientId() { return clientId; }
	void setClientDir(std::string newDir);
private:
	std::string clientId;  // use as the client id!
	std::string testPath;  //"../TestFiles-Project-3", local storage
	Async::BlockingQueue<HttpMessage> sendQ_;
	EndPoint myEndPoint, serverEndPoint;
	void sendMessage(HttpMessage& msg, Socket& socket);
	bool sendFile(const std::string& filename, Socket& socket);
	bool sendFileEx(HttpMessage& msg, Socket& socket);
	void sendMessageEx(HttpMessage& msg, Socket& socket);
};

EndPoint MsgClient::getClientEndPoint(const std::string ip, size_t port)
{
	EndPoint myAddr = ip + Converter<size_t>::toString(port);
	return myAddr;
}

// ----< update client sending dir >------------------------------
void MsgClient::setClientDir(std::string newDir)
{
	testPath = newDir;
}

// ----< configure server ip and port >------------------------------
void MsgClient::setServerEndPoint(const std::string &ip, size_t port)
{
	this->serverEndPoint = ip + Converter<size_t>::toString(port);
}

// ----< configure server with EndPoint >------------------------------
void MsgClient::setServerEndPoint(EndPoint &serverEndPoint)
{
	this->serverEndPoint = serverEndPoint;
}

//  ---- < get only after configuration by set >--------
EndPoint MsgClient::getServerEndPoint()
{
	return this->serverEndPoint;
}

// server ip, for socketconnector inilization
std::string MsgClient::getServerIp()
{
	std::string ip;
	EndPoint ep = this->serverEndPoint;
	std::size_t found = ep.find(":");
	if (found != std::string::npos)
	{
		ip = ep.substr(0, found);
	}
	return ip;
}
// server port, for socketconnector inilization
size_t MsgClient::getServerPort()
{
	size_t port = 0;
	EndPoint ep = this->serverEndPoint;
	std::size_t found = ep.find(":");
	if (found != std::string::npos)
	{
		std::string portStr = ep.substr(found + 1);
		port = Converter<size_t>::toValue(portStr);
	}
	return port;
}

//----< factory for creating messages >------------------------------
/*
* This function only creates one type of message for this demo.
* - There are general type of message to create.
* - The body may be an empty string.
* - EndPoints are strings of the form ip:port, e.g., localhost:8081. This argument
*   expects the receiver EndPoint for the toAddr attribute.
*/
//----< message type 1  POST, this is regular post message,  if send file, need add attribute "file:filename", and change content-length  >------------------------------

HttpMessage MsgClient::makePostMessage(const std::string& body, const EndPoint& toAddr)
{
	HttpMessage msg;
	msg.clear();
	msg.addAttribute(HttpMessage::attribute("command", "post")); // one way to make a attr
	msg.addAttribute(HttpMessage::Attribute("mode", "oneway"));     // another way to make a attr
	msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + toAddr)); // 3rd way
	msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + this->myEndPoint));

	msg.addBody(body);  // body could be null ""
	if (body.size() > 0)
	{
		msg.addAttribute(HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size())));
	}
	return msg;
}

//----< message type 2 , get  >------------------------------

HttpMessage MsgClient::makeGetMessage(const std::string &body, const EndPoint& toAddr)
{
	HttpMessage msg;
	msg.clear();
	msg.addAttribute(HttpMessage::attribute("command", "get")); // one way to make a attr
	msg.addAttribute(HttpMessage::Attribute("mode", "duplex"));     // another way to make a attr
	msg.addAttribute(HttpMessage::parseAttribute("toAddr:" + toAddr)); // 3rd way
	msg.addAttribute(HttpMessage::parseAttribute("fromAddr:" + this->myEndPoint));

	msg.addBody(body);
	if (body.size() > 0)
	{
		msg.addAttribute(HttpMessage::attribute("content-length", Converter<size_t>::toString(body.size())));
	}
	return msg;
}


//----< send message using socket >----------------------------------

void MsgClient::sendMessage(HttpMessage& msg, Socket& socket)
{
	if (msg.bodyString() == "quit") serverEndPoint = "";
	std::string msgString = msg.toString();
	socket.send(msgString.size(), (Socket::byte*)msgString.c_str());
}

//----< send file using socket >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool MsgClient::sendFile(const std::string& filename, Socket& socket)
{
	// assumes that socket is connected

	std::string fqname = FileSystem::Path::fileSpec(this->testPath, filename);
	FileSystem::FileInfo fi(fqname);
	size_t fileSize = fi.size();
	std::string sizeString = Converter<size_t>::toString(fileSize);
	FileSystem::File file(fqname);
	file.open(FileSystem::File::in, FileSystem::File::binary);
	if (!file.isGood())
		return false;
	HttpMessage msg = makePostMessage("", "localhost:8080");
	msg.addAttribute(HttpMessage::Attribute("file", filename));
	msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));

	sendMessage(msg, socket);  // send a msg first, specify the content-length, but without body

	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];
	while (true)
	{
		FileSystem::Block blk = file.getBlock(BlockSize); //!!! core part using file stream
		if (blk.size() == 0)
			break;
		for (size_t i = 0; i < blk.size(); ++i)
			buffer[i] = blk[i];
		socket.send(blk.size(), buffer);
		if (!file.isGood())
			break;
	}
	file.close();
	return true;
}

//----< this is the entry api that sends a deQue message from a sending Queue >-----

void MsgClient::sendMessageEx(HttpMessage& msg, Socket& socket)
{
	std::string command = msg.findValue("command");
	std::string filename = msg.findValue("file");

	//send file
	if (command == "post" && filename != "")
	{
		bool status = this->sendFileEx(msg, socket);
		if (status == false) Show::write("  warning: sending file falied...\n");
	}
	else
	{
		this->sendMessage(msg, socket); // send regular messag
	}
}


//----< send file EX using socket , if msg is post file, called by sendMessageEx >-------------------------------------
/*
* - Sends a message to tell receiver a file is coming.
* - Then sends a stream of bytes until the entire file
*   has been sent.
* - Sends in binary mode which works for either text or binary.
*/
bool MsgClient::sendFileEx(HttpMessage& msg, Socket& socket)
{
	// assumes that socket is connected, incoming msg shoudld be [command:post, mode, toAddr, fromAddr, file] \n nobody!

	std::string filename = msg.findValue("file");
	if (filename != "")
	{
		std::string fqname = FileSystem::Path::fileSpec(this->testPath, filename);
		FileSystem::FileInfo fi(fqname);
		size_t fileSize = fi.size();
		FileSystem::File file(fqname);
		file.open(FileSystem::File::in, FileSystem::File::binary);
		if (!file.isGood())
			return false;

		msg.addAttribute(HttpMessage::Attribute("content-length", Converter<size_t>::toString(fileSize)));

		sendMessage(msg, socket);  // send a msg first, specify the content-length, but without body

		const size_t BlockSize = 2048;
		Socket::byte buffer[BlockSize];
		while (true)
		{
			FileSystem::Block blk = file.getBlock(BlockSize); //!!! core part using file stream
			if (blk.size() == 0)
				break;
			for (size_t i = 0; i < blk.size(); ++i)
				buffer[i] = blk[i];
			socket.send(blk.size(), buffer); // using low-level socket send
			if (!file.isGood())
				break;
		}
		file.close();
		return true;
	}
	return false;
}


// ---< let someone put a HttpMessage into BlockingQueue sendQ >----

void MsgClient::enBlockQ(HttpMessage &msg)
{
	this->sendQ_.enQ(msg);
}

//----< this defines the behavior of the client sending msg from sendQ >--------------------

void MsgClient::execute(const size_t TimeBetweenMessages)
{
	//::SetConsoleTitle("Clients Running on Threads");
	Show::attach(&std::cout);
	Show::start();
	Show::title("Demonstrating two HttpMessage Clients each running on a child thread");
	try
	{
		SocketSystem ss;
		SocketConnecter si;
		HttpMessage msg;
		while (true)
		{
			msg = this->sendQ_.deQ();

			size_t pos = msg.findAttribute("toAddr");
			if (pos < msg.attributes().size())
			{
				/*
				* Sender class will need to accept messages from an input queue
				* and examine the toAddr attribute to see if a new connection
				* is needed.  If so, it would either close the existing connection
				* or save it in a map[url] = socket, then open a new connection.
				*/
				if (this->getServerEndPoint() != msg.attributes()[pos].second)
				{
					if (this->getServerEndPoint() != "")
					{
						sendMessage(makePostMessage("quit", this->getServerEndPoint()), si);
						Show::write("\n\n  *** client " + this->clientId + " sent shutdown msg , auto sent if new toAddr ***\n" + msg.toIndentedString());
					}

					this->setServerEndPoint(msg.attributes()[pos].second);  // storage the other msg-rcv endpoint 					
					while (!si.connect(getServerIp(), getServerPort()))
					{
						Show::write("\n client waiting to connect server " + getServerEndPoint());
						::Sleep(TimeBetweenMessages);
					}
				}
				
				this->sendMessageEx(msg, si); // sendMessageEx can handle both messgage and file
				Show::write("\n\n  *** client " + this->clientId + " sent ***\n" + msg.toIndentedString());
				::Sleep(TimeBetweenMessages);
			}
		}
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}



//----< factory functions >--------------------------------------------------

IClient* ClientFactory::newInstance(const std::string &clientId, const EndPoint &myAddr, const std::string &localpath)
{
	return new MsgClient(clientId, myAddr, localpath);
}


#ifdef NTEST_SOCKET_CLIENT
// --< Test Stub >--
//----< entry point - runs two clients each on their own threads >------

int main()
{
	ClientFactory factory;

	IClient* Jim = factory.newInstance("Jim", "localhost:8081", "../Repository-Client-Project-3");
	IClient* Zuck = factory.newInstance("Zuck", "localhost:8082", "../Repository-Client-Project-3");
	Zuck->enBlockQ( Zuck->makePostMessage("<msg> I am Zuck #1, post msg </msg>", "localhost:8080"));
	Zuck->enBlockQ(Zuck->makeGetMessage("<msg> I am Zuck #2, get msg </msg>", "localhost:8080"));  // test Get msg
	Zuck->enBlockQ(Zuck->makePostMessage("quit", "localhost:8080"));    // quit

	for (auto filename : FileSystem::Directory::getFiles("../Repository-Client-Project-3", "*.cpp"))
	{
		HttpMessage fmsg = Jim->makePostMessage("", "localhost:8080"); //null body message
		fmsg.addAttribute(HttpMessage::Attribute("file", filename)); // and add attri "file" to the message
		Jim->enBlockQ(fmsg);
	}
	Jim->enBlockQ(Jim->makePostMessage("quit", "localhost:8080")); // quit
	
	
	std::thread zSndThread( [&]() { Zuck->execute(100); } );// deQ every 100 millisec apart, loop forever
	

	std::thread jSndThread([&](){ Jim->execute(100); }
	);// deQ every 100 millisec apart
	
	zSndThread.join();
	jSndThread.join();	
}

#endif
