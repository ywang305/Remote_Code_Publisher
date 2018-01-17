/////////////////////////////////////////////////////////////////////
// PersistHelp.cpp - demo one way to persist objects to XML file   //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////

#ifdef TEST_PERSISTHELP
/*
 * - You can define and undefine using 
 *   project > Properties > C/C++ > Preprocessor > Processor Definitions
 * - or using command line
 *   cl /DTEST_PERSISTHELP persist.cpp [plus all the other cpp files needed]
 */

#include "PersistHelp.h"
#include <iostream>
#include "../Utilities/Utilities.h"
using namespace std;

int main()
{
  Title("Demonstrate Persisting to and from XML");
  putLine();


  Element<std::string> e1, e2;
  e1.key = "n0001";
  e1.name = "DataModel";
  e1.category = "NoSqlDBModel";
  e1.description = "keep in-memory database element";
  e1.data = "DataModel.h";
  e1.achildKey = "n0002";
  e1.achildKey = "n0004";
  e2.key = "n0002";
  e2.name = "Persist";
  e2.category = "PersistHelp";
  e2.description = "persist db from cache to xml";
  e2.data = "PersistHelp.h";
  e2.achildKey = "n0003";
  e2.achildKey = "n0004";
  e2.achildKey = "n0007";

  std::unordered_map<std::string, Element<string>> cache;
  cache[e1.key] = e1;
  cache[e2.key] = e2;

  Persist<string> persistor;
  std::string xml = persistor.toXml(cache);

  // persist xml involving  e1 and e2 
  std::string fileSpec = "..\\nosqldb.xml";
  persistor.writeBack(xml, fileSpec);

  xml = persistor.readBack(fileSpec);
  cout << xml << endl;

  subTitle("show a readBack element");
  std::vector<Element<std::string>> vec = persistor.fromXml(xml);
  for (auto e : vec)
  {
	  cout << e.ShowElem();
  }
}

#endif
