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
      std::string alg = "SeqGreedy");

 private:
  std::vector<int> SequentialGreedySolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);
  std::vector<int> SequentialLubySolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);
  std::vector<int> ParallelLubySolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);
};  // class Solver

}  // namespace luby

#endif  // LUBY_SOLVER_H_

