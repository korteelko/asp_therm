#ifndef _CORE__SUBROUTINS__FILE_STRUCTS_H_
#define _CORE__SUBROUTINS__FILE_STRUCTS_H_

#include <array>
#include <string>

#include <stdint.h>

typedef uint32_t node_type;
#define GAS_NODE_COUNT      5
#define GASMIX_NODE_COUNT   3
#define NODE_T_ROOT         0
#define NODE_T_UNDEFINED    0xff

/// class for initializing gas component by file
class gas_node {
  static std::array<std::string, GAS_NODE_COUNT> node_t_list;

public:
  node_type gas_node_type;
  std::string name,
              value;

  gas_node(node_type itype, std::string name);
  gas_node(node_type itype, std::string name, std::string value);

  static std::string get_root_name();
  static node_type get_node_type(std::string type);
};

/// class for initializing gasmix by file
class gasmix_node {
  static std::array<std::string, GASMIX_NODE_COUNT> node_t_list;

public:
  node_type mix_node_type;
  std::string name,
              value;

  gasmix_node(node_type itype, std::string name);
  gasmix_node(node_type itype, std::string name, std::string value);

  static std::string get_root_name();
  static node_type get_node_type(std::string type);
};

#endif  // !_CORE__SUBROUTINS__FILE_STRUCTS_H_
