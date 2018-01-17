/////////////////////////////////////////////////////////////////////////
// Client.cpp - C++ Client wrapper for Win32 socket api                    //
// ver 1.0                                                                 //
// ----------------------------------------------------------------------- //
// copyright  Yaodong Wang, 2017                                           //
// All rights granted provided that this notice is retained                //
// ----------------------------------------------------------------------- //
// Language:    Visual C++, Visual Studio 2015                             //
// Platform:    HP-Q5F5, Core i3, Windows 10                               //
// Build req: "CodePub.h"                                                  //
// Application: Project #4, CSE687 - Object Oriented Design, S2017         //
// Author:      Yaodong Wang , Syracuse University                         //
//              ywang305@syr.edu                                           //
///////////////////////////////////////////////////////////////////////////*/

#include "Server.h"
#include "../HttpMessage/HttpMessage.h"
#include "../Sockets/Sockets.h"
#include "../FileSystem/FileSystem.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"
#include "../Utilities/Utilities.h"
#include <string>
#include <iostream>
#include <thread>
using Show = Logging::StaticLogger<1>;
using namespace Utilities;




/////////////////////////////////////////////////////////////////////
// ClientHandler class
/////////////////////////////////////////////////////////////////////
// - instances of this class are passed by reference to a SocketListener
// - when the listener returns from Accept with a socket it creates an
//   instance of this class to manage communication with the client.
// - You need to be careful using data members of this class
//   because each client handler thread gets a reference to this 
//   instance so you may get unwanted sharing.
// - I may change the SocketListener semantics (this summer) to pass
//   instances of this class by value.
// - that would mean that all ClientHandlers would need either copy or
//   move semantics.
//
class ClientHandler
{
public:
	ClientHandler(const std::string &repositoryPath, Async::BlockingQueue<HttpMessage>& rcvQ)
		: testPath(repositoryPath), recvQ_(rcvQ) {}
	void operator()(Socket socket);
private:
	bool connectionClosed_;
	std::string testPath;  // server's repository path
	HttpMessage readMessage(Socket& socket);
	void processPostMessageBody(HttpMessage& msg, Socket& socket);
	void processGetMesageBody(HttpMessage& msg, Socket& socket);
	bool readFile(const std::string& filename, size_t fileSize, Socket& socket);
	Async::BlockingQueue<HttpMessage>& recvQ_;
};


/////////////////////////////////////////////////////////////////////
// MsgServer class
// - Containing socket listener, clientHandler 
//   clientHander setup recv msg run on child thread
//
class MsgServer : public IServer
{
public:
	MsgServer(const std::string &repositoryPath) : fileRepoPath_(repositoryPath) {}
	void execute(size_t listenport); // start ClientListener and create a new ClientHandler with the port, then loop
	                                 // listenport should be myAddr
	HttpMessage deBlockQ();
private:
	std::string fileRepoPath_;
	Async::BlockingQueue<HttpMessage> recvQ_;
};







//----< this defines processing to frame messages >------------------

HttpMessage ClientHandler::readMessage(Socket& socket)
{
	connectionClosed_ = false;
	HttpMessage msg;

	// read message attributes

	while (true)
	{
		std::string attribString = socket.recvString('\n');
		if (attribString.size() > 1)
		{
			HttpMessage::Attribute attrib = HttpMessage::parseAttribute(attribString);
			msg.addAttribute(attrib);
		}
		else
		{ // Terminator term_ = "\n" in HttpMessage.h,  [attribute \n attribute \n ... attribute\n \n body]
			break;
		}
	}
	// If client is done, connection breaks and recvString returns empty string

	if (msg.attributes().size() == 0)
	{
		connectionClosed_ = true;
		return msg;
	}
	// read body if POST - all messages in this demo are POSTs

	if (msg.attributes()[0].second == "post")
	{
		this->processPostMessageBody(msg, socket);
	}
	else if (msg.attributes()[0].second == "get")
	{
		this->processGetMesageBody(msg, socket);
	}
	return msg;
}

// ----< process "command:post" type of msg >--------------------
void ClientHandler::processPostMessageBody(HttpMessage& msg, Socket& socket)
{	// is this a file message?
	std::string filename = msg.findValue("file");
	if (filename != "")
	{
		size_t contentSize;
		std::string sizeString = msg.findValue("content-length");
		if (sizeString != "")
		{
			contentSize = Converter<size_t>::toValue(sizeString);
			this->readFile(filename, contentSize, socket);
		}
		else { sizeString = "0"; }
		// construct message body
		msg.removeAttribute("content-length");
		std::string bodyString = "<file>" + filename + ", filesize = " + sizeString + "</file>";
		sizeString = Converter<size_t>::toString(bodyString.size());
		msg.addAttribute(HttpMessage::Attribute("content-length", sizeString));
		msg.addBody(bodyString);
	}
	else  // read gerneral message body { post msg with regular <Message> body }
	{
		size_t numBytes = 0;
		size_t pos = msg.findAttribute("content-length");
		if (pos < msg.attributes().size())
		{
			numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
			Socket::byte* buffer = new Socket::byte[numBytes + 1];
			socket.recv(numBytes, buffer); // continued recv is the body part, since the header part has been finished by recvString("\n") in while loop at begining...
			buffer[numBytes] = '\0';
			std::string msgBody(buffer);
			msg.addBody(msgBody);
			delete[] buffer;
		}
	}
}

void ClientHandler::processGetMesageBody(HttpMessage& msg, Socket& socket)
{  // same as post regular message  part
	size_t numBytes = 0;
	size_t pos = msg.findAttribute("content-length");
	if (pos < msg.attributes().size())
	{
		numBytes = Converter<size_t>::toValue(msg.attributes()[pos].second);
		Socket::byte* buffer = new Socket::byte[numBytes + 1];
		socket.recv(numBytes, buffer); // continued recv is the body part, since the header part has been finished by recvString("\n") in while loop at begining...
		buffer[numBytes] = '\0';
		std::string msgBody(buffer);
		msg.addBody(msgBody);
		delete[] buffer;
	}
}

//----< read a binary file from socket and save >--------------------
/*
* This function expects the sender to have already send a file message,
* and when this function is running, continuosly send bytes until
* fileSize bytes have been sent.
*/
bool ClientHandler::readFile(const std::string& filename, size_t fileSize, Socket& socket)
{
	std::string fqname = FileSystem::Path::fileSpec(this->testPath, filename );
	FileSystem::File file(fqname);
	file.open(FileSystem::File::out, FileSystem::File::binary);
	if (!file.isGood())
	{
		/*
		* This error handling is incomplete.  The client will continue
		* to send bytes, but if the file can't be opened, then the server
		* doesn't gracefully collect and dump them as it should.  That's
		* an exercise left for students.
		*/
		Show::write("\n\n  can't open file " + fqname);
		return false;
	}

	const size_t BlockSize = 2048;
	Socket::byte buffer[BlockSize];

	size_t bytesToRead;
	while (true)
	{
		if (fileSize > BlockSize)
			bytesToRead = BlockSize;
		else
			bytesToRead = fileSize;

		socket.recv(bytesToRead, buffer);

		FileSystem::Block blk;
		for (size_t i = 0; i < bytesToRead; ++i)
			blk.push_back(buffer[i]);

		file.putBlock(blk);
		if (fileSize < BlockSize)
			break;
		fileSize -= BlockSize;
	}
	file.close();
	return true;
}


//----< receiver functionality is defined by this function >---------

void ClientHandler::operator()(Socket socket)
{
	/*
	* There is a potential race condition due to the use of connectionClosed_.
	* If two clients are sending files at the same time they may make changes
	* to this member in ways that are incompatible with one another.  This
	* race is relatively benign in that it simply causes the readMessage to
	* be called one extra time.
	*
	* The race is easy to fix by changing the socket listener to pass in a
	* copy of the clienthandler to the clienthandling thread it created.
	* I've briefly tested this and it seems to work.  However, I did not want
	* to change the socket classes this late in your project cycle so I didn't
	* attempt to fix this.
	*/
	while (true)
	{
		HttpMessage msg = readMessage(socket);
		this->recvQ_.enQ(msg);
		Show::write("\n\n  +++++ server recvd message +++++\n" + msg.headerString() + msg.bodyString());

		if (connectionClosed_ || msg.bodyString() == "quit")
		{
			Show::write("\n\n  clienthandler thread is terminating");
			break;
		}
	}
}

// this define server's semantic
// *start ClientListener and create a new ClientHandler with the port, then loop, run this only once on server

void MsgServer::execute(size_t listenport)
{
	//::SetConsoleTitle("HttpMessage Server - Runs Forever");
	Show::attach(&std::cout); 
	Show::start();
	Show::title("\n  HttpMessage Server started");

	try
	{
		SocketSystem ss;
		SocketListener sl(listenport, Socket::IP6);  // server with "IP6" can listen both IP4& IP6 connect request!!
		ClientHandler cp(this->fileRepoPath_, this->recvQ_);
		sl.start(cp);  // loop waiting for incoming message unless "quit"
			
		/*
		* Since this is a server the loop below never terminates.
		* We could easily change that by sending a distinguished
		* message for shutdown.
		*/
		while (true)
		{
			
		} // if out of this scope, listener will be destroyed!
	}
	catch (std::exception& exc)
	{
		Show::write("\n  Exeception caught: ");
		std::string exMsg = "\n  " + std::string(exc.what()) + "\n\n";
		Show::write(exMsg);
	}
}

// ---< let someone get a HttpMessage from BlockingQueue recvQ >----

HttpMessage MsgServer::deBlockQ()
{
	return this->recvQ_.deQ();
}


//----< factory functions >--------------------------------------------------

IServer* ServerFactory::newInstance(const std::string &repositoryPath)
{
	return new MsgServer(repositoryPath);
}


//----< test stub >--------------------------------------------------
#ifdef NTEST_SOCKET_SERVER
int main()
{
	//MsgServer ms("../Repository-Server-Project-3");
	//ms.execute(8080);  /* this loop forever to listen*/
	//std::thread deRcvQThread(
	//	[&]() {
	//      while(true){
	//			HttpMessage msg = ms.deBlockQ();
	//			Show::write("\n\n  *** extract a message from Server's BlockingQ: ***\n" + msg.headerString() + msg.bodyString());
	//	    }
	//   }
	//);
	//deRcvQThread.detach();

	ServerFactory factory;
	IServer* Lisa = factory.newInstance("../Repository-Server-Project-3");

	Lisa->execute(8080); /* this loop forever to listen */ // 8080 is listened port, also generally should be "myAddr" 

	std::thread rcvThread([&]() { 
		while (true)
		{
			HttpMessage msg = Lisa->deBlockQ();
			Show::write("\n\n  *** extract a message from Server's BlockingQ: ***\n" + msg.headerString() + msg.bodyString());
		}		
	});

	rcvThread.join();
}
#endif
