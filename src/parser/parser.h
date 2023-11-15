#ifndef MIS_PARSER_H_
#define MIS_PARSER_H_

#include <iostream>
#include <vector>

namespace mis {

class Parser {
 public:
  Parser() = default;
  ~Parser() = default;

 public:
  bool Read(std::string filename,
            int& num_vertices,
            std::vector<std::pair<int, int>>& edges);

};  // class Parser

}  // namespace mis

#endif  // MIS_PARSER_H_

