#ifndef _CORE__SUBROUTINS__XML_READER_H_
#define _CORE__SUBROUTINS__XML_READER_H_

#include "pugixml.hpp"

#include <memory>
#include <string>
#include <vector>

#define GAS_NODE_T_ROOT      0
#define GAS_NODE_T_SECTION   1
#define GAS_NODE_T_GROUP     2
#define GAS_NODE_T_SUBGROUP  3
#define GAS_NODE_T_PARAMETER 4
#define GAS_NODE_T_UNDEFINED 0xFF
// parse xml with pugixml.lib

// pugixml library (http://pugixml.org).
// pugixml is Copyright (C) 2006-2018 Arseny Kapoulkine.

class gas_node {
  typedef std::vector<std::unique_ptr<gas_node>> vec_sibling;

public:
  int gas_node_t;
  std::unique_ptr<gas_node> first_child_;
  vec_sibling siblings_;
  std::string name_,
              value_;

  gas_node(int itype, std::string name);
  gas_node(int itype, std::string name, std::string value);
  gas_node(std::string type, std::string name);
  gas_node(std::string type, std::string name, std::string value);

  const gas_node *search_child_by_name(std::string name) const;
};

class XMLReader {
  std::string gas_xml_file_;
  std::unique_ptr<gas_node> gas_root_node_;
  pugi::xml_document xml_doc_;
  std::unique_ptr<pugi::xml_node> xml_root_node_;
  std::unique_ptr<pugi::xml_parse_result> xml_parse_result_;

private:
  XMLReader(std::string gas_xml_file);
  void tree_traversal(pugi::xml_node *xml_nd, gas_node *gas_nd);
  void init_subnode(pugi::xml_node *xml_nd, gas_node *gas_nd);
  void init_parameter(pugi::xml_node *xml_nd, gas_node *gas_nd);

public:
  static XMLReader *Init(std::string gas_xml_file);
  std::string GetRootName();
  int GetValueByPath(const std::vector<std::string> &xml_path,
      std::string *outstr) const;
};
#endif  // !_CORE__SUBROUTINS__XML_READER_H_
