#pragma once
#include <sstream>
#include "../DBShell/Shell.h"
#include "../Utilities/Utilities.h"
using namespace std;

class Widget
{
public:
	Widget(const std::string& str = "") : state(str) {}
	std::string value() { return state; }
private:
	std::string state;
};

string ShowVecInline(vector<string> vec)
{
	std::ostringstream out;
	for (auto u : vec)
		out << "  " << u;
	out << "\n";
	return out.str();
}


class TestNoSqlDB
{public:
	void req1();
	void req2();
	void req3();
	void req4();
	void req5();
	void req6();
	void req7();
	void req8();
	void req9();
	void req12();

	void IniE0()
	{
		Widget widget("a Widget type object");
		e0.key = "n0003";
		e0.name = "Widget";
		e0.category = "Testor";
		e0.description = "to test custom type for in-mem data model";
		e0.data = widget;	e0.achildKey = "n0001";	e0.achildKey = "n0002";	e0.achildKey = "n0005";
	}
	Element<string> InitE1(string key, string name, string category, string descr, string data)
	{
		Element<string> e;
		e.key = key; e.name = name; e.category = category; e.description = descr; e.data = data;
		return e;
	}
	TestNoSqlDB()
	{
		IniE0();
		elist.push_back(InitE1("n0001", "DataModel", "NoSqlDBModel", "keep in-memory database element", "DataModel.h"));
		elist.push_back(InitE1("n0002", "Persist", "PersistHelp", "persist db from cache to xml", "PersistHelp.h"));
		elist.push_back(InitE1("n0003", "Shell", "DBShell", "command interface for query", "Shell.h"));
		elist.at(0).achildKey = "n0002"; elist.at(0).achildKey = "n0004";
		elist.at(1).achildKey = "n0003"; elist.at(1).achildKey = "n0004";
		elist.at(2).achildKey = "n0001"; elist.at(2).achildKey = "n0004"; elist.at(2).achildKey = "n0005";
	}
private:
	vector<Element<string>> elist;
	Element<Widget> e0;
	Shell<string> shell;
};

void TestNoSqlDB::req2()
{
	Title("Req 2: template class providing key/value in-mem database");
	subTitle("\n  Creating and saving NoSqlDb elements with string data");
	for (auto e : elist)
	{
		e.ShowElem();
		std::cout.setf(std::ios::adjustfield, std::ios::left);
		std::cout << "    " << std::setw(8) << "data" << " : " << e.data << std::endl;
	}
	subTitle("\n  Creating and saving NoSqlDb elements with Widget object");
	std::cout << e0.ShowElem();
	std::cout << "    " << std::setw(8) << "data" << " : " << "Widge data" << std::endl;
}

void TestNoSqlDB::req3()
{
	putLines(2);
	Title("Req 3: support add/delete key/value pair");
	subTitle("add 3 items into in-memory database, check element with string data in memory");
	shell.addItem(elist[0]); shell.addItem(elist[1]); shell.addItem(elist[2]);

	std::cout << "\n    " << std::setw(8) << "cached items number " << " : " << shell.getDB().getCount();
	std::cout << "\n    " << std::setw(8) << "query all keys" << " : " << ShowVecInline(shell.getDB().getKeys());

	subTitle("delete a items with key=n0003");
	shell.delItem(elist[2].key);
	std::cout << "\n    " << std::setw(8) << "cached items number " << " : " << shell.getDB().getCount();
	std::cout << "\n    " << std::setw(8) << "query all keys" << " : " << ShowVecInline(shell.getDB().getKeys());
	//shell.addItem(e3);
}

void TestNoSqlDB::req4()
{
	putLines(2);
	Title("Req 4: edit values");
	subTitle("add children relationship to (n0001), add keys: n0007, n0008");
	cout << "\n before, e1's children =    " << ShowVecInline(shell.queryChildren(elist[0].key));
	shell.addDepend(elist[0].key, "n0007");
	shell.addDepend(elist[0].key, "n0008");
	cout << " after, (n0001)'s children =    " << ShowVecInline(shell.queryChildren(elist[0].key));

	subTitle("delete children relationship on e1, delete keys: n0007");
	shell.delDepend(elist[0].key, "n0007");
	cout << "\n after, (n0001)'s children =    " << ShowVecInline(shell.queryChildren(elist[0].key));

	subTitle("edit metadata on (n0001), ");
	cout << "\nrenew name : newNoSqlDataModel, category : newCategory, and description\n";
	shell.renewName(elist[0].key, "newNoSqlDataModel");
	shell.renewCateg(elist[0].key, "newCategory");
	shell.renewDescp(elist[0].key, "cache IN_MEM Data Model with string data");
	Element<string> tempElement = shell.queryValue(elist[0].key);
	std::cout << "\n    " << std::setw(8) << "new name " << " : " << tempElement.name;
	std::cout << "\n    " << std::setw(8) << "new category " << " : " << tempElement.category;
	std::cout << "\n    " << std::setw(8) << "new description " << " : " << tempElement.description;
	putLines(2);

	subTitle("replace existing value which key=n0001");
	tempElement.name = "TestExecu";
	tempElement.category = "Testor";
	tempElement.description = "to test custom project";
	tempElement.data = "Startup.cpp";
	tempElement.achildKey = "n0001";
	tempElement.achildKey = "n0002";
	tempElement.achildKey = "n0003";
	tempElement.achildKey = "n0005";

	shell.renewValue(elist[0].key, tempElement);
	cout << shell.queryValue(elist[0].key).ShowElem();
}

void TestNoSqlDB::req5()
{
	putLines(2);
	Title("Req 5:");
	subTitle("persist to  nosqldb.xml");
	string xml = shell.getDB().persistToXml();
	cout << xml << endl;
	subTitle("restore element state ");
	shell.getDB().updateCacheByDB();
	cout << "\n" << ShowVecInline( shell.getDB().getKeys() );
}

void TestNoSqlDB::req6()
{
	putLines(2);
	Title("Req 6: auto save after 3 times write/edit into in-memory db");
	cout << "\n  (1) add a key/value element into db...\n  (2) add a dependency to that element\n  (3) renew its description metadata\n";
	shell.addItem(elist[2]);
	shell.addDepend(elist[2].key, "n0009");
	shell.renewDescp(elist[2].key, "command Interface for query, called by TestExec package");
}

void TestNoSqlDB::req7()
{
	putLines(2);
	Title("Req 7:");
	subTitle("Query value of a key: n0002");
	if (shell.queryKey("n0002"))
		cout << shell.queryValue("n0002").ShowElem() << endl;
	subTitle("children of key n0002");
	putLine();
	cout << ShowVecInline(shell.queryChildren(elist[1].key));
	subTitle("keys that contain \"00\"");
	cout << endl << ShowVecInline(shell.queryKeysContainSubStr("00"));
	subTitle("keys that contain item name: \"sis\"");
	cout << endl << ShowVecInline(shell.queryKeysContainItemname("sis"));
	subTitle("keys that category contains: \"el\"");
	cout << endl << ShowVecInline(shell.queryKeysContainCategory("el"));
	subTitle("keys that data contains: \"Shell\"");
	cout << endl << ShowVecInline(shell.queryKeysContainData("Shell"));
	subTitle("keys that time interval: from start to now");
	cout << endl << ShowVecInline(shell.queryKeysContainTime("0", getTime()));
}

void TestNoSqlDB::req8()
{
	putLines(2);
	Title("Req 8:");
	subTitle("keys that filtered condition : name contain \"e\" && category contain \"B\"");
	cout << endl << ShowVecInline(shell.queryKeysFilterCondition("e", "B"));
}

void TestNoSqlDB::req9()
{
	putLines(1);
	Title("Req 9:");
	subTitle("keys that union condition : name contain \"e\" || category contain \"B\"");
	cout << endl << ShowVecInline(shell.queryKeysUnionCondition("e", "B"));
}

void TestNoSqlDB::req12()
{   putLines(2);
	Title("Req 12: expression query");
	//cout << "\n use express: %COMMAND Args\n";
	//cout << "e.g.:  %children arg1: query children of key \n";
	//cout << "       %name arg1: query keys contain name \n";	cout << "       %category arg1: query keys contain category \n";
	//cout << "       %data arg1: query keys contain data \n";	cout << "       %time arg1 arg2: query keys within time interval \n";
	//cout << "       %filter arg1 arg2: 2 queries with filter pattern \n";	cout << "       %union arg1 arg2: 2 queries with union pattern \n";
		subTitle("%children n0003"); putLine();
		{string cmd, arg1, arg2;
		istringstream ss("%children n0003");
		ss >> cmd >> arg1 >> arg2;
		cout << ShowVecInline(shell.queryExpression(cmd, arg1, arg2)); }
	putLine();
	{subTitle("%name Shell"); putLine();
	string cmd, arg1, arg2;
	istringstream ss("%name Shell");
	ss >> cmd >> arg1 >> arg2;
	cout << ShowVecInline(shell.queryExpression(cmd, arg1, arg2)); }
	putLine();
	{subTitle("%category PersistHelp"); putLine();
	string cmd, arg1, arg2;
	istringstream ss("%category PersistHelp");
	ss >> cmd >> arg1 >> arg2;
	cout << ShowVecInline(shell.queryExpression(cmd, arg1, arg2)); }
	putLine();
	{subTitle("%data Model"); putLine();
	string cmd, arg1, arg2;
	istringstream ss("%data Model");
	ss >> cmd >> arg1 >> arg2;
	cout << ShowVecInline(shell.queryExpression(cmd, arg1, arg2)); }
	putLine();
	{subTitle("%time 2017/07/02 2018/07/02"); putLine();
	string cmd, arg1, arg2;
	istringstream ss("%time 2017/07/02 2018/07/02");
	ss >> cmd >> arg1 >> arg2;
	cout << ShowVecInline(shell.queryExpression(cmd, arg1, arg2)); }
	putLine();
	{subTitle("%filter e B"); putLine();
	string cmd, arg1, arg2;
	istringstream ss("%filter e B");
	ss >> cmd >> arg1 >> arg2;
	cout << ShowVecInline(shell.queryExpression(cmd, arg1, arg2)); }
	putLine();
	{subTitle("%union e B"); putLine();
	string cmd, arg1, arg2;
	istringstream ss("%union e B");
	ss >> cmd >> arg1 >> arg2;
	cout << ShowVecInline(shell.queryExpression(cmd, arg1, arg2)); }
}