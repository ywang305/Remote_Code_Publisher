///////////////////////////////////////////////////////////////
// DataModel.cpp - Test stub for DataModel.h      //
// ver 1.0                                                   //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2013 //
///////////////////////////////////////////////////////////////

// --<  test stub  >----
#ifdef TEST_DATAMODEL
#include <ctime>
#include "DataModel.h"
#include "../Utilities/Utilities.h"
class Widget
{
public:
	Widget(const std::string& str = "") : state(str) {}
	std::string& value() { return state; }
private:
	std::string state;
};

int main()
{
	Element<std::string> e1, e2;
	e1.key = "n0001";	e1.name = "DataModel";
	e1.category = "NoSqlDBModel";	e1.description = "keep in-memory database element";
	e1.data = "DataModel.h";	e1.achildKey = "n0002";	e1.achildKey = "n0004";
	Title("\n Demonstrate Element stored in in-memory DataModel");
	subTitle("\n  Creating and saving NoSqlDb elements with string data");
	std::cout<< e1.ShowElem();
	std::cout.setf(std::ios::adjustfield, std::ios::left);
	std::cout << "    " << std::setw(8) << "data" << " : " << e1.data.getValue()<<std::endl;
	e2.key = "n0002";
	e2.name = "Persist";
	e2.category = "PersistHelp";
	e2.description = "persist db from cache to xml";
	e2.data = "PersistHelp.h";
	e2.achildKey = "n0003";	e2.achildKey = "n0004";
	Element<Widget> e3;
	Widget widget("a Widget type object");
	e3.key = "n0003";
	e3.name = "Widget";
	e3.category = "Testor";
	e3.description = "to test custom type for in-mem data model";
	e3.data = widget;
	e3.achildKey = "n0001";	e3.achildKey = "n0002";	e3.achildKey = "n0005";
	subTitle("\n  Creating and saving NoSqlDb elements with Widget object");
	std::cout<< e3.ShowElem();
	std::cout << "    " << std::setw(8) << "data" << " : " << widget.value() << std::endl;
	NoSqlDb<std::string> dbstr("demo");
	dbstr.saveToCache(e1.key, e1);
	dbstr.saveToCache(e2.key, e2);
	NoSqlDb<Widget> dbwidget("demowidget");
	dbwidget.saveToCache(e3.key, e3);
	subTitle("\n  check element with string data, cached in NoSqlDb");
	std::cout << "\n    " << std::setw(8) << "cached items" << " : " << dbstr.getCount();
	std::cout << "\n    " << std::setw(8) << "query all keys" << " : " << dbstr.getKeys()[0] <<" "<<dbstr.getKeys()[1];
	//std::cout << "\n    " << std::setw(8) << "query element via key " << e1.key<< " : " << *dbstr.getValue(e1.key).ShowElem();
	dbstr.eraseItemByKey(e3.key);
	subTitle("\n  erase an element item, cached in NoSqlDb");
	std::cout << "\n    " << std::setw(8) << "cached items" << " : " << dbstr.getCount();
	std::cout << "\n    " << std::setw(8) << "query all keys" << " : " << dbstr.getKeys()[0];
	putLines(2);
	subTitle("\n  test persistToXml and updateCacheByDB");
	dbstr.persistToXml();
	dbstr.updateCacheByDB();
	//for (auto tster : dbstr.cache)	cout << tster.first << " , " << tster.second.name << endl;
}
#endif // TEST_STUB