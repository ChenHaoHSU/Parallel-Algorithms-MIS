#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "src/checker/checker.h"
#include "src/parser/parser.h"
#include "src/solver/solver.h"

int main(int argc, char** argv) {

  std::cout << "[Info] Hello, Luby!\n";

  if (argc < 3) {
    std::cout << "Usage: " << argv[0]
      << " [input_file] [output_file] <alogrithm>\n";
    return 1;
  }
  std::string input_filename = argv[1]; 
  std::string output_filename = argv[2];
  std::string algorithm = "SeqGreedy";
  if (argc > 3) {
    algorithm = argv[3]; 
  }

  // Database
  int num_vertices = 0;
  std::vector<std::pair<int, int>> edges;

  // Read database from file
  luby::Parser parser;
  if (!parser.Read(input_filename, num_vertices, edges)) {
    std::cerr << "[Error] Fail to read "
        << std::quoted(input_filename) << ".\n";
    return 2;
  }

  // Run solver
  luby::Solver solver;
  std::vector<int> mis = solver.Run(num_vertices, edges, algorithm);

  // Check MIS
  luby::Checker checker;
  if (!checker.Run(num_vertices, edges, mis)) {
    std::cout << "[Error] MIS check: Fail...\n";
  } else {
    std::cout << "[Info] MIS check: Pass!!!\n";
  }

  return 0;
}  // end main

