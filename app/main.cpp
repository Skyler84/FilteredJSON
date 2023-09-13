#include "filteredjson/json.hpp"
#include "filteredjson/parser.hpp"
#include <iostream>
#include <string>

int main()
{
  std::string str;
  FilteredJSON::Parser parser;
  while (std::cin.peek() != EOF)
  {
    std::getline(std::cin, str);
    parser.parseContinue(str);
  }
  FilteredJSON::Value &root = parser.getValue();
  std::cout << "Root type: " << (int)root.getType() << std::endl;
  std::cout << "Root stringified: " << root.stringify(0) << std::endl;
  return 0;
}