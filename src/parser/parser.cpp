#include <fstream>
#include <iomanip>

#include "src/parser/parser.h"

namespace mis {

bool Parser::Read(std::string filename,
                  int& num_vertices,
                  std::vector<std::pair<int, int>>& edges) {
  std::cout << "[Info] Reading " << std::quoted(filename) << "\n";

  // Open file
  std::ifstream fin(filename, std::ios::in);
  if (!fin) {
    std::cerr << "[Error] Cannot open " << std::quoted(filename) << ".\n";
    return false;
  }

  // Parse num_vertices and num_edges
  int num_edges;
  fin >> num_vertices;
  fin >> num_edges;

  // Parse edges
  edges.clear();
  int v1 = 0;
  int v2 = 0;
  for (int i = 0; i < num_edges; ++i) {
    fin >> v1 >> v2;
    edges.emplace_back(v1, v2);
  }

  // Close file
  fin.close();

  // Print #vertices / #edges
  std::cout << "[Info] #Vertices: " << num_vertices << "\n";
  std::cout << "[Info] #Edges: " << edges.size() << "\n";

  return true;
}  // Parser::Read

}  // namespace mis

