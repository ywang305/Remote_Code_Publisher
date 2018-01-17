#pragma once
/////////////////////////////////////////////////////////////////////
// Utilities.h - format screen print                               //
//                                                                 //
// Prof Fawcett credit, CSE687 - Object Oriented Design, Spring 2017       //
/////////////////////////////////////////////////////////////////////
/*
* Package Operations:
* -------------------
* print fomratted Title, subTitle, printLine...
*	----------------------------
*  Maintenance History:
* --------------------
*  ver 1.0 : 07 Feb 2017
*	- first release
*/

#include <functional>
#include <iostream>

/////////////////////////////////////////////////////////////////////
// lambdas that provide mildly useful local processing

/////////////////////////////////////////////////////////////////////
// lambdas that provide mildly useful local processing

std::function<void()> putLine = []() { std::wcout << "\n"; };

std::function<void(size_t)> putLines = [](size_t n) {
  for (size_t i = 0; i < n; ++i)
    putLine();
};

std::function<void(const std::string&, char)> titleCore = [](const std::string& msg, char underline = '-') {
  std::cout << "\n  " << msg.c_str();
  std::wcout << "\n " << std::string(msg.size() + 2, underline).c_str();
};

std::function<void(const std::string&)> Title = [](const std::string& msg) { titleCore(msg, '='); };

std::function<void(const std::string&)> subTitle = [](const std::string& msg) { titleCore(msg, '-'); };
