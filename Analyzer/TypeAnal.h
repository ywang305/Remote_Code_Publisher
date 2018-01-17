#pragma once

/////////////////////////////////////////////////////////////////////
// TypeAnal.h - Establish Type Table                               //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package defines a TypeAnal class that to create an TypeTable for file collection.
* The Type Table is built on the specific Parser that in the parser it configures a set 
* of ActionAndRules to detect global function and class type.
* The TypeTable is a hash map that take required typename as key, package name as value.
*  ----------------------------
*
* Build Process:
* --------------
* Required Files: Tokenizer.h, AbstrSynTree.h, ActionsAndRules.h, Utilities.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 02 Mar 2017
*	- first release: establish Type Table, extract AST and Tokens
*/

#include "../Parser/ActionsAndRules.h"
#include "../Utilities/Utilities.h"
#include "../Tokenizer/Tokenizer.h"
#include "../AbstractSyntaxTree/AbstrSynTree.h"
#include <iostream>
#include <functional>
#include <unordered_map>

#pragma warning (disable : 4101)  // disable warning re unused variable x, below

namespace CodeAnalysis
{
	using namespace std;

	//////////////////////////////////////////////////////////////////////////////////////
	/*
	*  - TypeAnal uses global function and class definition to search requested types.
	*  - keep type-file ralationship into Type Table
	*/
	using Key = std::string;
  
	class TypeAnal
	{
	public:
		using Value = struct { string name; string type; string package; string path; string space; };
		using SPtr = std::shared_ptr<ASTNode*>;

		TypeAnal();
		void doTypeAnal();
		std::vector<Value>& GetTable();
		std::string ShowTable();
		ASTNode* getAstRoot();
	private:
		void DFS(ASTNode* pNode);
		void AddToTable(ASTNode *pnode);
		AbstrSynTree& ASTref_;
		ScopeStack<ASTNode*> scopeStack_;
		Scanner::Toker& toker_;
		vector<Value> table_;
	};

	//----< constructor >-----

	inline TypeAnal::TypeAnal() :
		ASTref_(Repository::getInstance()->AST()),
		scopeStack_(Repository::getInstance()->scopeStack()),
		toker_(*(Repository::getInstance()->Toker()))
	{
		std::function<void()> test = [] { int x; };  // This is here to test detection of lambdas.
	}                                              // It doesn't do anything useful for dep anal.

	

	

	// --- < root of AST > --
	inline ASTNode* TypeAnal::getAstRoot()
	{
		return ASTref_.root();
	}

	//----< retrieve AST, demonstrate its structure and content, then store to Type Table >-----

	inline void TypeAnal::doTypeAnal()
	{
		//std::cout << "\n  starting type analysis ,  scanning AST:\n";
		ASTNode* pRoot = getAstRoot();
		//TreeWalk(pRoot);
		AddToTable(pRoot);
	}

	// condition matching
	inline bool isMatched(ASTNode* pNode)
	{
		static std::string toDisplay[] = {
			"namespace", "function", "lambda", "class", "struct", "enum", "alias", "typedef"
		};

		for (std::string type : toDisplay)
		{
			if ( pNode->type_ == type && pNode->name_ != "main")
				return true;
		}
		return false;
	}


	// DFS
	inline void TypeAnal::DFS(ASTNode* pNode)
	{
		if (isMatched(pNode))
		{
			if (pNode->name_ == "T")
			{
				pNode->name_ = "template member";
				pNode->type_ = "function";
				pNode->parentName_ = [this, pNode ]() { auto tb = this->GetTable();
				for (auto p = begin(tb); p != end(tb); ++p)
					if (p->package == pNode->package_ && p->type=="class") return p->name;
				return string("");
				}();
			}
			if (pNode->parentName_ == "") pNode->parentName_ = "Global Namespace";
			this->table_.push_back({pNode->name_, pNode->type_, pNode->package_, pNode->path_, pNode->parentName_});
		}
		for (auto pChild : pNode->children_)
		{
			if (pNode->type_ == "alias" || pNode->type_ == "typedef")
			{
				pChild->parentName_ = pNode->parentName_;
			}
			else {
				pChild->parentName_ = pNode->name_;
			}
			
			DFS(pChild);
		}
			
	}

	//----< store to hash map: keep typename as key, pacagename as value >-----
	inline void TypeAnal::AddToTable(ASTNode *node)
	{
		DFS(node);
	}

	inline std::vector<TypeAnal::Value>& TypeAnal::GetTable()
	{
		return table_;
	}

	//----< demonstrate the content of Type Table >-----

	inline std::string TypeAnal::ShowTable()
	{
		std::ostringstream oss;
		oss.setf(std::ios::adjustfield, std::ios::left);
		
		Utilities::StringHelper::title(" type_name   :  type  :    package    :   namespace", oss);
		oss << "\n";
		for (auto &item : table_)
		{
			oss <<"   "<< setw(15) << item.name << " : " << setw(15)<< item.type << " : "<< setw(15) << item.package << "  :  " << item.space<<endl;
		}
		return oss.str();
	}

}