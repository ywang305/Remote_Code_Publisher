#pragma once

/////////////////////////////////////////////////////////////////////
// DepAnal.h - Type mathching for File dependency analysis         //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package support tokenizing a specific file and do anlysis on dependency to collection
* support specific path and file requested,
* manage a NoSqlDb object to help persist the analysis result
* the NoSqlDb support assembly a file and its depdendencies into an nosqldb element 
* the element will be hold in-memory first, then persist to xml.
* the element has structure as : Key- filename, Name- filename, category- pattern(header/cpp),
*   description- specific path,  data - filename, childrenkey- a vector of dependent filenames
*
* Build Process:
* --------------
* Required Files: Tokenizer.h, TypeAnal.h, Utilities.h, Utilities.cpp
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 02 Mar 2017
*	- first release:
*  ver 1.1 : 04 Mar 2017
*   - matching funtionality between a single file and collection
*   - access to Project 1- NoSqlDB
*  ver 1.2 : 05 Mar 2017
*   - fullfil depanal and persist to NoSqlDB
*  ver 1.3 : 03 April 2017
*   - bug fixed, the dependency logic concsider namespace.
*  ver 1.4 : 01 May 2017
*   - add "category" member for project 4, this is to keep request ip:port address.
*/

#include "../Parser/ActionsAndRules.h"
#include "../Utilities/Utilities.h"
#include "../Tokenizer/Tokenizer.h"
#include "../AbstractSyntaxTree/AbstrSynTree.h"
#include "../NoSqlDB/DBShell/Shell.h"
#include "TypeAnal.h"
#include <iostream>
#include <functional>
#include <unordered_map>

#pragma warning (disable : 4101)  // disable warning re unused variable x, below

namespace CodeAnalysis
{
	using namespace std;
	using Utils = Utilities::StringHelper;
	//////////////////////////////////////////////////////////////////////////////////////
	/*
	*  - DepAnal hold TT from TypeAnal package, and file that need to be analysized
	*  - The NoSqlDB object "dbshell" is called to persist anaysis if token match with TT
	*/
	using Key = std::string;
	using TypeTable = vector<TypeAnal::Value>;
	class DepAnal
	{
	public:
		using SPtr = std::shared_ptr<ASTNode*>;

		DepAnal() = default;
		DepAnal(Shell<string>* dbshell, TypeTable table, string category="none");
		void doTypeMatch(std::string pacakage, std::string fileSpec);
		bool doDbPersist();
		string demoDepAnal();
	private:
		Element<string> AssemblyElement(string package, string filepath);
		AbstrSynTree& ASTref_;
		ScopeStack<ASTNode*> scopeStack_;
		Scanner::Toker& toker_;
		
		TypeTable table_;
		Shell<string> *dbshell_;
		string category_;
	};

	//----< constructor use 1,Project1's NoSqlDb class to help persist result; 2, typetable >-----

	inline DepAnal::DepAnal(Shell<string>* dbshell, TypeTable table, string category) :
		ASTref_(Repository::getInstance()->AST()),
		scopeStack_(Repository::getInstance()->scopeStack()),
		toker_(*(Repository::getInstance()->Toker())),
		dbshell_(dbshell),
		table_(table),
		category_(category)
	{
		std::function<void()> test = [] { int x; };  // This is here to test detection of lambdas.
	}                                              // It doesn't do anything useful for dep anal.


	//----< check wrapped namespace typename >-----
	inline string hasMachedTypename(string tok, TypeTable& table, string &fileSpec)
	{
		for (const auto& item : table)
		{
			if (tok == item.name && fileSpec != item.path)
			{
				return tok;
			}
		}
		return "";
	}

	//----< check custom namespace typename >-----
	inline string hasMatchedNamespace(string tok, TypeTable& table, string& fileSpec)
	{
		for (const auto& item : table)
		{
			if (tok == item.space && fileSpec != item.path)
			{
				return tok;
			}
		}
		return "";
	}



	//----< check "fileSpe"c owns namespace "space" >-----

	inline bool hasNamespaceFromFile(string fileSpec, const string &space, TypeTable &table)
	{
		if (space == "Global Namespace" ) return true;
		for (auto itor = begin(table); itor != end(table); ++itor)
		{
			if (itor->path == fileSpec && itor->space==space)
			{ 
				return true;
			}
		}
		return false;
	}

	//----< check matched items out of typetable >-----
	inline TypeTable getMatchedItems(string tok, TypeTable& table, string fileSpec)
	{
		TypeTable items;
		for (const auto& item : table)
		{
			if (tok == item.name && fileSpec != item.path && hasNamespaceFromFile(fileSpec, item.space, table))
			{
				bool flag = true;
				for (auto itor = begin(items); itor != end(items); ++itor)
				{
					if (itor->package == item.package)
					{
						flag = false;
						break;
					}
				}
				if( flag )	items.push_back(item);
			}
		}
		return items;
	}


	//----< tokenize the parameter package, if match the TT, call databse to store >-----
	inline void DepAnal::doTypeMatch(std::string package, std::string fileSpec)
	{
		dbshell_->addItem(AssemblyElement(package, fileSpec)); // Assembly tokenized pacakge info as a nosqldb element
		try
		{ // start tokenizing
			// if token match with TT, add a dependent key(=dep file name) to the Assemblyed element
			std::ifstream in(fileSpec);
			if (!in.good())
			{
				std::cout << "\n  can't open " << fileSpec << "\n\n";
				return;
			}
			Scanner::Toker toker;	toker.returnComments(false); 	toker.attach(&in);
			do {
				std::string tok = toker.getTok();
				if (tok == "\n") continue;
				auto items = getMatchedItems(tok, table_, fileSpec);
				if(!items.empty()) dbshell_->renewDescp(package, "dependency exists");
				while (!items.empty())
				{
					dbshell_->addDepend(package, items.back().package);
					items.pop_back();
				}
	
			} while (in.good());

		}
		catch (std::logic_error& ex)
		{
			std::cout << "\n  " << ex.what() << "\n";
		}
	}

	//----< Assebly an NoSqlDB element based on package to be analyzed >---- -

	inline Element<string> DepAnal::AssemblyElement(string file, string filepath)
	{
		Element<string> elem;
		elem.key = file;
		elem.name = file;
		elem.description = "No dependency"; // temperary initialize as this description
		elem.data = filepath;
		elem.category = category_;
		/*if (file.find(".h") != std::string::npos) elem.category = "header";
		else if (file.find(".cpp") != std::string::npos) elem.category = "cpp";
		else elem.category = "none";*/
		return elem;
	}

	//----< Persist the database cache to ../nosqlda.xml >---- 

	inline bool DepAnal::doDbPersist()
	{
		dbshell_->getDB().persistToXml();
		return true;
	}

	inline string DepAnal::demoDepAnal()
	{
		std::ostringstream oss;
		Utils::title("\n  Result.... list all dependencies for each file in the collection ", oss);
		for (auto key : dbshell_->getDB().getKeys())
		{
			oss << "\n file \"" << dbshell_->queryValue(key).name.getValue() << "\" depends :     ";
			for (auto kidKey : dbshell_->queryChildren(key))
			{
				oss << kidKey << " , ";
			}
		}
		return oss.str();
	}

}