///////////////////////////////////////////////////////////////////
// TypeAnal_DepAnal_test.cpp - Test stub for TypeAnal.h DepAnal.h//
// ver 1.0                                                     //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017 //
///////////////////////////////////////////////////////////////

#ifdef TEST_DEPANA
#include "TypeAnal.h"
#include "DepAnal.h"
namespace CodeAnalysis {
	using namespace CodeAnalysis;

	//----< test stub >--------------------------------------------

	int main()
	{
		TypeAnal ta;
		Shell<string> dbshell;
		DepAnal da(dbshell, ta.GetTable());

		/*
		TypeAnal ta;

		ta.doTypeAnal();
		std::cout<< ta.ShowTable();

		// check type matching for each file in the collection
		Shell<string> dbshell;

		DepAnal da( dbshell, ta.GetTable());

		for (auto map : exec.getFileMap())
		for (auto path : map.second)
		{
		string package = path.substr(path.find_last_of("\\")+1);
		da.doTypeMatch(package, path);
		}
		da.doDbPersist();
		*/
	}	
}

#endif // TEST_DEPANA
