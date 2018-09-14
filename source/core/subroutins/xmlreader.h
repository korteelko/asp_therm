#ifndef _CORE__COMMON__XML_READER_H_
#define _CORE__COMMON__XML_READER_H_

// #include "common.h"
#include "pugixml.hpp"

// #include <map>
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

// pugixml acknowledgment:

// This software is based on pugixml library (http://pugixml.org).
// pugixml is Copyright (C) 2006-2018 Arseny Kapoulkine.

  /// структура описывающая xml-файл
  /// В общем так:
  /// По переданной версии xml-файла создаём его скелет,
  /// инициализируя только тип узлов(section, group, subgroup, parameter),
  /// далее передаём это дело в чтец xml-файлов где паралельно проходим ещё и
  /// по файлу с определениями, инициализируя значения
  /// Так победим
class gas_node {
  typedef std::vector<std::unique_ptr<gas_node>> vec_sibling;

public:
  int gas_node_t;
  std::unique_ptr<gas_node> first_child_;
  vec_sibling siblings_;
  std::string name_,
              value_;

// private:
  // constructors
  gas_node(int itype, std::string name);
  gas_node(int itype, std::string name, std::string value);
  gas_node(std::string type, std::string name);
  gas_node(std::string type, std::string name, std::string value);

  // find name
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
  // recursive init gas_node_tree
  void tree_traversal(pugi::xml_node *xml_nd, gas_node *gas_nd);

public:
  static XMLReader *Init(std::string gas_xml_file);

  // pass to this method vector of strings -- path to parameter
  //   and get string value of this parameter
  // example (do not set at first root node "gas"!!!):
  //   "parameters", "constants", "critical_point", "pressure"
  // return 0 for success case
  int GetValueByPath(const std::vector<std::string> &xml_path,
      std::string *outstr) const;
};
#endif // ! _CORE__COMMON__XML_READER_H_
