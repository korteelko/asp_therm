#include "file_structs.h"

#include "models_logging.h"

#include <assert.h>
#include <stdio.h>

std::array<std::string, GAS_NODE_COUNT> gas_node::node_t_list = {
  "gas", "section", "group", "subgroup", "parameter"
};

std::array<std::string, GASMIX_NODE_COUNT> gasmix_node::node_t_list = {
  "gasmix", "group", "parameter"
};

// gas_node
gas_node::gas_node(node_type itype, std::string name)
  : gas_node_type(itype), name(name), value("") {}

gas_node::gas_node(node_type itype, std::string name, std::string value)
  : gas_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  Logging::Append(io_loglvl::debug_logs, "Create gasnode: %s #%d %s",
      value.c_str(), itype, name.c_str());
#endif  // _DEBUG
  }

// static
std::string gas_node::get_root_name() {
  return gas_node::node_t_list[NODE_T_ROOT];
}

node_type gas_node::get_node_type(std::string type) {
  for (uint32_t i = 0; i < gas_node::node_t_list.size(); ++i)
    if (gas_node::node_t_list[i] == type)
      return i;
  return NODE_T_UNDEFINED;
}

gasmix_node::gasmix_node(node_type itype, std::string name)
  : mix_node_type(itype), name(name) {}

gasmix_node::gasmix_node(node_type itype, std::string name, std::string value)
  : mix_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  Logging::Append(io_loglvl::debug_logs, "Create gasmixnode: %s #%d %s",
      value.c_str(), itype, name.c_str());
#endif
}

// static
std::string gasmix_node::get_root_name() {
  return gasmix_node::node_t_list[NODE_T_ROOT];
}

node_type gasmix_node::get_node_type(std::string type) {
  for (uint32_t i = 0; i < gasmix_node::node_t_list.size(); ++i)
    if (gasmix_node::node_t_list[i] == type)
      return i;
  return NODE_T_UNDEFINED;
}
