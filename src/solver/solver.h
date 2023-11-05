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

};  // class Solver

}  // namespace ruby 

#endif  // RUBY_SOLVER_H_

