#ifndef CHILD_H
#define CHILD_H

#include <iostream>
#include <vector>
#include <map>


int globalData = 10;

namespace ChildTest {
	
	struct element {
		std::string name, type;
		int start_line, end_line;
		std::vector<element> children;
	};
	enum Numbers { one = 1, two, three, four };
	class Child {
	public:
		Child() {};
		~Child() {};
		void inlineMethod(int a) {
			int i;
			for (i = 0; i < 2; i++) {
				std::cout << a;
				if (i == 1)
				{
					std::cout << i;
				}
			}
			if (i == 1) {
				std::cout << a;
			}
			else
			{
			}
		}
		int method1();
		int method3();
		int method2()
		{
			std::cout << "Hello OOD";
			return 0;
		}
		int method4();
	};
	


	static void globalFunction() {
		std::map<int, int> map = { { 1,1 },{ 2,2 } };
		while (true) {
			{
				std::cout << "\n Global Function";
			}
			break;
		}
	}
}

static void globalFunction1() {
	std::map<int, int> map = { { 1,1 },{ 2,2 } };
	while (true) {
		{
			std::cout << "\n Global Function";
		}
		break;
	}
}
#endif

