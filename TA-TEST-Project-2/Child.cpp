#include "Child.h"
#include "Grandparent.h"
#include <iostream>

using namespace ChildTest;

int Child::method1() {
	std::cout << "From Method 1";
	return 0;
}
int Child::method3() {
	std::cout << "From Method 3";
	return 0;
}
int Child::method4() {
	std::cout << "From Method 4";
	return 0;
}