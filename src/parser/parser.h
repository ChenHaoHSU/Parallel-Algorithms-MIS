#ifndef RUBY_PARSER_H_
#define RUBY_PARSER_H_

#include <iostream>
#include <vector>

namespace ruby {

class Parser {
 public:
  Parser() = default;
  ~Parser() = default;

 public:
  bool Read(std::string filename,
            int& num_vertices,
            std::vector<std::pair<int, int>>& edges);

};  // class Parser

}  // namespace ruby 

#endif  // RUBY_PARSER_H_

