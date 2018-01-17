#include "Grandparent.h"
#include "Parent.h"
#include "Child2.h"

int GrandParent::GPmethod3() {
	std::cout << "GP Method definition";
	return 0;
}

#ifdef TEST_TEST
int main() {
	str s = " Hello";
	parent1::parent2::Parent<str> p;
	p.push("Hi");
	p.pop();
	ChildTest2::Child child;
}
#endif
