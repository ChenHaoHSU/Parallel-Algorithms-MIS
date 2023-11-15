#ifndef LUBY_SOLVER_H_
#define LUBY_SOLVER_H_

#include <iostream>
#include <vector>

namespace luby {

class Solver {
 public:
  Solver() = default;
  ~Solver() = default;

 public:
  std::vector<int> Run(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges,
      std::string alg = "SeqGreedy",
      int num_threads = 1);

 private:
  std::vector<int> SequentialGreedySolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);
  std::vector<int> LubySolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);
  std::vector<int> BlellochSolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);

};  // class Solver

}  // namespace luby

#endif  // LUBY_SOLVER_H_

