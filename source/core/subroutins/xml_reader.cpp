#include "xml_reader.h"

#include "models_errors.h"

#include <map>
#include <type_traits>

#include <assert.h>
#include <stdio.h>

// gas_node
gas_node::gas_node(node_type itype, std::string name)
  : gas_node_type(itype), name(name), value("") {}

gas_node::gas_node(node_type itype, std::string name, std::string value)
  : gas_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  std::cerr << "Create gas node: " << value << " #" << itype << " " << name << "\n";
#endif  // _DEBUG
  }

gas_node::gas_node(std::string type, std::string name)
  : gas_node(type, name, "") {}

gas_node::gas_node(std::string type, std::string name, std::string value)
  : name(name), value(value) {
  if (type == "section")
    gas_node_type = NODE_T_SECTION;
  else if (type == "group")
    gas_node_type = NODE_T_GROUP;
  else if (type == "subgroup")
    gas_node_type = NODE_T_SUBGROUP;
  else if (type == "parameter")
    gas_node_type = NODE_T_PARAMETER;
  else
    gas_node_type = NODE_T_UNDEFINED;
}

// static
std::string gas_node::get_root_name() {
  return "gas";
}
  
/*
gasmix_node::gasmix_node(int itype) 
  : mix_node_type(itype), value("") {}
*/
gasmix_node::gasmix_node(node_type itype, std::string value)
  : mix_node_type(itype), value(value) {} 

gasmix_node::gasmix_node(node_type itype, std::string name, std::string value)
  : mix_node_type(itype), name(name), value(value) {
#ifdef _DEBUG_SUBROUTINS
  std::cerr << "Create gasmix node: " << value << " #" << itype << " " << name << "\n";
#endif
}

gasmix_node::gasmix_node(std::string type, std::string name)
  : gasmix_node(type, name, "") {}

gasmix_node::gasmix_node(std::string type, std::string name, std::string value)
  : name(name), value(value) {
  if (type == "group")
    mix_node_type = NODE_T_GROUP;
  else if (type == "parameter")
    mix_node_type = NODE_T_PARAMETER;
  else
    mix_node_type = NODE_T_UNDEFINED;
#ifdef _DEBUG_SUBROUTINS
  std::cerr << "Create gasmix node: " << value << " " << type << " " << name << "\n";
#endif
}

// static
std::string gasmix_node::get_root_name() {
  return "gasmix";
}
