#pragma once
/////////////////////////////////////////////////////////////////////
// PersistHelp.h - demo one way to persist objects to XML file     //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* This package builds wrapper classes for strings, doubles, and Widgets.
* They provide the same functionality as the wrapped type, but also
* attach methods save() and restore() to persist the wrapped type.
*
* Package operations:
* -------------------
* This package contains:
* - IPersist interface - defines the methods needed to
*   serialize to XML and deserialize from XML
* - StringWrapper class that persists strings
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 07 Feb 2017
*	- first release
*  ver 1.1 : 06 Mar 2017
*   - fix bug, fromXml() return a vector of elements, rather one elment
*   - complement function prologues
*  ver 1.2 : 13 Mar 2017
*   - bug fixed for the readBack()
*   - add parameter for readBack() and writeBack() to sepcify the file path. 
*
* Required Files:
* ---------------
*   PersistHelp.h, PersistHelp.cpp,
*   Convert.h, Convert.cpp,
*   CppProperties.h, CppProperties.cpp,
*   XmlDocument.h, XmlDocument.cpp, XmlElement.h, XmlElement.cpp,
*  ---- required to build XmlDocument from file or string ----
*   XmlParser.h, XmlParser.cpp,
*   XmlElementParts.h, XmlElementParts.cpp,
*   ITokenizer.h, Tokenizer.h, Tokenizer.cpp,
*
* Build Instructions:
* -------------------
* - Uses XmlDocument, so build XmlDocument project as static library
*   after undefining its test stub
*/
#include <string>
#include <vector>
#include <fstream>
#include <sstream>
#include "../Serialize/Convert.h"
#include "../CppProperties/CppProperties.h"
#include "../XmlDocument/XmlDocument/XmlDocument.h"
#include "../XmlDocument/XmlElement/XmlElement.h"
#include "../NoSqlDbModel/Element.h"
#include "../StrHelper.h"

using namespace XmlProcessing;
using namespace std;

/////////////////////////////////////////////////////////////////////
// Persist Class read/write file, it use 
//
// Element Wrapper class for persist
template<typename Data>
class Persist
{
public:
	using Key = std::string;
	using SPtr = std::shared_ptr<AbstractXmlElement>;
	using Xml = std::string;
	Persist();

	Xml toXml(std::unordered_map<Key, Element<Data>> cache);
	std::vector<Element<std::string>> fromXml(const Xml &xml);
	bool writeBack(const Xml& xml, const std::string& fileSpec);
	Xml readBack(const std::string& fileSpec);
	
private:
	Element<std::string> elem_;
	XmlDocument doc_;
};

//----< Constructor for XmlDocumtent root >-----

template<typename Data>
Persist<Data>::Persist()
{
	SPtr pRoot = makeTaggedElement("db");
	doc_.docElement() = pRoot;
}

//----< serialize data element to xml string >-----

template<typename Data>
typename Persist<Data>::Xml Persist<Data>::toXml(unordered_map<Key, Element<Data>> cache)
{
	SPtr pRecElem, pNameElem, pTextElem;
	auto lambda = [&pRecElem, &pNameElem, &pTextElem](string tag, string text) {
		pNameElem = makeTaggedElement(tag);
		pTextElem = makeTextElement(text);
		pNameElem->addChild(pTextElem);
		pRecElem->addChild(pNameElem);
	};
	for (auto elem : cache)
	{
		pRecElem = makeTaggedElement("record");
		lambda("key", elem.second.key);
		lambda("name", elem.second.name);
		lambda("category", elem.second.category);
		lambda("description", elem.second.description);
		lambda("timestamp", elem.second.timeDate);
		std::string children = "";
		for (auto k : elem.second.getChildrenKeys())	children += k + " ";
		lambda("dependency", children);
		lambda("data", elem.second.data);
		doc_.docElement()->addChild(pRecElem);
	}
	return doc_.toString();
}

//----< deserialize xml strings to database element instance >-----

template<typename Data>
std::vector<Element<std::string>> Persist<Data>::fromXml(const typename Persist::Xml &xml)
{
	std::vector<Element<std::string>> restoredElements;
	XmlDocument doc(xml, XmlDocument::str);
	std::vector<Persist::SPtr> desc = doc.descendents("record").select();

	for (auto record : desc)
	{
		Element<std::string> e;
		auto lambda = [&record](int i)->std::string {
			return trim( record->children()[i]->children()[0]->value());
		};
		try {
			e.key = lambda(0);
			e.name = lambda(1);
			e.category = lambda(2);
			e.description = lambda(3);
			e.timeDate = lambda(4);
			e.data = lambda(6);
			if (record->children()[5]->children().size() != 0)
			{
				std::string dependency = record->children()[5]->children()[0]->value();
				std::istringstream iss(dependency);
				string word;
				while (iss >> word) { e.achildKey = word; }
			}
		}
		catch (std::exception& ex)
		{
			std::cout << "\n  something bad happend when reading xml file";
			std::cout << "\n  " << ex.what();
		}
		restoredElements.push_back(e);
	}
	return restoredElements;
}

//----< persist to xml file >-----

template<typename Data>
bool Persist<Data>::writeBack(typename const Persist::Xml& xml, const std::string& fileSpec)
{
	std::ofstream of(fileSpec);
	if (!of.good())
		return false;

	of << xml;
	of.close();
	return true;
}

//----< restore from xml file >-----

template<typename Data>
typename Persist<Data>::Xml Persist<Data>::readBack(const std::string& fileSpec)
{
	std::ifstream inf(fileSpec);
	if (!inf.good())  return "error on open xml file\n";
	Persist::Xml xml;
	while (inf.good())
	{
		/*if(getline(inf, temp))
			xml += temp+"\n";*/
		char ch = inf.get();
		if (!inf.good()) break;
		xml += ch;
	}
	inf.close();
	return xml;
}