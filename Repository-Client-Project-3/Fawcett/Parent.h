#ifndef SCOPESTACK_H
#define SCOPESTACK_H
#pragma once


#include "Child2.h"
#include "Child.h"
#include "Invalid.h"
#include <list>


namespace parent1
{
	namespace parent2
	{
		template<typename T>
		class Parent {
			using namespace ChildTest;
		public:
			void push(const T& item);
			T pop();
		private:
			data d;
			std::list<T> stack;
			Child child;
		};


		template<class T>
		void Parent<T>::push(const T& item)
		{
			stack.push_back(item);
		}

		template<class T>
		T Parent<T>::pop()
		{
			T item = stack.back();
			stack.pop_back();
			globalData++;
			return item;
		}
	}

}
#endif
