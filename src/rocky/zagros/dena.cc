#define ROCKY_USE_MPI
#include <rocky/zagros/dena.h>

using namespace rocky::zagros;

// all registered nodes
std::vector<dena::flow_node_variant> dena::node::nodes = std::vector<dena::flow_node_variant>();
// flow transitions
std::map<int, int> dena::node::next_node = std::map<int, int>();