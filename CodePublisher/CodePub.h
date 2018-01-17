#pragma once
/////////////////////////////////////////////////////////////////////
// CodePub.h - Publish source code to htm                          //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* CodePub means CodePublisher, and its work dependes project2's DepAnal
* package. It convert a source code file (c++,h) to htm presentation. In
* this process, different tags wrapped the contents. The wrappers are
* provided as member functions, including <htm> <head> <body> <nav> <pre>
* <aside>. code is wrapped in <pre> to preserve origianl format; depedent
* pakcages are lists in <nav>; and an outline list with collaps&expand is
* wrapped in tag <aside>
*   - doDepAnalAndPersist() generates the dependent results from package DepAnal
*     which implies it accesses nosqldb as implemented in project2.
*   - toHtm() publish a collection of src codes from dstPath.
*  ----------------------------
*
* Build Process:
* --------------
* Required Files: DepAnal.h, AnalyzerExec.h, PubList.h, Utilities.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 29 Mar 2017
*	- first release: meet reqs
*  ver 1.1 : 01 May 2017
*	- revised function: pass parameter category ( client ip:port addr) from CodePublisher down to DA
*
*/

#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include "../FileSystem/FileSystem.h"
#include "../Logger/Logger.h"
#include "../Utilities/Utilities.h"
#include "AnalyzerExec.h"
#include "../Analyzer/DepAnal.h"
#include "PubList.h"

namespace CodePublisher 
{
	using namespace CodeAnalysis;
	using Rslt = Logging::StaticLogger<0>;  // use for application results
	using Demo = Logging::StaticLogger<1>;  // use for demonstrations of processing
	using Dbug = Logging::StaticLogger<2>;  // use for debug output
	
	//////////////////////////////////////////////////////////////////////////////////////
	/*
	*  - CodePub hold aggregates object of TypeDep and DepAnal, the processing logic is
	*  - to extract dependency from DepAnal, then publish to repositry as htm. 
	*////////////////////////////////////////////////////////////////////////////////////
	class CodePub
	{
	public:		
		CodePub(int argc, char* argv[], string category="none");
		CodePub(int argc, char* argv[], Shell<string> *pDbShell);
		bool initCmdline(int argc, char* argv[]);
		void doDepAnalAndPersist();
		string fromCode(string srcFile);
		string wrap_Pre(string srcPath);
		string wrap_Nav(string srcFile);
		string getWebList(string srcFile);
		string wrap_Aside(string srcFile);
		string wrap_Body(string srcFile, string srcPath);
		string wrap_Head();
		string wrap_Htm(string srcPath);
		CodeAnalysisExecutive& getExec() { return exec; };
		void toHtm(string srcPath, string dstPath);
		vector<string> toHtms(string dstDir);
		~CodePub();
		string getCategory() { return category_; }
		Shell<string>* getDBShell() { return dbshell; }
 	private:
		CodeAnalysisExecutive exec;
		Shell<string> *dbshell;
		TypeAnal ta;
		DepAnal *da;
		string category_;
		int PROJECT4 = true;
	};

	// -- < constructor for project4 > --
	inline CodePub::CodePub(int argc, char* argv[], Shell<string> *pDbShell)
	{
		if (initCmdline(argc, argv))
		{
			PROJECT4 = true;

			string tmp = exec.getAnalysisPath();
			auto pos = exec.getAnalysisPath().find_last_of("\\") + 1;
			auto file = tmp.substr(pos);
			dbshell = pDbShell;
			ta.doTypeAnal();
			da = new DepAnal(dbshell, ta.GetTable(), "none");
			doDepAnalAndPersist();
		}
	}
	// -- < constructor uses cmd arguments to locate the source code collection > --
	inline CodePub::CodePub(int argc, char* argv[], string category) : category_(category)
	{
		if ( initCmdline(argc, argv) )
		{
			string tmp = exec.getAnalysisPath();
			auto pos = exec.getAnalysisPath().find_last_of("\\")+1;
			auto file = tmp.substr(pos);
			dbshell = new Shell<string>(file);
			ta.doTypeAnal();
			da = new DepAnal(dbshell, ta.GetTable(), category);
			doDepAnalAndPersist();
		}
		
	}

	inline CodePub::~CodePub()
	{
		if( da!=NULL) delete da;
		if (PROJECT4==false && dbshell != NULL) delete dbshell;
	}

	// -- < parse command line > --
	inline bool CodePub::initCmdline(int argc, char* argv[])
	{
		if (!exec.ProcessCommandLine(argc, argv))   return false;

		exec.setDisplayModes();
		exec.startLogger(std::cout);
		std::ostringstream tOut("CodePublisher");
		Utils::sTitle(tOut.str(), 3, 92, tOut, '=');
		Rslt::write(tOut.str());
		Rslt::write("\n     " + exec.systemTime());
		Rslt::flush();
		exec.showCommandLineArguments(argc, argv);
		Rslt::write("\n");

		exec.getSourceFiles();
		exec.processSourceCode(true);
		exec.complexityAnalysis();
		exec.dispatchOptionalDisplays();
		exec.flushLogger();
		exec.displayMetricSummary(50, 10);

		exec.flushLogger();
		Rslt::write("\n");
		std::ostringstream out;
		out << "\n  " << std::setw(10) << "searched" << std::setw(6) << exec.numDirs() << " dirs";
		out << "\n  " << std::setw(10) << "processed" << std::setw(6) << exec.numFiles() << " files";
		Rslt::write(out.str());
		Rslt::write("\n");
		exec.stopLogger();
		return true;
	}

	// -- < keep dependant relationship of source code collection into database > --
	inline void CodePub::doDepAnalAndPersist()
	{
		for (auto map : exec.getFileMap())
			for (auto path : map.second)
			{
				string package = path.substr(path.find_last_of("\\") + 1);
				da->doTypeMatch(package, path);
			}
		da->doDbPersist();
	}

	// -- < Convert src file to string, replacing < > with escaper "&lt;" "&gt;" > --
	inline string CodePub::fromCode(string srcFile)
	{
		stringstream ss;
		try
		{
			std::ifstream inf(srcFile);
			if (inf.good())
			{
				char tok;
				while (inf.get(tok))
				{
					if (tok == '<')  ss << "&lt;";
					else if (tok == '>') ss << "&gt;";
					else ss << tok;
				}
			}
			inf.close();
		}
		catch (std::logic_error& ex)
		{
			std::cout << "\n  " << ex.what() << "\n";
		}
		return ss.str();
	}


	// -- < "header&nav" wrap header, nav > --
	inline string CodePub::wrap_Nav(string srcFile)
	{
		stringstream ssNav;
		ssNav << "\n<header>" << srcFile << "</header>\n";
		ssNav << "\n<nav>\n" << "  Dependencies:\n";
		if (!srcFile.empty())
		{ 
			for (auto child : dbshell->queryChildren(srcFile))
			{
				ssNav << R"(  <a href=")" << child << ".htm" 
					<< R"(">)" << child << R"(</a>)" << endl;
			}
		}
		else
		{
			cout << "\n  irregular# no key found in database as "<< srcFile <<"  :  wrap_Nav, CodePub\n";
		}
		ssNav << "</nav>\n";
		return ssNav.str();
	}

	// -- < "pre" wrap source code > --
	inline string CodePub::wrap_Pre(string srcPath)
	{
		stringstream ssPre;
		ssPre << "\n<pre>\n" << fromCode(srcPath) << "\n</pre>\n";
		return ssPre.str();
	}


	// -- < used by outline wrap_aside, capture scope, class, function >--
	//    the cascading list is generated from class PGLList, PubList.h

	inline string CodePub::getWebList(string srcFile)
	{ // only return global class function
		auto lambdaIDCLASS = [this](SPtr sp)->string {
			stringstream IdClass;
			IdClass << " id=\"" << sp->name << "\" class=\"c" << sp->level << "\"";
			return IdClass.str();
		};
		stringstream list;
		list << "\n\t<ul>";
		PGLList pgl(ta.GetTable());
		SPtr sp_r = pgl.getWebList();  // we get the web list for tag <aside>, it has packages - globals - locals
		for (auto pkg : sp_r->children)
		{ if( pkg->name=="Parent.h")
		{
			auto ppackage = pkg;
		}
			list << "\n\t\t<li>" << R"(<a href="#" onclick="Toggle(')" << pkg->name << "')\">" << pkg->name << "</a>"
				<< "\n\t\t\t<ul" << lambdaIDCLASS(pkg) <<">";
			for (auto glo : pkg->children)
			{
				list << "\n\t\t\t\t<li>" << R"(<a href="#" onclick="Toggle(')" << glo->name << "')\">"  << glo->name << "</a>"
					<< "\n\t\t\t\t\t<ul" << lambdaIDCLASS(glo) << ">";
				for (auto loc : glo->children)
				{
					list << "\n\t\t\t\t\t\t<li>" << R"(<a href="#" onclick="Toggle(')" << loc->name << "')\">" << loc->name << "</a>"
						<< "\n\t\t\t\t\t\t<ul"<<lambdaIDCLASS(loc) << ">";
					for (auto lowloc : loc->children)
					{
						list << "\n\t\t\t\t\t\t\t<li>" << R"(<a href="#">)" << lowloc->name << "</a>" << "</li>";
					}
					list << "\n\t\t\t\t\t\t</ul>\n\t\t\t\t\t</li>";
				}
				list << "\n\t\t\t\t\t</ul>\n\t\t\t\t</li>";
			}
			list << "\n\t\t\t</ul>\n\t\t</li>";
		}
		list << "\n\t</ul>\n";
		return list.str();
	}

	// -- < "aside" wraps outline for hide and unhide "pre" >
	inline string CodePub::wrap_Aside(string srcFile)
	{
		stringstream ssAside;
		ssAside << "\n<aside>\n";
		
		ssAside << getWebList(srcFile);

		ssAside << "\n</aside>\n";
		return ssAside.str();
	}

	// -- < "body" wraps "header&nav", "pre", "aside" > --
	inline string CodePub::wrap_Body(string srcFile, string srcPath)
	{
		stringstream ssBody;
		ssBody << "\n<body>\n" << wrap_Nav(srcFile) << wrap_Aside(srcFile) << wrap_Pre(srcPath) << "\n</body>\n";
		return ssBody.str();
	}

	// --< head tag wraps css, javascript >--
	inline string CodePub::wrap_Head()
	{
		stringstream ssHead;
		ssHead << "\n<head>\n" << "  <title>Code Publisher</title>\n";
		ssHead << R"(  <link rel="stylesheet" type="text/css" href="style/site.css">)" <<endl;
		ssHead << R"(  <script src="js/myScript.js"></script>)";
		ssHead << "\n</head>\n";
		return ssHead.str();
	}

	// -- < "htm" is root wrapper that wraps  "prologue", "head", "body" > --
	inline string CodePub::wrap_Htm(string srcPath)
	{
		string srcFile = dbshell->queryKeyByData(srcPath);
		stringstream ssHtm;
		ssHtm << "<!-------------------------------------------------------------------------\n"
		<< srcFile << "\nPublished:  " << exec.systemTime() << endl
  << R"(  Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017

	Note:
		-Markup characters in the text part, enclosed in <pre>...< / pre> have to be
			replaced with escape sequences, e.g., < becomes &lt; and > becomes &gt;
		-Be careful that you don't replace genuine markup characters with escape 
			sequences, e.g., everything outside of the <pre>...< / pre> section.
  ----------------------------------------------------------------------------->)";
		ssHtm << "\n<!DOCTYPE html>\n<html>\n";
		ssHtm << wrap_Head();
		ssHtm << wrap_Body(srcFile, srcPath);
		ssHtm << "\n</html>\n";
		return ssHtm.str();
	}

	// -- < convert src to htm, and publish as dstPath>--
	inline void CodePub::toHtm(string srcPath, string dstPath)
	{
		try
		{ 
			string htm = wrap_Htm(srcPath);

			std::ofstream outf(dstPath);
			outf << htm;
			outf.close();
		}
		catch (std::logic_error& ex)
		{
			std::cout << "\n  " << ex.what() << "\n";
		}
	}

	// -- < publish a collection of src codes from directory: dstPath >--
	inline vector<string> CodePub::toHtms(string dstDir)
	{
		vector<string> path_htms;
		string codePath, htmPath;
		for (auto key : dbshell->queryAllKeys())
		{
			codePath = dbshell->queryValue(key).data;
			htmPath = dstDir + "\\" + key + ".htm";
			toHtm(codePath, htmPath);
			path_htms.push_back(htmPath);
		}
		return path_htms;
	}

	

}


