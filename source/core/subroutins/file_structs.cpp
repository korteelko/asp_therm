#include "file_structs.h"

#include "configuration_strtpl.h"
#include "models_logging.h"

#include <assert.h>
#include <stdio.h>

std::array<std::string, GAS_NODE_COUNT> gas_node::node_t_list = {
  "gas", "section", "group", "subgroup", "parameter"
};

std::array<std::string, GASMIX_NODE_COUNT> gasmix_node::node_t_list = {
  "gasmix", "group", "parameter"
};

std::array<std::string, CONFIG_NODE_COUNT> config_node::node_t_list = {
  "program_config", "group", "parameter"
};

// config_node
config_node::config_node(node_type itype, std::string name)
  : config_node_type(itype), name(name), value("") {}

config_node::config_node(node_type itype, std::string name, std::string value)
  : config_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  Logging::Append(io_loglvl::debug_logs, "Create confignode: %s #%d %s",
      value.c_str(), itype, name.c_str());
#endif  // _DEBUG
  }

// static
std::string config_node::get_root_name() {
  return config_node::node_t_list[NODE_T_ROOT];
}

node_type config_node::get_node_type(std::string type) {
  for (uint32_t i = 0; i < config_node::node_t_list.size(); ++i)
    if (config_node::node_t_list[i] == type)
      return i;
  return NODE_T_UNDEFINED;
}

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


/* PARSE TEMPLATE VALUES */
merror_t set_bool(const std::string &val, bool &ans) {

}
