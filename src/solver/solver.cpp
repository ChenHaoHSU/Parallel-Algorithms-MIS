#include <algorithm>
#include <iostream>
#include <utility>
#include <vector>

#include "src/solver/solver.h"

namespace ruby {

std::vector<int> Solver::Run(int num_vertices,
                             const std::vector<std::pair<int, int>>& edges) {
  std::cout << "[Info] Start solving...\n"; 
  
  // Build the adjacency list
  BuildAdj(num_vertices, edges);

  std::vector<int> mis = SequentialSolve(num_vertices, edges);
  
  std::cout << "[Info] |MIS| = " << mis.size() << "\n";

  return mis;
}  // End Solver::Run
  
void Solver::BuildAdj(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges) {
  adj_.clear();
  adj_.resize(num_vertices);
  for (auto &[v1, v2] : edges) {
    adj_[v1].emplace_back(v2);
    adj_[v2].emplace_back(v1);
  }

#ifdef RUBY_DEBUG
  for (unsigned i = 0; i < adj_.size(); ++i) {
    std::cout << "[" << i << "]";
    for (auto nei : adj_[i]) {
      std::cout << " " << nei;
    }
    std::cout << "\n";
  }
#endif  // RUBY_DEBUG

}  // End Solver::BuildAdj

std::vector<int> Solver::SequentialSolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges) {
  std::vector<int> mis;

  // Sequential method 
  std::vector<int> sorted_indices(num_vertices);
  std::generate(sorted_indices.begin(), sorted_indices.end(),
                [i = 0]() mutable { return i++; });
  std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
                   [&] (int lhs, int rhs) {
                     return adj_[lhs].size() < adj_[rhs].size();
                   });

  // Iterate through all vertices in the sorted order
  std::vector<bool> marked(num_vertices, false);
  for (int i : sorted_indices) {
    // Skip marked vertices
    if (marked[i])
      continue;

    // Add vertex i to MIS
    mis.emplace_back(i);
    
    // Mark vertext i and its neighbors
    marked[i] = true;
    for (int nei : adj_[i])
      marked[nei] = true;
  }

  return mis;
}  // End Solver::SequentialSolve

};  // namespace ruby

