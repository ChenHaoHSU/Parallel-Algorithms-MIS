#include <iostream>
#include <vector>

#include "src/checker/checker.h"

namespace luby {

bool Checker::Run(
    int num_vertices,
    const std::vector<std::pair<int, int>>& edges,
    const std::vector<int>& mis) {
  bool valid = true;

  // Check if it is an independent set
  if (!CheckIsIndependentSet(num_vertices, edges, mis)) {
    valid = false;
    std::cout << "[Error] Checker: Not an independent set.\n";
  }

  // Check if it is maximal
  if (!CheckIsMaximal(num_vertices, edges, mis)) {
    valid = false;
    std::cout << "[Error] Checker: Not maximal.\n";
  }

  return valid;
}  // End Checker::Run

bool Checker::CheckIsIndependentSet(
    int num_vertices,
    const std::vector<std::pair<int, int>>& edges,
    const std::vector<int>& mis) {
  // Mark the vertices in MIS
  std::vector<bool> marked(num_vertices, false);
  for (int v : mis) {
    marked[v] = true;
  }

  // Check if each edge has at most one vertex marked (selected)
  bool valid = true;
  for (auto &[v1, v2] : edges) {
    if (marked[v1] && marked[v2]) {
      valid = false;
      std::cout << "[Error] Checker: Vertices in edge "
          << "(" << v1 << ", " << v2 << ") are selected. "
          << "Not an independent set\n";
    }
  }

  return valid;
}  // End Checker::CheckIsIndependentSet

bool Checker::CheckIsMaximal(
    int num_vertices,
    const std::vector<std::pair<int, int>>& edges,
    const std::vector<int>& mis) {
  // Build the adjacency list
  std::vector<std::vector<int>> adj(num_vertices);
  for (auto &[v1, v2] : edges) {
    adj[v1].emplace_back(v2);
    adj[v2].emplace_back(v1);
  }

  // Mark the vertices in MIS
  std::vector<bool> marked(num_vertices, false);
  for (int v : mis) {
    marked[v] = true;
  }

  // Mark the neighbors of the vertices in MIS
  for (int v : mis) {
    for (int nei : adj[v]) {
      marked[nei] = true;
    }
  }

  // Check if there is any vertex unmarked
  bool valid = true;
  for (int i = 0; i < num_vertices; ++i) {
    if (!marked[i]) {
      valid = false;
      std::cout << "[Error] Checker: vertex " << i << " can be added. Not maximal.\n";
    }
  }

  return valid;
}  // End Checker::CheckIsMaximal

}  // namespace luby

