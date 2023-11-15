#include <algorithm>
#include <chrono>
#include <iostream>
#include <iomanip>
#include <set>
#include <string>
#include <utility>
#include <vector>

#include <cstdlib>
#include <omp.h>

#include "src/solver/solver.h"

namespace mis {

std::vector<int> Solver::Run(int num_vertices,
                             const std::vector<std::pair<int, int>>& edges,
                             std::string alg,
                             int num_threads) {
  // Print info
  std::cout << "[Info] Algorithm: " << alg << "\n";
  std::cout << "[Info] Number of threads: " << num_threads << "\n";
  std::cout << "[Info] Start solving...\n";

  // Set number of threads use in omp
  omp_set_num_threads(num_threads);

  // Fix random seed
  std::srand(0);
 
  // Solve by the specified algorithm
  std::vector<int> mis;
  if (alg == "SeqGreedy") {
    mis = SequentialGreedySolve(num_vertices, edges);
  } else if (alg == "Luby") {
    mis = LubySolve(num_vertices, edges);
  } else if (alg == "Blelloch") {
    mis = BlellochSolve(num_vertices, edges);
  } else {
    std::cerr << "[Error] Unknown algorithm mode " << std::quoted(alg) << "\n";
    return {};
  }

  // Print the size of MIS
  std::cout << "[Info] |MIS| = " << mis.size() << "\n";

  return mis;
}  // End Solver::Run
  
/** Sequential greedy algorithm for MIS.
 *  Sort all vertices by degree in the increasing order, and then select
 *  vertices sequtentially if they can be added into an MIS.
 */
std::vector<int> Solver::SequentialGreedySolve(
    int num_vertices,
    const std::vector<std::pair<int, int>>& edges) {
  
  auto start = std::chrono::high_resolution_clock::now();
  
  // Build adjaceny list
  std::vector<std::vector<int>> adj(num_vertices);
  for (auto &[v1, v2] : edges) {
    adj[v1].emplace_back(v2);
    adj[v2].emplace_back(v1);
  }

  std::vector<int> mis;

  // Sequential method 
  std::vector<int> sorted_indices(num_vertices);
  std::generate(sorted_indices.begin(), sorted_indices.end(),
                [i = 0]() mutable { return i++; });
  //std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
                   //[&] (int lhs, int rhs) {
                     //return adj[lhs].size() < adj[rhs].size();
                   //});

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
    for (int nei : adj[i])
      marked[nei] = true;
  }

  auto stop = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "[Info] Runtime: " << duration.count() << " ms.\n";
  
  return mis;
}  // End Solver::SequentialGreedySolve

std::vector<int> Solver::LubySolve(
    int num_vertices,
    const std::vector<std::pair<int, int>>& edges) {
  auto start1 = std::chrono::high_resolution_clock::now();

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
  bool done = false;

  auto start2 = std::chrono::high_resolution_clock::now();

  while (!done) {
     ++iteration;
    //std::cout << "Iteration " << iteration << "\n";

    // foreach v in V(G)
    #pragma omp parallel for default(shared)
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
    #pragma omp parallel for default(shared)
    for (int v = 0; v < num_vertices; ++v) {
      int x_v;
      //#pragma omp atomic read
      x_v = X[v];
      if (x_v == iteration) {
        #pragma omp parallel for
        for (int nei : adj.at(v)) {
          int x_nei;
          x_nei = X[nei];
          if (x_nei == iteration) {
            int deg_v, deg_nei;
            deg_v = deg[v];
            deg_nei = deg[nei];
            if (deg_v < deg_nei) {
              marked[v] = iteration;
            } else if (deg_v == deg_nei && v < nei) {
              marked[v] = iteration;
            }
          }
        }
      }
    }

    // S <- S U X
    // Y <- X U deg(X)
    // G <- G|V(G)\Y
    #pragma omp parallel for default(shared)
    for (int v = 0; v < num_vertices; ++v) {
      if (G[v]) {
        int x_v;
        int marked_v;
        x_v = X[v];
        marked_v = marked[v];

        if (x_v == iteration && marked_v != iteration) {
          S[v] = true;
          G[v] = false;
        }
      }
    }
    
    #pragma omp parallel for default(shared)
    for (int v = 0; v < num_vertices; ++v) {
      if (G[v]) {
        for (int nei : adj[v]) {
          if (X[nei] == iteration && marked[nei] != iteration) {
            --deg[v];
            G[v] = false;
          }
        }
      }
    }
   
    #pragma omp single
    {
      done = true;
    }

    #pragma omp parallel for default(shared)
    for (int v = 0; v < num_vertices; ++v) {
      if (G[v])
        done = false;
    }
  }
  
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration1 =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start1);
  auto duration2 =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start2);
  std::cout << "[Info] Runtime1: " << duration1.count() << " ms.\n";
  std::cout << "[Info] Runtime2: " << duration2.count() << " ms.\n";
  std::cout << "[Info] Iteration: " << iteration << "\n";

  // Collect MIS from status
  std::vector<int> mis;
  for (int i = 0, n = S.size(); i < n; ++i) {
    if (S.at(i))
      mis.emplace_back(i);
  }

  return mis;
}  // End Solver::LubySolve

std::vector<int> Solver::BlellochSolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges) {
  auto start1 = std::chrono::high_resolution_clock::now();

  std::vector<int> priority_list(num_vertices);
  std::vector<int> permuted_indices(num_vertices);  

  // Generate a random permutation of the vertices
  std::generate(permuted_indices.begin(), permuted_indices.end(),
                [i = 0]() mutable { return i++; });
  std::random_shuffle(permuted_indices.begin(), permuted_indices.end());

  // Adjacency list (set)
  std::vector<std::set<int>> adj(num_vertices);
  for (auto const& [v1, v2] : edges) {
    adj[v1].emplace(v2);
    adj[v2].emplace(v1);
  }

  #pragma omp parallel for 
  for (int i = 0; i < num_vertices; ++i) {
    int my_priority = permuted_indices[i];
    int count_nbrs_before_me = 0;
    // iterate through all neighbors of i
    for (auto nei : adj[i]) {
      if (permuted_indices[nei] < my_priority) {
        ++count_nbrs_before_me;
      }
    }
    priority_list[i] = count_nbrs_before_me;
  }

  // compute the initial root
  std::vector<int> roots(num_vertices, 0);

  #pragma omp parallel for
  for(int i = 0; i < num_vertices; ++i) {
    int priority_i;
    priority_i = priority_list[i];
    if (priority_i == 0) {
      roots[i] = 1;
    }
  }

  std::vector<int> removed(num_vertices, 0);
  
  // recursively compute the MIS
  std::vector<bool> is_mis(num_vertices, false);

  auto start2 = std::chrono::high_resolution_clock::now();
  
  int iteration = 0;
  bool done = false;
  while (!done){
    iteration++;
    //std::cout << "Iteration " << iteration << "\n";

    // add roots to the MIS, and remove the nbrs of roots from the graph
    #pragma omp parallel for 
    for (int v = 0; v < num_vertices; ++v) {
      if (roots[v] == iteration) {
        is_mis[v] = true;
      }
    }
    #pragma omp parallel for 
    for (int v = 0; v < num_vertices; ++v) {
      if (priority_list[v] > 0) {
        for (auto nei : adj[v]) {
          if (roots[nei] == iteration && removed[v] != iteration) {
            priority_list[v] = 0;
            removed[v] = iteration;
          }
        }
      }
    }

    #pragma omp parallel for
    for (int v = 0; v < num_vertices; ++v) {
      for (auto nei : adj[v]){
        if (removed[nei] == iteration) {
          if (permuted_indices[v] > permuted_indices[nei]) {
            priority_list[v] = priority_list[v] - 1;
            if (priority_list[v] == 0) {
              roots[v] = iteration + 1;
            }
          }
        }
      }
    }

    #pragma omp single 
    {
      done = true;
    }

    #pragma omp parallel for 
    for (int v = 0; v < num_vertices; ++v) {
      if (roots[v] == iteration + 1) {
        done = false;
      }
    }
  }
  
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration1 =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start1);
  auto duration2 =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start2);
  std::cout << "[Info] Runtime1: " << duration1.count() << " ms.\n";
  std::cout << "[Info] Runtime2: " << duration2.count() << " ms.\n";
  std::cout << "[Info] Iterations: " << iteration << "\n";

  // Collect MIS from status
  std::vector<int> mis;
  for (int i = 0; i < num_vertices; ++i) {
    if (is_mis[i]) {
      mis.emplace_back(i);
    }
  }

  return mis;
}  // End Solver::BlellochSolve


}  // namespace mis

