#pragma once
/////////////////////////////////////////////////////////////////////
// Element.h - Defines Element template    
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package implements a template element class providing in-memory
* data structure, with key/value pair. The element consists two parts:
* One is metadata that is record information about the read data; the other
* part is the stored data, which is generic template design, that hold 
* various custom data type.
*	----------------------------
*
* Build Process:
* --------------
* Required Files: CppProperties.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 07 Feb 2017
*	- first release
*  ver 1.1 : 06 Mar 2017
*   - complement function prologues
*/
#include <vector>
#include <unordered_map>
#include <string>
#include <ctime>
#include <sstream>
#include <iomanip>
#include <iostream>
#include <fstream>
#include "../CppProperties/CppProperties.h"

/////////////////////////////////////////////////////////////////////
// Element class uses Property package to provide property.
//
template<typename Data>
class Element
{
private:
	std::vector<std::string> childrenKeys;
	std::string getTime();
public:
	Element();
	Property<std::string> key;
	Property<std::string> name;
	Property<std::string> category;
	Property<std::string> timeDate;
	Property<std::string> description;
	Property<std::string> achildKey;
	Property<Data> data;
	std::vector<std::string> & getChildrenKeys();
	std::string ShowElem();
};

//----< constructor uses lambda to redefine setter for children vector & timeDate >-----

template<typename Data>
Element<Data>::Element()
{
	timeDate.setter([&](const std::string& s) { return getTime(); }
	);
	achildKey.setter(
		[&](const std::string& s) { childrenKeys.push_back(s); return s; }
	);
	timeDate = ""; // invoke setter to set current time
}

//----< get back all dependent keys >-----

template<typename Data>
std::vector<std::string> &Element<Data>::getChildrenKeys()
{
	return childrenKeys;
}

//----< current time >-----

template<typename Data>
std::string Element<Data>::getTime()
{
	std::time_t t = std::time(nullptr);
	std::tm tm;  localtime_s(&tm, &t);
	std::stringstream ss; ss << std::put_time(&tm, "%Y/%d/%m  %H:%M:%S");
	return ss.str();
}

//----< demonstrate content of an element instance >-----

template<typename Data>
std::string Element<Data>::ShowElem()
{
	std::ostringstream out;
	out.setf(std::ios::adjustfield, std::ios::left);
	out << "\n    " << std::setw(8) << "key" << " : " << key.getValue();
	out << "\n    " << std::setw(8) << "name" << " : " << name.getValue();
	out << "\n    " << std::setw(8) << "category" << " : " << category.getValue();
	out << "\n    " << std::setw(8) << "timeDate" << " : " << timeDate.getValue();
	out << "\n    " << std::setw(8) << "description" << " : " << description.getValue();
	for (auto kid : childrenKeys)
	{
		out << "\n    " << std::setw(8) << "child-key" << " : " << kid;
	}
	out << "\n";
	return out.str();
}
