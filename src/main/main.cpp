#include <iomanip>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

#include "src/checker/checker.h"
#include "src/parser/parser.h"
#include "src/solver/solver.h"

int main(int argc, char** argv) {
  // Parse command-line arguments
  if (argc < 2) {
    std::cout << "Usage: " << argv[0]
      << " [input_file] <alogrithm> <num_threads>\n";
    return 1;
  }
  std::string input_filename = argv[1]; 
  std::string algorithm = "SeqGreedy";
  int num_threads = 1;
  if (argc > 2) {
    algorithm = argv[2];
  }
  if (argc > 3) {
    num_threads = std::atoi(argv[3]);
  }

  // Database
  int num_vertices = 0;
  std::vector<std::pair<int, int>> edges;

  // Read database from file
  mis::Parser parser;
  if (!parser.Read(input_filename, num_vertices, edges)) {
    std::cerr << "[Error] Fail to read "
        << std::quoted(input_filename) << ".\n";
    return 2;
  }

  // Run solver
  mis::Solver solver;
  std::vector<int> mis = solver.Run(
      num_vertices, edges, algorithm, num_threads);

  // Check MIS
  mis::Checker checker;
  if (!checker.Run(num_vertices, edges, mis)) {
    std::cout << "[Error] MIS check: Fail...\n";
  } else {
    std::cout << "[Info] MIS check: Pass!!!\n";
  }

  return 0;
}  // end main

