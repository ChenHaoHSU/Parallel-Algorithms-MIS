#include <algorithm>
#include <chrono>
#include <iostream>
#include <set>
#include <utility>
#include <vector>

#include <cstdlib>
#include <omp.h>

#include "src/solver/solver.h"

namespace luby {

std::vector<int> Solver::Run(int num_vertices,
                             const std::vector<std::pair<int, int>>& edges) {
  std::cout << "[Info] Start solving...\n"; 
  
  // Fix random seed
  std::srand(0);
  
  // Build the adjacency list
  BuildAdj(num_vertices, edges);

  //std::vector<int> mis = SequentialSolve(num_vertices, edges);
  //std::vector<int> mis = SequentialLubySolve(num_vertices, edges);
  std::vector<int> mis = ParallelLubySolve(num_vertices, edges);
  
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

#ifdef LUBY_DEBUG
  for (unsigned i = 0; i < adj_.size(); ++i) {
    std::cout << "[" << i << "]";
    for (auto nei : adj_[i]) {
      std::cout << " " << nei;
    }
    std::cout << "\n";
  }
#endif  // LUBY_DEBUG

}  // End Solver::BuildAdj

/** Sequential greedy algorithm for MIS.
 *  Sort all vertices by degree in the increasing order, and then select
 *  vertices sequtentially if they can be added into an MIS.
 */
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

std::vector<int> Solver::SequentialLubySolve(
    int num_vertices,
    const std::vector<std::pair<int, int>>& edges) {
  // Status vector (true if in MIS)
  std::vector<bool> S(num_vertices, false);

  // Active vertices
  std::set<int> G;
  for (int i = 0; i < num_vertices; ++i) {
    G.emplace(i);
  }

  // Adjacency list (set)
  std::vector<std::set<int>> adj(num_vertices);
  for (auto const& [v1, v2] : edges) {
    adj[v1].emplace(v2);
    adj[v2].emplace(v1);
  }

  // Marked
  std::vector<int> marked(num_vertices, 0);

  // Degree
  std::vector<int> deg(num_vertices, 0);
  for (auto const& [v1, v2] : edges) {
    ++deg[v1];
    ++deg[v2];
  }

  // Luby's algorithm starts here
  int iteration = 0;
  while (!G.empty()) {
    ++iteration;
    //std::cout << "Iteration " << iteration << "\n";

    // X <- emptySet
    std::set<int> X;

    // foreach v in V(G)
    for (int v : G) {
      if (deg[v] == 0) {
        X.emplace(v);
      } else {
        double x = ((double)std::rand() / (double)RAND_MAX);
        double prob = ((double)1.0 / (double)(2 * deg[v]));
        if (x < prob) {
          X.emplace(v);
        }
      }
    }
    
    // foreach unordered pair (v, w) in X x X
    for (int v : X) {
      for (int nei : adj.at(v)) {
        if (X.count(nei) > 0) {
          if (deg[v] < deg[nei]) {
            marked[v] = iteration;
          } else if (deg[v] == deg[nei] && v < nei) {
            marked[v] = iteration;
          }
        }
      }
    }

    // S <- S U X
    // Y <- X U deg(X)
    // G <- G|V(G)\Y
    for (int v : X) {
      if (marked.at(v) != iteration) {
        S[v] = true;
        for (int nei : adj.at(v)) {
          --deg[nei];
          G.erase(nei);
        }
        G.erase(v);
      }
    }
  }

  // Collect MIS from status
  std::vector<int> mis;
  for (int i = 0, n = S.size(); i < n; ++i) {
    if (S.at(i))
      mis.emplace_back(i);
  }

  return mis;
}  // End Solver::SequentialLubySolve

std::vector<int> Solver::ParallelLubySolve(
    int num_vertices,
    const std::vector<std::pair<int, int>>& edges) {
  // Status vector (true if in MIS)
  std::vector<bool> S(num_vertices, false);

  // Active vertices
  // True if the vertex is still active (not removed)
  std::vector<int> G(num_vertices, true);

  // Adjacency list (set)
  std::vector<std::vector<int>> adj(num_vertices);
  for (auto const& [v1, v2] : edges) {
    adj[v1].emplace_back(v2);
    adj[v2].emplace_back(v1);
  }

  // Marked
  std::vector<int> marked(num_vertices, 0);

  // Degree
  std::vector<int> deg(num_vertices, 0);
  for (auto const& [v1, v2] : edges) {
    ++deg[v1];
    ++deg[v2];
  }

  // X set
  std::vector<int> X(num_vertices, 0);

  // Luby's algorithm starts here
  int iteration = 0;
  int num_active = num_vertices;
  
  auto start = std::chrono::high_resolution_clock::now();
  while (num_active > 0) {
     ++iteration;
    std::cout << "Iteration " << iteration << "\n";

    // foreach v in V(G)
#pragma omp parallel for
    for (int v = 0; v < num_vertices; ++v) {
      if (G.at(v)) {
        if (deg[v] == 0) {
          X[v] = iteration;
        } else {
          double x = ((double)std::rand() / (double)RAND_MAX);
          double prob = ((double)1.0 / (double)(2 * deg[v]));
          if (x < prob) {
            X[v] = iteration;
          }
        }
      }  // End if G.at(v)
    }  // End for v = 0..n-1
    
    // foreach unordered pair (v, w) in X x X
#pragma omp parallel for
    for (int v = 0; v < num_vertices; ++v) {
      if (X[v] == iteration) {
#pragma omp parallel for
        for (int nei : adj.at(v)) {
          if (X.at(nei) == iteration) {
            if (deg[v] < deg[nei]) {
              marked[v] = iteration;
            } else if (deg[v] == deg[nei] && v < nei) {
              marked[v] = iteration;
            }
          }
        }
      }
    }

    // S <- S U X
    // Y <- X U deg(X)
    // G <- G|V(G)\Y
    for (int v = 0; v < num_vertices; ++v) {
      if (X.at(v) == iteration && marked.at(v) != iteration) {
        S[v] = true;
        for (int nei : adj.at(v)) {
          --deg[nei];
          if (G[nei]) {
            G[nei] = false;
            --num_active;
          }
        }
        if (G[v]) {
          G[v] = false;
          --num_active;
        }
      }
    }
  }
  
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "[Info] Runtime: " << duration.count() << " ms.\n";


  // Collect MIS from status
  std::vector<int> mis;
  for (int i = 0, n = S.size(); i < n; ++i) {
    if (S.at(i))
      mis.emplace_back(i);
  }

  return mis;
}  // End Solver::ParallelLubySolve

}  // namespace luby

