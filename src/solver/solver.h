#ifndef RUBY_SOLVER_H_
#define RUBY_SOLVER_H_

#include <iostream>
#include <vector>

namespace ruby {

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

 private:
  std::vector<std::vector<int>> adj_;

};  // class Solver

}  // namespace ruby 

#endif  // RUBY_SOLVER_H_

