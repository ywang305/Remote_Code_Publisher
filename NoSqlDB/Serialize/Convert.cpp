///////////////////////////////////////////////////////////////
// Convert.cpp - Test stub for Convert      //
// ver 1.0                                                   //
// Yaodong Wang, CSE687 - Object Oriented Design, Spring 2013 //
///////////////////////////////////////////////////////////////

#include <iostream>
#include <functional>
#include "Convert.h"
#include "..\Utilities\Utilities.h"

/////////////////////////////////////////////////////////////////////
// Widget class - shows user defined types can be converted
// - must have operator<< and operator>>

class Widget
{
public:
	Widget(const std::string &str = ""): state(str){}
	std::string &getValue() { return state; }
private:
	std::string state;
};

std::ostream& operator<<(std::ostream& out, Widget& widget)
{
	out << widget.getValue();
	return out;
}

std::istream& operator >> (std::istream& in, Widget& widget)
{
	std::string temp;
	while (in.good())  // extract all the words from widget's string state
	{
		in >> temp;
		widget.getValue() += temp + " ";
	}
	return in;
}


#ifdef DEBUG
int main()
{
	Title("Demonstrating Converting");
	putLine();

	subTitle("Conversion of numerical types");
	std::string intString = Convert<int>::toString(42);
	std::cout << "\n  conversion of integer: " << intString << std::endl;
	std::cout << "  retrieving integer:    " << Convert<int>::fromString(intString) << std::endl;

	subTitle("Demonstrating Conversion of instances of Widget classe");
	Widget widget("Joe Widget");
	std::string wdgString = Convert<Widget>::toString(widget);
	std::cout << "\n  conversion of Widget:  " << wdgString;
	Widget newWidget = Convert<Widget>::fromString(wdgString);
	std::cout << "\n  newWidget state = " << newWidget.getValue() << std::endl;

	system("pause");
}
#endif