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

namespace luby {

std::vector<int> Solver::Run(int num_vertices,
                             const std::vector<std::pair<int, int>>& edges,
                             std::string alg) {
  std::cout << "[Info] Algorithm: " << alg << "\n";
  std::cout << "[Info] Start solving...\n";
  
  // Fix random seed
  std::srand(0);
  
  std::vector<int> mis;
  if (alg == "SeqGreedy") {
    mis = SequentialGreedySolve(num_vertices, edges);
  } else if (alg == "SeqLuby") {
    mis = SequentialLubySolve(num_vertices, edges);
  } else if (alg == "ParLuby") {
    mis = ParallelLubySolve(num_vertices, edges);
  } else if (alg == "SeqRoot") {
    mis = SequentialRootBasedSolve(num_vertices, edges);
  } else if (alg == "ParRoot") {
    mis = ParallelRootBasedSolve(num_vertices, edges);
  } else {
    std::cerr << "[Error] Unknown algorithm mode " << std::quoted(alg) << "\n";
    return {};
  }
  
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
  std::stable_sort(sorted_indices.begin(), sorted_indices.end(),
                   [&] (int lhs, int rhs) {
                     return adj[lhs].size() < adj[rhs].size();
                   });

  auto start = std::chrono::high_resolution_clock::now();
  
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
    //std::cout << "Iteration " << iteration << "\n";

    // foreach v in V(G)
    #pragma omp parallel for default(shared)
    for (int v = 0; v < num_vertices; ++v) {
      if (G.at(v)) {
        if (deg[v] == 0) {
          #pragma omp atomic write
          X[v] = iteration;
        } else {
          double x = ((double)std::rand() / (double)RAND_MAX);
          double prob = ((double)1.0 / (double)(2 * deg[v]));
          if (x < prob) {
            #pragma omp atomic write
            X[v] = iteration;
          }
        }
      }  // End if G.at(v)
    }  // End for v = 0..n-1
    
    // foreach unordered pair (v, w) in X x X
    {
      #pragma omp parallel for default(shared)
      for (int v = 0; v < num_vertices; ++v) {
        int x_v;
        #pragma omp atomic read
        x_v = X[v];
        if (x_v == iteration) {
          #pragma omp parallel for
          for (int nei : adj.at(v)) {
            int x_nei;
            #pragma omp atomic read
            x_nei = X[nei];
            if (x_nei == iteration) {
              int deg_v, deg_nei;
              #pragma omp atomic read
              deg_v = deg[v];
              #pragma omp atomic read
              deg_nei = deg[nei];
              if (deg_v < deg_nei) {
                #pragma omp atomic write
                marked[v] = iteration;
              } else if (deg_v == deg_nei && v < nei) {
                #pragma omp atomic write
                marked[v] = iteration;
              }
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
  std::cout << "[Info] Iteration: " << iteration << "\n";

  // Collect MIS from status
  std::vector<int> mis;
  for (int i = 0, n = S.size(); i < n; ++i) {
    if (S.at(i))
      mis.emplace_back(i);
  }

  return mis;
}  // End Solver::ParallelLubySolve

std::vector<int> Solver::SequentialRootBasedSolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges) {
  std::vector<int> mis;

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

  // #pragma omp parallel for 
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
  std::vector<int> roots;
  auto zero_f = [&](int i) { return priority_list[i] == 0; };

  // #pragma omp parallel for
  for(int i = 0; i < num_vertices; ++i) {
    if (zero_f(i)) {
      roots.emplace_back(i);
    }
  }

  // recursively compute the MIS
  std::vector<bool> is_mis(num_vertices, false);
  int finished = 0;
  int round = 0;

  auto start = std::chrono::high_resolution_clock::now();
  while (finished != num_vertices){
    // while(round < 10){
    std::vector<int> removed;
    // add roots to the MIS, and remove the nbrs of roots from the graph
    // #pragma omp parallel for 
    for (auto root: roots) {
      is_mis[root] = true;
      for (auto nei : adj[root]) {
        if (priority_list[nei] > 0) {
          priority_list[nei] = 0;
          removed.emplace_back(nei);
        }
      }
    }

    // compute new roots
    std::vector<int> new_roots;
    // #pragma omp parallel for 
    for (auto u : removed) {
      for (auto v: adj[u]){
         if ( permuted_indices[v] > permuted_indices[u] ) {
          priority_list[v]-=1;
          if (priority_list[v] == 0) {
            new_roots.emplace_back(v);
          }
        }
      }
    }
    
    finished += roots.size();
    finished += removed.size();
          
    roots = std::move(new_roots);
    ++round;
  }

  auto stop = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "[Info] Runtime: " << duration.count() << " ms.\n";
  std::cout << "[Info] Rounds: " << round << "\n";

  // # pragma omp parallel for 
  for (int i = 0; i < num_vertices; ++i) {
    if (is_mis[i]) {
      mis.emplace_back(i);
    }
  }

  return mis;
}  // End Solver::SequentialRootBasedSolve

std::vector<int> Solver::ParallelRootBasedSolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges) {
  std::vector<int> mis;

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

  auto start = std::chrono::high_resolution_clock::now();
  
  int round = 0;
  bool done = false;
  while (!done){
    round++;

    std::cout << "Round " << round << "\n";
    // while(round < 10){
    // add roots to the MIS, and remove the nbrs of roots from the graph
    #pragma omp parallel for 
    for (int v = 0; v < num_vertices; ++v) {
      if (roots[v] == round) {
        is_mis[v] = true;
      }
    }
    #pragma omp parallel for 
    for (int v = 0; v < num_vertices; ++v) {
      if (priority_list[v] > 0) {
        for (auto nei : adj[v]) {
          if (roots[nei] == round && removed[v] != round) {
            priority_list[v] = 0;
            removed[v] = round;
          }
        }
      }
    }

    #pragma omp parallel for 
    for (int v = 0; v < num_vertices; ++v) {
      for (auto nei : adj[v]){
        if (removed[nei] == round) {
          if (permuted_indices[v] > permuted_indices[nei]) {
            priority_list[v] -= 1;
            if (priority_list[v] == 0) {
              roots[v] = round + 1;
            }
          }
        }
      }
    }

    done = true;
    #pragma omp parallel for 
    for (int v = 0; v < num_vertices; ++v) {
      if (roots[v] == round + 1) {
        done = false;
      }
    }
  }
  
  auto stop = std::chrono::high_resolution_clock::now();
  auto duration =
    std::chrono::duration_cast<std::chrono::milliseconds>(stop - start);
  std::cout << "[Info] Runtime: " << duration.count() << " ms.\n";
  std::cout << "[Info] Rounds: " << round << "\n";

  // # pragma omp parallel for 
  for (int i = 0; i < num_vertices; ++i) {
    if (is_mis[i]) {
      mis.emplace_back(i);
    }
  }

  return mis;
}  // End Solver::ParallelRootBasedSolve


}  // namespace luby

