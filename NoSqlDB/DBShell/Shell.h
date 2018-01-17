#pragma once
/////////////////////////////////////////////////////////////////////
// Shell.h - Defines query edit add delete save... for metadata, data//
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package support addition and deletion of key/value pairs
* support editing of metadata including name, category, description, 
* children, and also editing of data.
* support replace the instance of existing value in-memory db
* on command to persist database to XML file
* Query include keys and value query with multiple pattern, e.g. container, union,...
*	----------------------------
*
* Build Process:
* --------------
* Required Files: DataModel.h, XmlDocument package
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 07 Feb 2017
*	- first release
*  ver 1.1 : 05 Mar 2017
*   - bug fix: add the member addDepend() that avoid duplicated childkey being added.
*  ver 1.3 : 06 Mar 2017
*   - complement class function prologues
*  ver 1.4 : 26 Mar 2017
*   - add methods: Constructor loads xml to memory. Destructor persist memory to xml. 
*  ver 1.5 : 27 Mar 2017
*   - add a member function: queryKeyByData() for CodePublisher project!
*  ver 1.6 : 2 Apr 2017
*   - indicate xml file name in constructor
*/
#include <iostream>
#include <vector>
#include <string>
#include <functional>
#include "../NoSqlDbModel/DataModel.h"
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////
/*
*  - The class Shell as the name is a interface for users to manipuate database.
*  - It compose a NoSqlDB object in its lifecyle to react with instructions.
*/

template<typename Data>
class Shell
{public:
	vector<string> queryExpression(string cmd, string arg1, string arg2);

	void addItem(Element<Data> item);
	void delItem(string key);
	void addDepend(string key, string addKey);
	void delDepend(string key, string delKey);
	void renewName(string key, string newContent);
	void renewCateg(string key, string newContent);
	void renewDescp(string key, string newContent);
	void renewValue(string key, Element<Data> newitem);

	bool queryKey(string specKey);
	vector<string> queryAllKeys();
	string queryKeyByData(string data);
	Element<Data> queryValue(string specKey);
	vector<string> queryChildren(string specKey);
	vector<string> queryKeysContainSubStr(string substr);
	vector<string> queryKeysContainCategory(string categ);
	vector<string> queryKeysContainItemname(string name);
	vector<string> queryKeysContainData(string name);
	vector<string> queryKeysContainTime(string time1 = getMyTime(), string time2 = getMyTime());

	vector<string> queryKeysFilterCondition( string arg1, string arg2);
	vector<string> queryKeysUnionCondition(string arg1, string arg2);
	
	NoSqlDb<Data>& getDB() { return *db; }
	static string getMyTime()
	{
		std::time_t t = std::time(nullptr);
		std::tm tm;  localtime_s(&tm, &t);
		std::stringstream ss; ss << std::put_time(&tm, "%Y/%d/%m  %H:%M:%S");
		return ss.str();
	}
	Shell(string file);
	~Shell();
private:
	int counterQuery = 0;
	const int MAXQUERYS = 90;
	NoSqlDb<Data> *db;

	void autoSave();
};


// -- < Contor that ensures the load from xml to in-memory database > --
template<typename Data>
Shell<Data>::Shell(string file): db(new NoSqlDb<Data>(file))
{
	db->updateCacheByDB();
}

// -- < Destructor that ensures the persisting > --
template<typename Data>
Shell<Data>::~Shell()
{
	db->persistToXml();
	delete db;
}

//----< pattern query >-----

template<typename Data>
vector<string> Shell<Data>::queryExpression(const string cmd, const string arg1, const string arg2)
{
	if (cmd == "%children")  return queryChildren(arg1);
	else if(cmd == "%name") return queryKeysContainItemname(arg1);
	else if (cmd == "%category")	return queryKeysContainCategory(arg1);
	else if(cmd== "%data") return queryKeysContainData(arg1);
	else if(cmd == "%time") return queryKeysContainTime(arg1, arg2);
	else if (cmd == "%filter") return queryKeysFilterCondition(arg1, arg2);
	else if( cmd == "%union") return queryKeysUnionCondition(arg1, arg2);
	else return vector<string>();
}

//----< auto save once receving MAXQUERYS times writing operation to db cache >-----

template<typename Data>
void Shell<Data>::autoSave()
{
	counterQuery++;
	if (counterQuery% MAXQUERYS == 0)
	{
		db->persistToXml();
		//cout << "\n    -(  auto saving happens after " << MAXQUERYS << " times writing to cache  )-\n";
	}
}

//----< add an element to in-memory hash map >-----

template<typename Data>
void Shell<Data>::addItem(Element<Data> item)
{
	db->saveToCache(item.key, item);
	autoSave();
}

//----< delete an element to in-memory hash map >-----

template<typename Data>
void Shell<Data>::delItem(string key)
{
	db->eraseItemByKey(key);
	autoSave();
}

//----< add a child key to elment referring to specific key in-memory hash map >-----

template<typename Data>
void Shell<Data>::addDepend(string key, string addkey)
{
	if (queryKey(key))
	{
		Element<Data> *pele = db->getValue(key);
		if (!pele) return;
		vector<string> childkeys = pele->getChildrenKeys();

		// sieve the duplicated child keys to be added, only add new to element's children.
		if (end(childkeys) ==
			std::find(begin(childkeys), end(childkeys), addkey))
		{
			pele->getChildrenKeys().push_back(addkey);
		}
	}
	autoSave();
}

//----< delete child key for an element referring to specific key in-memory hash map >-----

template<typename Data>
void Shell<Data>::delDepend(string key, string delkey)
{
	if (queryKey(key))
	{
		Element<Data> *pele = db->getValue(key);
		if (!pele) return;
		vector<string> &children = pele->getChildrenKeys();
		auto iterator = std::find(children.begin(), children.end(), delkey);
		if (iterator != children.end())
			children.erase(iterator);
	}
	autoSave();
}

//----< modify name property of an element referring to specific key in-memory hash map >-----

template<typename Data>
void Shell<Data>::renewName(string key, string newContent)
{
	if (queryKey(key))
	{
		Element<Data> *pele = db->getValue(key);
		if (!pele) return;
		pele->name = newContent;
	}
	autoSave();
}

//----< modify category property of an element referring to specific key in-memory hash map >-----

template<typename Data>
void Shell<Data>::renewCateg(string key, string newContent)
{
	if (queryKey(key))
	{
		Element<Data> *pele = db->getValue(key);
		if (!pele) return;
		pele->category = newContent;
	}
	autoSave();
}

//----< modify description property of an element referring to specific key in-memory hash map >-----

template<typename Data>
void Shell<Data>::renewDescp(string key, string newContent)
{
	if (queryKey(key))
	{
		Element<Data> *pele = db->getValue(key);
		if (!pele) return;
		pele->description = newContent;
	}
	autoSave();
}

//----< modify value part of hash map's element referring to specific key in-memory hash map >-----

template<typename Data>
void Shell<Data>::renewValue(string key, Element<Data> newitem)
{
	if (queryKey(key))
	{
		newitem.key = key;
		db->eraseItemByKey(key);
		db->saveToCache(key, newitem);
	}
	autoSave();
}

// -- < specific key query with data >--
template<typename Data>
string Shell<Data>::queryKeyByData(string data)
{
	string query = "";
	for (auto key : db->getKeys())
	{
		Element<std::string> *pele = db->getValue(key);
		if (pele == nullptr) break;
		if ( data == pele->data.getValue())
		{
			query = key;
			break;
		}
	}
	return query;
}

// -- < all keys > -- 
template<typename Data>
vector<string> Shell<Data>::queryAllKeys()
{
	return db->getKeys();
}

//----< query the key of an element >-----

template<typename Data>
bool Shell<Data>::queryKey(string specKey)
{
	vector<string> tmp = db->getKeys();
	for (auto b = begin(tmp); b != end(tmp); b++)
	{
		if (*b == specKey)
		{
			return true;
		}
	}
	return false;
}

//----< query the value of an element via specific key >-----

template<typename Data>
Element<Data> Shell<Data>::queryValue(string specKey)
{
	if (queryKey(specKey))
	{
		Element<Data> *pele = db->getValue(specKey);
		return *pele;
	}
	else
		return Element<Data>();
	
}

//----< query all dependent keys of an element via specific key >-----

template<typename Data>
vector<string> Shell<Data>::queryChildren(string specKey)
{
	vector<string> result;
	vector<string> tmp = db->getKeys();
	auto iterator = std::find(tmp.begin(), tmp.end(), specKey);
	if (iterator != tmp.end())
	{
		Element<Data> *pele = db->getValue(*iterator);
		if (pele)
		{
			for (auto key : pele->getChildrenKeys())
				result.push_back(key);
		}
	}	
	return result;
}

//----< pattern query as meet the substr of primary key  >-----

template<typename Data>
vector<string> Shell<Data>::queryKeysContainSubStr(string substri)
{
	vector<string> allKeys = db->getKeys();
	vector<string> results;
	for (auto key : allKeys)
	{
		if (key.find(substri) != std::string::npos)
			results.push_back(key);
	}
	return results;
}

//----< pattern query that contains designated category  >-----

template<typename Data>
vector<string> Shell<Data>::queryKeysContainCategory(string categ)
{
	vector<string> results;
	const vector<string> allKeys = db->getKeys();
	for (auto k : allKeys)
	{
		Element<std::string> *pele = db->getValue(k);
		if (pele == nullptr) continue;
		string cat = pele->category.getValue();
		if (cat.find(categ) != std::string::npos)
			results.push_back(k);
	}
	return results;
}

//----< pattern query that contains designated name property >-----

template<typename Data>
vector<string> Shell<Data>::queryKeysContainItemname(string name)
{
	vector<string> results;
	vector<string> allKeys = db->getKeys();
	for (auto k : allKeys)
	{
		Element<std::string> *pele = db->getValue(k);
		std::string str = pele->name;
		if (str.find(name) != std::string::npos)
			results.push_back(k);
	}
	return results;
}

//----< pattern query that contains designated data property >-----

template<typename Data>
vector<string> Shell<Data>::queryKeysContainData(string name)
{
	vector<string> results;
	const vector<string> allKeys = db->getKeys();
	for (auto k : allKeys)
	{
		Element<std::string> *pele = db->getValue(k);
		std::string str = pele->data;
		if (str.find(name) != std::string::npos)
			results.push_back(k);
	}
	return results;
}

//----< pattern query that  filter query that the first query result is filtered by the second query. >-----

template<typename Data>
vector<string> Shell<Data>::queryKeysContainTime(string time1 = getMyTime(), string time2 = getMyTime())
{
	vector<string> results;
	const vector<string> allKeys = db->getKeys();
	for (auto k : allKeys)
	{
		Element<std::string> *pele = db->getValue(k);
		std::string str = pele->timeDate;
		if (str.compare(time1)>=0 && str.compare(time2)<=0)
			results.push_back(k);
	}
	return results;
}

//----< pattern query that forming a union query keys from two queries with name and category metadata >-----

template<typename Data>
vector<string> Shell<Data>::queryKeysFilterCondition(string arg1,  string arg2)
{// name , category filted
	vector<string> results;
	vector<string> keyset1 = queryKeysContainItemname(arg1);
	vector<string> keyset2 = queryKeysContainCategory(arg2);
	for( auto k1 : keyset1)
		for (auto k2 : keyset2)
		{
			if (k1 == k2)
			{
				results.push_back(k1);
				break;
			}
		}
	return results;

}

//----< pattern query that union of two args >-----

template<typename Data>
vector<string> Shell<Data>::queryKeysUnionCondition(string arg1, string arg2)
{	// name , category filted
	vector<string> keyset1 = queryKeysContainItemname(arg1);
	vector<string> keyset2 = queryKeysContainCategory(arg2);
	vector<string> results = keyset1;
	for (auto k2 : keyset2)
	{
		auto i = begin(keyset1);
		for (; i!=end( keyset1); i++)
		{
			if (*i == k2) break;
		}
		if (i == end(keyset1)) results.push_back(k2);
	}
	return results;
}
		