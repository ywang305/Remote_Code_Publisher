#pragma once
/////////////////////////////////////////////////////////////////////
// PubList.h - a tool package dedicated to cascading list          //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* as required in package CodePub, an outline list should be demo on the published
* website. This list mainly outlines the published package structure, e.g., 
* package name, global members, local members..., PubList provide structure that
* facilitate implmentation an object of the list.
* Here are two structures designed to carry list information:  PGLItem, ListNode
*   - PGLItem represents an item of Package-Global-Local. 
*   - ListNode represents an tree structure for implement cascading list, that the 
*	  level indicates cascading order - 0: packages, 1: globals, 2: locals. Through
*     level traversing tree, we can extract packages, and its children members, as
*	  well as grandchildren local members.
* A class PGLList encapused PGLItem and ListNode, and owns methods to generate the 
* cascading list based on extracting the ASTree.
*   - it owns a table to store a serial of PGLItem, which generated from retrieving 
*	  AST *tree.
*   - various query methods retrieve the table to generate ListNode tree, via DFS AST.
*   - ListNode tree root is return as an entry to the list.
*  ----------------------------
*
* Build Process:
* --------------
* Required Files: AbstrSynTree.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 29 Mar 2017
*	- first release
*/


#include <sstream>
#include <string>
#include <fstream>
#include <vector>
#include "../Analyzer/TypeAnal.h"
#include "../AbstractSyntaxTree/AbstrSynTree.h"


namespace CodePublisher
{
	using namespace std;
	using namespace CodeAnalysis;

	// PGL(packages( global members( local members )) )


	// 3-level tree structure to describe cascading list structure.

	struct ListNode {
		int level;
		string name;
		vector<std::shared_ptr<ListNode>> children;
		ListNode(int l, string n) : level(l), name(n) {}
	};
	using SPtr = std::shared_ptr<ListNode>;

	//////////////////////////////////////////////////////////////////////////////////////
	/*
	*  - PGList generate the list with a root of ListNode, 
	*  - the processing logic is to DFS ASTree to extract different level of items,
	*    e.g., -1 root, 0 - package, 1 - global class/func, 2- local function,
	*////////////////////////////////////////////////////////////////////////////////////

	class PGLList
	{
	public:
		PGLList(std::vector<TypeAnal::Value> typetable);
		vector<TypeAnal::Value> getTable();
		SPtr getWebList();
	private:
		vector<string> queryPackages();
		vector<string> queryGlobalsByPackage(string package);
		vector<string> queryLocalsByGlobal(string package, string global);
		vector<TypeAnal::Value> pglTable;
	};

	// ---< pass ASTNode root to dfs extraction >---

	inline PGLList::PGLList(std::vector<TypeAnal::Value> type_table): pglTable(type_table)
	{

	}
	/*inline PGLList::PGLList( ASTNode* pNode)
	{
		buildTable(pNode);
	}*/
	

	/*
	// --< wrapper of dfs >---
	inline void PGLList::buildTable( ASTNode* pNode)
	{
		DFS(pNode, 1);  //0 package,  1 global, 2 local

	}

	
	//  ---< retrieve ASTNode tree, extract to own table >---
	inline void PGLList::DFS(ASTNode* pNode, int level)
	{
		if (doDisplay(pNode))
		{
			if (pNode->parentName_ == "") pNode->parentName_ = "Global Namespace";
			pglTable.push_back(PGLItem(level, pNode->name_, pNode->parentName_, pNode->package_));
			++level;
		}
		if (level <=2 )
		{
			for (auto pChild : pNode->children_)
			{
				DFS(pChild, level);
			}
		}
	}


	//----< demonstrate the specific types >-----

	inline bool PGLList::doDisplay( ASTNode* pNode)
	{
		static std::string toDisplay[] = {
			"function", "lambda", "class", "struct", "enum", "alias", "typedef"
		};
		for (std::string type : toDisplay)
		{
			if (pNode->type_ == type)
			{
				if (pNode->type_ == "lambda") pNode->name_ = "lambda";
				return true;
			}		
		}
		return false;
	}*/



	//  ---< ... >---
	inline vector<TypeAnal::Value> PGLList::getTable()
	{
		return pglTable;
	}

	//  ---< level 0: package >---
	inline vector<string> PGLList::queryPackages()
	{
		vector<string> pkgs;
		for (auto itor = begin(pglTable); itor != end(pglTable); ++itor)
		{
			string pkg = itor->package;
			if (end(pkgs) == std::find(begin(pkgs), end(pkgs), pkg))
			{
				pkgs.push_back(pkg);
			}
		}
		return pkgs;
	}

	//  ---< level 1: globals refering to specific pkg >---
	inline vector<string> PGLList::queryGlobalsByPackage(string package)
	{
		vector<string> globals;
		for (auto itor = begin(pglTable); itor != end(pglTable); ++itor)
		{
			if ( package == itor->package && itor->space=="Global Namespace")
			{
				globals.push_back(itor->name);
			}
		}
		if (globals.empty())
		{
			for (auto itor = begin(pglTable); itor != end(pglTable); ++itor)
			{
				if (package == itor->package)
				{
					globals.push_back(itor->name);
				}
			}
		}
		return globals;
	}

	//  ---< level2: locals that belong to specifi glocal >---
	inline vector<string> PGLList::queryLocalsByGlobal(string package, string global)
	{
		vector<string> locals;
		for (auto itor = begin(pglTable); itor != end(pglTable); ++itor)
		{
			if (itor->package==package && global == itor->space )
			{
				if (end(locals) == find(begin(locals), end(locals), itor->name))
				{
					locals.push_back(itor->name);
				}
			}
		}
		return locals;
	}

	//  ---< public interface to return list    4 levels>---
	inline SPtr PGLList::getWebList()
	{
		SPtr sp_r( new ListNode(-1, "root") ); // root node
		for (string p : queryPackages())
		{
			SPtr sp_p (new ListNode(0, p));  // package
			sp_r->children.push_back(sp_p);
			for (string g : queryGlobalsByPackage(p)) // globals
			{
				SPtr sp_g( new ListNode(1, g) );
				sp_p->children.push_back(sp_g);
				for (string local : queryLocalsByGlobal(p, g))  //locals
				{
					SPtr sp_l(new ListNode(2, local));
					sp_g->children.push_back(sp_l);
					//sp_g->children.push_back(std::make_shared<ListNode>(2, local));
					for (string local_2 : queryLocalsByGlobal(p, local))   // locals-lower level
					{
						sp_l->children.push_back(std::make_shared<ListNode>(3, local_2));
					}
				}
			}
		}
		return sp_r;
	}
}