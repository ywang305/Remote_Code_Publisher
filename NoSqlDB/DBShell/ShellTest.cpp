///////////////////////////////////////////////////////////////
// ShellTest.cpp - Test stub for Shell                       //
// ver 1.0                                                   //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017 //
///////////////////////////////////////////////////////////////

#ifdef TEST_SHELL
#include "Shell.h"
#include "../Utilities/Utilities.h"

//----< test stub >--------------------------------------------
void ShowVecInfo(const vector<string>& vec)
{
	for (auto u : vec)
		cout << u << " \n";
	cout << endl << endl;
}

void testor1()
{
	Title("test req 3 and 4");
	Element<std::string> e1, e2, e3;
	e1.key = "n0001";
	e1.name = "DataModel";	e1.category = "NoSqlDBModel";	e1.description = "keep in-memory database element";
	e1.data = "DataModel.h";	e1.achildKey = "n0002";	e1.achildKey = "n0004";
	NoSqlDb<string> db("DemoFile");
	Shell<string> shell("DemoFile");
	shell.addItem(e1);
	ShowVecInfo(shell.queryChildren(e1.key));
	shell.addDepend(e1.key, "n0009");
	ShowVecInfo(shell.queryChildren(e1.key));
	shell.delDepend(e1.key, "n0002");
	ShowVecInfo(shell.queryChildren(e1.key));
	shell.renewDescp(e1.key, "cache IN_MEM Data Model with string data");
	cout << shell.queryValue(e1.key).description.getValue() << endl;
	e2.key = "n0001";
	e2.name = "Persist";	e2.category = "PersistHelp";	e2.description = "persist db from cache to xml";
	e2.data = "PersistHelp.h";	e2.achildKey = "n0003";	e2.achildKey = "n0004";
	shell.renewValue(e1.key, e2);	cout << shell.queryValue(e1.key).name.getValue() << endl;
	subTitle("test req 7: suporrted queries.\n");
	putLine();
	e3.key = "n0003";	e3.name = "Shell";	e3.category = "DBShell";
	e3.description = "command interface for query";
	e3.data = "Shell.h";	e3.achildKey = "n0001";	e3.achildKey = "n0004";	e3.achildKey = "n0005";
	shell.addItem(e3);
	vector<string> keys;
	subTitle("\ntest keys parttern with substr");
	keys = shell.queryKeysContainSubStr("n");
	for (auto k : keys) cout << k << endl;
	subTitle("\ntest keys parttern queryKeysContainItemname");
	keys = shell.queryKeysContainItemname("She");
	for (auto k : keys) cout << k << endl;
	subTitle("\ntest keys parttern queryKeysContainData");
	keys = shell.queryKeysContainData("ell");
	for (auto k : keys) cout << k << endl;
	subTitle("\ntest keys parttern queryKeysContainData");
	keys = shell.queryKeysContainTime("0", Shell<string>::getMyTime());
	for (auto k : keys) cout << k << endl;
	subTitle("\ntest keys filter pattern");
	keys = shell.queryKeysFilterCondition("e", "B");
	for (auto k : keys) cout << k << endl;
}

void testor2()
{
	Shell<string> dbshell("DemoFile");

	auto vec = dbshell.getDB().getKeys();
	ShowVecInfo(vec);
}

int main() 
{
	// testor1();
	testor2();
	system("pause");
}

#endif