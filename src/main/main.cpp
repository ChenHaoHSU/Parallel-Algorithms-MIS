#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "src/parser/parser.h"

int main(int argc, char** argv) {

  std::cout << "[Info] Hello, Ruby!\n";

  if (argc < 3) {
    std::cout << "Usage: " << argv[0] << " [input_file] [output_file]\n";
    return 1;
  }
  std::string input_filename = argv[1]; 
  std::string output_filename = argv[2];

  // Database
  int num_vertices = 0;
  std::vector<std::pair<int, int>> edges;

  // Read database from file
  ruby::Parser parser;
  if (!parser.Read(input_filename, num_vertices, edges)) {
    std::cerr << "[Error] Fail to read "
        << std::quoted(input_filename) << ".\n";
    return 2;
  }

  return 0;
}  // end main

