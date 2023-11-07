#ifndef LUBY_PARSER_H_
#define LUBY_PARSER_H_

#include <iostream>
#include <vector>

namespace luby {

class Parser {
 public:
  Parser() = default;
  ~Parser() = default;

 public:
  bool Read(std::string filename,
            int& num_vertices,
            std::vector<std::pair<int, int>>& edges);

};  // class Parser

}  // namespace luby

#endif  // LUBY_PARSER_H_

