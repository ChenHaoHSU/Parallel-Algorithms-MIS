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
      const std::vector<std::pair<int, int>>& edges);

 private:
  void BuildAdj(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);
  std::vector<int> SequentialSolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);
  std::vector<int> SequentialLubySolve(
      int num_vertices,
      const std::vector<std::pair<int, int>>& edges);

 private:
  std::vector<std::vector<int>> adj_;

};  // class Solver

}  // namespace luby

#endif  // LUBY_SOLVER_H_

