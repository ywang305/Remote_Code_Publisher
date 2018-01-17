/////////////////////////////////////////////////////////////////////
// PubList.cpp - to test PubList.h,                                //
// behaivor    - PubList.h is dedicated to build list of outline   //
//               for project 3 web demo                            //
// ver 1.0                                                         //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////

#include "PubList.h"

//  --- < Test Stub for Htm list > ---

#ifdef TEST_PUBLIST
int main(int argc, char* argv[])
{
	ASTNode* root = ASTref_.root();
	PGLList pgl(root);
	for (auto p : pgl.queryPackages())
	{
		cout << p << endl;
		for (auto g : pgl.queryGlobalsByPackage(p))
		{
			cout << "   " << g << endl;
			for (auto lo : pgl.queryLocalsByGlobal(g))
			{
				cout << "        " << lo << endl;
			}
		}
	}
}
#endif