#pragma once
/////////////////////////////////////////////////////////////////////
// NoSqlDb.h - key/value pair in-memory database                   //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* This package provide in-memory no sql database model, for caching a bunch of
* elements. The cache mode is built up unsorted_map using key/value pair. 
* It restores new coming element into its map cache, 
* It supports methods that called by Shell, including: query keys, values and save
* that for Shell.
* It also compose persistor object for persisting cache into XML.
*
* Build Process:
* --------------
* Required Files: Element.h, CppProperties.h, PersistHelp.h
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 07 Feb 2017
*	- first release
*  ver 1.1 : 06 Mar 2017
*   - remove persitor as a class memeber data, instead construct that in member function as needed
*   - complement class function prologues
*   - warning fixed, getValue return a pointer to Element or NULL, rather than reference.
*  ver 1.2 : 26 Mar 2017
*   - bug fixed for "updateCachefromDB" that handle the exceptional case when xml is non existed
   ver 1.3 : 02 May 2017
    - updateCacheByDB fixed bug, when reading from file to memory, only unmatched key is loaded in-mem db
		to ensure the same key-value in-memory cache is not polluted by files old item
*/
#include <time.h>
#include "Element.h"
#include "../CppProperties/CppProperties.h"
#include "../Persist/PersistHelp.h"

/////////////////////////////////////////////////////////////////////
// NoSqlDb class is a key/value pair in-memory database
// - stores and retrieves elements
// -  add query capability
//   That should probably be another class that composes/aggregates NoSqlDb object
//
template<typename Data>
class NoSqlDb {
public:
	using Key = std::string;
	using Keys = std::vector<Key>;
	NoSqlDb(string file): fileSpec_("..\\" + file +"_nosqldb.xml") {  }
	~NoSqlDb(){}
	bool saveToCache(Key key, Element<Data> elem);
	void eraseItemByKey(Key key);
	Keys getKeys();
	Element<Data>* getValue(Key key);
	size_t getCount();
	std::string persistToXml();
	void updateCacheByDB();
private:
	using Item = std::pair<Key, Element<Data>>;
	std::unordered_map<Key, Element<Data>> cache;
	std::string fileSpec_;
};

//----< key collection of in-memory hash map >-----

template<typename Data>
typename NoSqlDb<Data>::Keys NoSqlDb<Data>::getKeys()
{
	Keys keys;
	for (Item item : cache)
	{
		keys.push_back(item.first);
	}
	return keys;
}

//----< add a key-value element into in-memory database >-----

template<typename Data>
bool NoSqlDb<Data>::saveToCache(Key key, Element<Data> elem)
{
	cache[key] = elem;
	return true;
}

//----< delete a key-value element of in-memory database >-----

template<typename Data>
void NoSqlDb<Data>::eraseItemByKey(Key key)
{
	cache.erase(key);
}

//----< low level query that return an element isntance from hash map's value >-----

template<typename Data>
Element<Data>* NoSqlDb<Data>::getValue(Key key)
{
	if (cache.find(key) != cache.end())
		return &cache[key];
	return nullptr;
}

//----< number of elements in-memory database >-----

template<typename Data>
size_t NoSqlDb<Data>::getCount()
{
	return cache.size();
}

//----< persist in-memory database into xml file >-----

template<typename Data>
std::string NoSqlDb<Data>::persistToXml()
{
	Persist<Data> persistor;
	std::string xml;
	xml = persistor.toXml(cache);
	persistor.writeBack(xml, fileSpec_);
	return xml;
}

//----< restore database from xml to memory >-----

template<typename Data>
void NoSqlDb<Data>::updateCacheByDB()
{
	Persist<Data> persistor;
	std::string xml = persistor.readBack(fileSpec_);
	if (xml == "error on open xml file\n")
		return;
	std::vector<Element<std::string>> vec = persistor.fromXml(xml);
	for (auto &elem : vec)
	{
		if( cache.end() == cache.find(elem.key))
			cache[elem.key] = elem;
	}
}

