#ifndef GP_H
#define GP_H

#include <iostream>
#include <vector>
#include <map>
#include "Child.h"
using namespace ChildTest;

typedef std::string str;

class GrandParent {
public:
	GrandParent() {};
	~GrandParent() {};
	int GPmethod2();
	int GPmethod3();
private:
	Child child;
};

int GrandParent::GPmethod2() { 
	child.method2();
	return -1; 
}


static void globalFunction2() {
	std::cout << "\n Global Function";
}

#endif

