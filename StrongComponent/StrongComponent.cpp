///////////////////////////////////////////////////////////////
// StrongComponent.cpp - Test stub for SCC                   //
// ver 1.0                                                   //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017 //
///////////////////////////////////////////////////////////////

#ifndef TEST_SSC
#include "StrongComponent.h"
#include <vector>

class HelpTestor
{
public:
	void IniVex(int i)
	{
		stringstream ss;
		ss << "V" << i;
		vv.push_back(Vertex(ss.str()));
	}

	Vertex operator[](int index) { return vv.at(index); }

	vector<Vertex> getVec() { return vv; }
private:
	vector<Vertex> vv;
};

//----< test stub >--------------------------------------------

int main()
{
	HelpTestor t; Graph g;
	for (int i = 0; i < 8; ++i)
		t.IniVex(i);

	for (auto v : t.getVec())
		g.AddVertex(v);

	g.AddEdge(t[0], t[1]);	g.AddEdge(t[1], t[2]);
	g.AddEdge(t[2], t[0]);	g.AddEdge(t[3], t[1]);
	g.AddEdge(t[3], t[2]);	g.AddEdge(t[3], t[4]);
	g.AddEdge(t[4], t[3]);	g.AddEdge(t[4], t[5]);
	g.AddEdge(t[5], t[2]);	g.AddEdge(t[5], t[6]);
	g.AddEdge(t[6], t[5]);	g.AddEdge(t[7], t[4]);
	g.AddEdge(t[7], t[6]);	g.AddEdge(t[7], t[7]);

	StrongComponent sc;
	sc.Tarjan(g);
}
#endif // TEST_SSC
