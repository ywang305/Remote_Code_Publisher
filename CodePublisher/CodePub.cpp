/*////////////////////////////////////////////////////////////////////////////
// CodePub.cpp -  test stub                                                //
// ver 1.0                                                                 //
// ----------------------------------------------------------------------- //
// copyright  Yaodong Wang, 2017                                           //
// All rights granted provided that this notice is retained                //
// ----------------------------------------------------------------------- //
// Language:    Visual C++, Visual Studio 2015                             //
// Platform:    HP-Q5F5, Core i3, Windows 10                               //
// Build req: "CodePub.h"                                                  //
// Application: Project #3, CSE687 - Object Oriented Design, S2015         //
// Author:      Yaodong Wang , Syracuse University                         //
//              ywang305@syr.edu                                           //
///////////////////////////////////////////////////////////////////////////*/


#include <string>
#include "CodePub.h"

// --< Test Stub >--

#ifdef TEST_CODEPUB
using namespace CodePublisher;

int main(int argc, char* argv[])
{
	string dstDir = R"(..\CodePublisher_Test_Repository)";
	CodePub cp(argc, argv);
	auto pubset = cp.toHtms(dstDir);

	for (auto htm : pubset)
	{
		string browser{ "start " };
		browser += htm;
		std::system(browser.c_str());
	}

	return 0;
}
#endif // TEST_CODEPUB

