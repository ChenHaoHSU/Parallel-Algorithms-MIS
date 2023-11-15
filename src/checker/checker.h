#ifndef MIS_CHECKER_H_
#define MIS_CHECKER_H_

#include <iostream>
#include <vector>

namespace mis {

class Checker {
 public:
  Checker() = default;
  ~Checker() = default;

 public:
  bool Run(int num_vertices,
           const std::vector<std::pair<int, int>>& edges,
           const std::vector<int>& mis);
  bool CheckIsIndependentSet(int num_vertices,
                             const std::vector<std::pair<int, int>>& edges,
                             const std::vector<int>& mis);
  bool CheckIsMaximal(int num_vertices,
                      const std::vector<std::pair<int, int>>& edges,
                      const std::vector<int>& mis);

};  // class Checker

}  // namespace mis

#endif  // MIS_CHECKER_H_

