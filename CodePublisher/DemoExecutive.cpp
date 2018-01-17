/*////////////////////////////////////////////////////////////////////////////
// DemoExecutive.cpp  - Project4 demonstration                              //
// ver 2.0                                                                 //
// ----------------------------------------------------------------------- //
// copyright  Yaodong Wang, 2017                                           //
// All rights granted provided that this notice is retained                //
// ----------------------------------------------------------------------- //
// Language:    Visual C++, Visual Studio 2015                             //
// Platform:    HP-Q5F5, Core i3, Windows 10                               //
// Build req:   CodePub.h    Utilities.h                                              //
// Application: Project #3, CSE687 - Object Oriented Design, S2015         //
// Author:      Yaodong Wang , Syracuse University                         //
//              ywang305@syr.edu                                           //
///////////////////////////////////////////////////////////////////////////*/
/*
Note for project 3:
- The demo of project3 goes here.
- This project uses DepAna package and nosqldb in project 2.
- THe Publisher program is a local webpage provider, it convert source file, e.g.
xxx.cpp into xxx.cpp.htm file. The converted htm is relocated to a dedicative
direcotry under the solution path - CodePublisher_Test_Repository. In this direcotry
there is a sub directorys providing ".css", ".js" to meet htm style and behavior
of collaps/expand list elements.
- The htm consists tags like <nav> to demo depedency, <aside> to demo cascading lists
and <pre> to demo source code.
- Cmd args is set up in the project property to demo specific path handling.
- Package CodePub uses package PubList to help produce 3-level list for htm.


Note for project 4:
  this demo is building on the previous projects including nosql-database, code analyzer
  , code publisher...,  and additional socket communication funciton is implementaiton.
  For compatible consdieration, some direcotry didn't change, as:
    codepublisher - read source from "TA-TEST-PROJECT2"
				  - Code_Publisher_Test is where output htmls
	
*/

#include <string>
#include <thread>
#include <unordered_map>
#include "CodePub.h"
#include "../NoSqlDB/DBShell/Shell.h"
#include "../ChannelClient/Client.h"
#include "../ChannelServer/Server.h"
#include "../Cpp11-BlockingQueue/Cpp11-BlockingQueue.h"

using namespace std;
using namespace CodePublisher;

////////////////////////////////
//   demo pro3 helper class     
//
//////////////////////////////

class Demo3Exec
{
	using Utils = Utilities::StringHelper;
public:
	static void Req3()
	{
		std::string fs = FileSystem::Path::fileSpec(".", "CodePub.h");
		//std::cout << "\n  Path::fileSpec(\".\",\"temp.txt\") = " << fs;
		//std::string dir = FileSystem::Path::getPath(fs);
		//std::cout << "\n  Path::getPath(\"" + fs + "\") = " << path;
		std::string currdir = FileSystem::Directory::getCurrentDirectory();
		//std::cout << "\n  Path::getFullFileSpec(\"" + fs + "\") = " << ffs;

		Utils::Title("\n    providing publisher");
		std::cout << "\n        Publisher program's path : " << currdir+"\\Debug\\CodePublisher.exe";
	}

	static void Req5(string dir)
	{
		std::string fs = FileSystem::Path::fileSpec(dir + "\\style", "site.css");
		
		std::string ffs = FileSystem::Path::getFullFileSpec(fs);
		Utils::Title("\n    css and javascript");
		std::cout << "\n       CSS path : " << ffs;
		fs = FileSystem::Path::fileSpec(dir+"\\js", "myScript.js");
		ffs = FileSystem::Path::getFullFileSpec(fs);
		std::cout << "\n       javascript path : " << ffs;
	}

	static void Req8(CodeAnalysisExecutive& exec, int argc, char* argv[])
	{
		Utils::Title("\n Req8 - Processing command line");

		std::ostringstream out;
		out << "\n     Path: \"" << FileSystem::Path::getFullFileSpec(argv[1]) << "\"\n     Args: ";
		for (int i = 2; i < argc - 1; ++i)
			out << argv[i] << ", ";
		out << argv[argc - 1];
		cout << out.str() << endl;
		Utils::title("------------------------------------"); cout << endl;
		for (auto map : exec.getFileMap())
		{
			cout << "  pattern :   " << map.first << "\n";
			for (auto path : map.second)
			{
				string package = path.substr(path.find_last_of("\\") + 1);
				cout << "    Processing...   " << path << endl;
			}
		}
	}

	// --< core part of Code Publisher, parameter dstDir reprensent for "where htms stored"
	static void Req_Publisher(string dstDir, int argc, char* argv[])
	{
		CodePub cp(argc, argv);
		auto pubset = cp.toHtms(dstDir);

		for (auto htm : pubset)
		{
			string browser{ "start " };
			browser += htm;
			std::system(browser.c_str());
		}
		Utils::Title("\n  Launching brower ... ");
		Req8(cp.getExec(), argc, argv);
	}

};


////////////////////////////////
//   demo pro4 helper class     
//
//////////////////////////////
class Demo4RemoteServer
{
public:
	////  bewared that this server default runs at endpoint "localhost:8080"
	Demo4RemoteServer() = delete;
	Demo4RemoteServer(string srcDir, string dstDir, int argc, char* argv[], Shell<string> *pshell ): 
		srcDir_(srcDir), dstDir_(dstDir), pDbshell(pshell)
	{
		/// replace argv[2] with srcDir
		auto pchar = argv[1];
		for (auto itor = srcDir.begin(); itor != srcDir.end(); ++itor)
		{
			*pchar = *itor;
			pchar++;
		}
		*pchar = '\0';

		ServerFactory servMaker;
		LisaRcv = servMaker.newInstance(srcDir);	// "srcDir" if rcv files , server keeps them there	
		ClientFactory clntMaker;
		LisaSnd = clntMaker.newInstance("Lisa", "localhost:8080", dstDir);  //"dstDir" here is to send htms, that published by CodePublisher
	
		LisaRcv_listenThread = new std::thread (
			[&]() { LisaRcv->execute(8080); }  /* this loop forever to listen */ // 8080 is listened port, also generally should be "myAddr"
		);

		LisaSnd_deQThread = new std::thread([&]() { LisaSnd->execute(100); });  // deQ every 100 millisec apart
	}

	void start( int argc, char* argv[])
	{
		Utilities::StringHelper::Title("\n CodePublisher Server start");
		LisaRcv_listenThread->detach();
		LisaSnd_deQThread->detach();
		while (true)
		{
			HttpMessage msg = LisaRcv->deBlockQ();
			responseMsgWrapper(msg, argc, argv);
		}	
	}

	~Demo4RemoteServer()
	{
		delete LisaRcv;
		delete LisaSnd;
	}

private:
	// ---------< Server business logic to handle "post message", "post file", "get file" >-------------

	void responseMsgWrapper(HttpMessage& msg, int argc, char* argv[])
	{
		if (msg.findValue("command") == "post" && msg.findValue("file") == "")
		{	// General message, should be 'quit'
			responseGeneralPostMessage(msg, argc, argv);	
		}
		else if (msg.findValue("command") == "post" && msg.findValue("file") != "")
		{   //// if post a file, wait until a "quit" to call codepublisher
			if (msg.findValue("fromAddr") == "localhost:8081")
				rcvingFileFlag_Fawcett = true;
			else if (msg.findValue("fromAddr") == "localhost:8082")
				rcvingFileFlag_Jackson = true;
			file_addr_map[msg.findValue("file")] = msg.findValue("fromAddr");
		}
		else if (msg.findValue("command") == "get" && (msg.bodyString()=="listall" || msg.bodyString()== "localhost:8081" || msg.bodyString() == "localhost:8082"))
		{	
			responseGetListMessage(msg);
		}
		else
		{
			responseGetFileAndDepdentMessage(msg);
		}
	}

	// ----< repsonse General post message >------
	void responseGeneralPostMessage(HttpMessage& msg, int argc, char* argv[])
	{
		/// if "quit" msg, check whethear rcvFileFlag true, if yes, go publishing
		if (msg.bodyString() == "quit")
		{//// batch produce htm from files, if rcvFileFlag = true, means still recving, should wait until file xmit finished.
			if (msg.findValue("fromAddr") == "localhost:8081" &&  rcvingFileFlag_Fawcett == true)
			{
				rcvingFileFlag_Fawcett = false;
			}
			else if (msg.findValue("fromAddr") == "localhost:8082" && rcvingFileFlag_Jackson==true )
			{
				rcvingFileFlag_Jackson = false;
			}
			
			Req_Publisher( srcDir_, dstDir_, argc, argv, msg.findValue("fromAddr") );
		}
	}

	// ----< repsonse category list request, sending back the category and related files >------
	void responseGetListMessage(HttpMessage& msg)
	{
		string msgBody = msg.bodyString();

		// send back files under the category: all or specified

		if (msgBody == "listall")
		{   	
			///////// simplify for DEMO only
			LisaSnd->enBlockQ( LisaSnd->makePostMessage("localhost:8081", msg.findValue("fromAddr")));
			LisaSnd->enBlockQ( LisaSnd->makePostMessage("localhost:8082", msg.findValue("fromAddr")));

			/// send back file list , not files
			for (auto element : file_addr_map)
			{
				HttpMessage listmsg = LisaSnd->makePostMessage( element.first, msg.findValue("fromAddr"));
				LisaSnd->enBlockQ(listmsg);
			}
		}
		else
		{  // get file list under specified category
			for (auto element: file_addr_map)
			{  
				if (msg.bodyString() == element.second)
				{
					HttpMessage listmsg = LisaSnd->makePostMessage(element.first, msg.findValue("fromAddr"));
					LisaSnd->enBlockQ(listmsg);
				}	
			}
		}
		LisaSnd->enBlockQ(LisaSnd->makePostMessage("quit", msg.findValue("fromAddr")));
	}

	void responseGetFileAndDepdentMessage(HttpMessage& msg)
	{
		auto filename = msg.bodyString();

		if (this->pDbshell->queryKey(filename))
		{
			auto fmsg = LisaSnd->makePostMessage("", msg.findValue("fromAddr")); //null body message
			fmsg.addAttribute(HttpMessage::Attribute("file", filename + ".htm")); // and add attri "file" to the message
			LisaSnd->enBlockQ(fmsg);

			////////XXXXXXXXXXXXX///////
			LisaSnd->enBlockQ(LisaSnd->makePostMessage("demo:" + filename, msg.findValue("fromAddr")));

			for (auto depenFilename : this->pDbshell->queryChildren(filename))
			{
				auto fmsg = LisaSnd->makePostMessage("", msg.findValue("fromAddr")); //null body message
				fmsg.addAttribute(HttpMessage::Attribute("file", depenFilename + ".htm")); // and add attri "file" to the message
				LisaSnd->enBlockQ(fmsg);

				////////XXXXXXXXXXXXX///////
				LisaSnd->enBlockQ(LisaSnd->makePostMessage("demo:"+depenFilename, msg.findValue("fromAddr")));
			}

			LisaSnd->enBlockQ(LisaSnd->makePostMessage("quit", msg.findValue("fromAddr")));
		}

	}

	// --< core part of Code Publisher, parameter dstDir reprensent for "where htms stored"
	void Req_Publisher(string srcDir, string dstDir, int argc, char* argv[], string category)
	{
		CodePub cp(argc, argv, pDbshell);
		auto pubset = cp.toHtms(dstDir);
	}

	IServer *LisaRcv;
	IClient *LisaSnd;
	std::thread *LisaRcv_listenThread, *LisaSnd_deQThread;
	string srcDir_, dstDir_;
	bool rcvingFileFlag_Fawcett = false;
	bool rcvingFileFlag_Jackson = false;
	std::unordered_map<std::string, std::string> file_addr_map;
	Shell<string> *pDbshell;
};


#ifdef TEST_DEMOPRO4
// -- <  tets stub >----


// --< Demo pro4 :  ..\Parser  *.h    *.cpp     /f      /r     >--
//             argv[1]  argv[2] argv[3]  argv[4]  argv[5]

int main(int argc, char* argv[])
{
	/////  at server end,
	///  srcDir = rcv direcotry for c++, header source file;
	///  dstDir = send directory,  for htms

	::SetConsoleTitle("HttpMessage Server - Runs Forever for codePublish");

	string srcDir = R"(..\TA-TEST-Project-2)";  // where the src codes are. This will replace argv[1] in Req_Publisher
	string dstDir = R"(CodePublisher_Test_Repository)"; // dstDir = "where htms be published"
	auto pshell = new Shell<string>("TA-TEST-Project-2");
	Demo4RemoteServer Lisa(srcDir, dstDir, argc, argv, pshell );
	Lisa.start( argc, argv );


	return 0;
}
#endif




#ifdef TEST_DEMOPRO3
//  ----< project 3 demo >-----


// --< Demo :  ..\Parser  *.h    *.cpp     /f      /r     >--
//             argv[1]  argv[2] argv[3]  argv[4]  argv[5]

int main(int argc, char* argv[])
{

	for (int i = 0; i < argc; ++i)
	{
		auto what = argv[i];
		std::cout << what << " ";
	}
	std::cout << endl; 


	//string dstDir = R"(..\CodePublisher_Test_Repository)";
	string dstDir = R"(CodePublisher_Test_Repository)";
	//FileSystem::Directory::create(dstDir);

	DemoExec::Req_Publisher(dstDir, argc, argv);
	DemoExec::Req3();
	DemoExec::Req5(dstDir);

	return 0;
}
#endif