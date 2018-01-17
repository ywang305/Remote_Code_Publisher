#pragma once
/////////////////////////////////////////////////////////////////////
// DepAnal.h - Type mathching for File dependency analysis         //
//                                                                 //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
A graph is a collection of vertices, connected by edges.  Each
vertex holds some kind of information held in an instance of
a type V and an integer id, which should be unique with a given
graph.  Each edge connects a parent vertex with a child
vertex and it too holds information in an instance of a type E.

This package provides two classes that support creation of
instances of a graph: 
       Vertex and Graph.  

The graph holds an aggregated collection of vertices.  Each
vertex may hold one or more edges.  The edge is presented as 
pair of two vertex ids .

StrongComponent class uses the Graph class, and  implements Tarjan 
algoraithm to find Strong Connection Components. Tarjan's algorithm 
is an algorithm in graph theory for finding the strongly connected 
components of a graph. It runs in linear time, matching the time 
bound for alternative methods including Kosaraju's algorithm and the 
path-based strong component algorithm.

Note:
It is important that vertex ids be unique.  If you set any of
them with the second argument of the Vertex constructor or with
the id() function it is up to you to ensure uniqueness.  If you
don't explicitly set any ids then the constructor insures that
vertices get sequential ids in the order constructed.
*
* Build Process:
* --------------
* Required Files: 
*
*  Maintenance History:
* --------------------
*  ver 1.0 : 04 Mar 2017
*	- first release:
*/
#include <vector>
#include <unordered_map>
#include <string>
#include <iostream>
#include <sstream>
#include <stack>
#include <algorithm>
using namespace std;

//////////////////////////////////////////////////////////////////////////////////////
/*
*  - Vertex member id must be unique because id will be used as a index in class Graph's
*  - adjacent  list.
*/
class Vertex
{
public:
	Vertex() = default;
	Vertex(string v);
	size_t GetId();
	string GetValue();
	void AddNeighborIndex(int index);
	vector<int>& GetNeighborIndex();
	bool& isVisted();
	bool& isOnStack();
	size_t& Accesslowlink();
private:
	string _value;
	size_t _id;
	size_t _lowlink;
	static size_t count;
	vector<int> _neighbors;
	bool _visited;
	bool _onStack;
};
size_t Vertex::count = 0;

//----< custome constructor for vertex, id is potentially used for adjacent list in Graph
//        value for keep practical data,  lowlink, visted, onStack are used for SSC algorithm  >-----

inline Vertex::Vertex(string v) :_value(v), _id(count++),
	_lowlink(_id), _visited(false), _onStack(false) {}

//----< get id >-----

inline size_t Vertex::GetId()
{ return this->_id; }

//----< get value >-----
inline string Vertex::GetValue()
{ return this->_value; }

//----< build an edge, which add tail-ended vertex >-----

inline void Vertex::AddNeighborIndex(int index)
{ _neighbors.push_back(index); }

//----< get all edges >-----

inline vector<int>& Vertex::GetNeighborIndex()
{ return this->_neighbors; }

//----< mark the node is visted, for Tarjan algorithm >-----

bool& Vertex::isVisted() 
{ return this->_visited; }

//----< mark the node is on stack, for Tarjan algorithm >-----

inline bool& Vertex::isOnStack()
{ return this->_onStack; }

//----< get the lowlink, for Tarjan algorithm >-----

inline size_t& Vertex::Accesslowlink()
{ return this->_lowlink; }



//////////////////////////////////////////////////////////////////////////////////////
/*
*  - Graph holds a adjVex as vertex adjacent list.
*  - It supports add vertex and add edge.
*/
class Graph
{
public:
	void AddVertex(Vertex vs);
	void AddEdge(Vertex vs, Vertex ve);
	vector<Vertex>& GetAdjVex();
	Vertex& GetAdjVexByIndex(int index);
private:
	vector<Vertex> adjVex;
};

//----< customed constructor that add a new Vertex into adjlist >-----

inline void Graph::AddVertex(Vertex vs)
{
	adjVex.push_back(vs);
}

//----< add edge, note that id is used for index the correct place >-----

inline void Graph::AddEdge(Vertex vs, Vertex ve)
{
	int v_index = vs.GetId();
	int w_index = ve.GetId();
	adjVex.at(v_index).AddNeighborIndex(w_index);
}

//----< get all nodes >-----

inline vector<Vertex>& Graph::GetAdjVex()
{ return adjVex; }

//----< get node by indexing adjVex  >-----

inline Vertex& Graph::GetAdjVexByIndex(int index)
{ return adjVex.at(index); }



//////////////////////////////////////////////////////////////////////////////////////
/*
*  - mainly implement the algorithm Tarjan.
*  - Tarjan is operated on Graph class
*/
class StrongComponent
{
public:
	void Tarjan(Graph g);
private:
	void StrongConnect(Vertex& v);
	stringstream outStrongComponent(vector<Vertex> scc);

	stack<Vertex> _stack;	
	Graph _g;
};

//----< Get nodes from Graph parameter, based on which run Tarjan alg >-----

inline void StrongComponent::Tarjan(Graph g)
{
	// index and lowlink already initialized in class Graph
	_g = g;
	for (auto& v : _g.GetAdjVex())
	{
		if (v.isVisted() == false)
			StrongConnect(v);
	}
}

//----< core part of Tarjan alg, reference to https://en.wikipedia.org/wiki/Tarjan >-----

inline void StrongComponent::StrongConnect(Vertex& v)
{
	_g.GetAdjVexByIndex(v.GetId()).isVisted() = true;
	_stack.push(v);
	_g.GetAdjVexByIndex(v.GetId()).isOnStack() = true;

	for (int windex : v.GetNeighborIndex())
	{
		Vertex& w = _g.GetAdjVexByIndex(windex);
		if (w.isVisted() == false)
		{
			StrongConnect(w);
			auto wlowlink = w.Accesslowlink();
			_g.GetAdjVexByIndex(v.GetId()).Accesslowlink() = min(v.Accesslowlink(), wlowlink);
		}
		else if (w.isOnStack() == true)
		{
			_g.GetAdjVexByIndex(v.GetId()).Accesslowlink() = min(w.Accesslowlink(), v.Accesslowlink());
		}
	}
	if (_g.GetAdjVexByIndex(v.GetId()).Accesslowlink() == v.GetId())
	{
		vector<Vertex> _SCC;
		Vertex w;
		do {
			w = _stack.top();
			_g.GetAdjVexByIndex(w.GetId()).isOnStack() = false;
			_stack.pop();
			_SCC.push_back(w);
		} while (w.GetId() != v.GetId());
		cout << "   find a Strong Component : " << outStrongComponent(_SCC).str() << endl;
	}
}

//----< helper method for demo >-----

inline stringstream StrongComponent::outStrongComponent(vector<Vertex> scc)
{
	stringstream ss;
	for (auto& v : scc)
	{
		ss << "  " << v.GetValue();
	}
	return ss;
}