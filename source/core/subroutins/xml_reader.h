#ifndef _CORE__SUBROUTINS__XML_READER_H_
#define _CORE__SUBROUTINS__XML_READER_H_

#include "models_errors.h"

#include "pugixml.hpp"

#include <memory>
#include <string>
#include <vector>

#include <string.h>

#ifdef _DEBUG
# include <iostream>
#endif  // _DEBUG

// xml_file type
// #define GAS_COMPONENT_FILE      0
// #define GASMIX_FILE             1

typedef unsigned int node_type;
#define NODE_T_ROOT         0
#define NODE_T_SECTION      1
#define NODE_T_GROUP        2
#define NODE_T_SUBGROUP     3
#define NODE_T_PARAMETER    4
#define NODE_T_UNDEFINED    0xFF
// parse xml with pugixml.lib
// pugixml library (http://pugixml.org).
// pugixml is Copyright (C) 2006-2018 Arseny Kapoulkine.

// class for initializing gas component by file
class gas_node {
public:
  node_type gas_node_type;
  std::string name,
              value;
              
  gas_node(node_type itype, std::string name);
  gas_node(node_type itype, std::string name, std::string value);
  gas_node(std::string type, std::string name);
  gas_node(std::string type, std::string name, std::string value);
  
  static std::string get_root_name();
};

// class for initializing gasmix by file
class gasmix_node {
public:
  node_type mix_node_type;
  std::string name,
              value;
  
  gasmix_node(node_type itype, std::string value);
  gasmix_node(node_type itype, std::string name, std::string value);
  gasmix_node(std::string type, std::string name);
  gasmix_node(std::string type, std::string name, std::string value);
  
  static std::string get_root_name();
};

template <class xml_node_t>
class gasxml_node {
  typedef std::vector<std::unique_ptr<gasxml_node<xml_node_t>>> vec_sibling;
  
public:
  std::unique_ptr<gasxml_node<xml_node_t>> first_child;
  vec_sibling siblings;
  xml_node_t node;
              
public:
  gasxml_node(xml_node_t node)
    : node(node) {}
  
  const gasxml_node<xml_node_t> *search_child_by_name(std::string name) const {
    if (first_child == nullptr)
      return nullptr;
    if (first_child->GetName() == name)
      return first_child.get();
    for (const auto &x : first_child->siblings)
      if (x->GetName() == name)
        return x.get();
    return nullptr;
  }
  
  std::string GetName() const {
    return node.name;
  }
  
  std::string GetValue() const {
    return node.value;
  }
  
  void SetName(const std::string &name) {
    node.name = name;
  }
  
  void SetValue(const std::string &value) {
    node.value = value;
  }
};

// template <class gasxml_node>
template <class xml_node_t>
class XMLReader {
  int xml_type_;
  std::string gas_xml_file_;
  std::unique_ptr<gasxml_node<xml_node_t>> gas_root_node_;
  pugi::xml_document xml_doc_;
  std::unique_ptr<pugi::xml_node> xml_root_node_;
  std::unique_ptr<pugi::xml_parse_result> xml_parse_result_;

private:
  XMLReader(std::string gas_xml_file)
    : gas_xml_file_(gas_xml_file), gas_root_node_(nullptr) {
    xml_parse_result_ = std::unique_ptr<pugi::xml_parse_result>(
        new pugi::xml_parse_result());  
    *xml_parse_result_ = xml_doc_.load_file(gas_xml_file.c_str());
    xml_root_node_ = std::unique_ptr<pugi::xml_node>(
        new pugi::xml_node());
    std::string root_node_name = xml_node_t::get_root_name();
    *xml_root_node_ = xml_doc_.child(root_node_name.c_str());
    gas_root_node_  = std::unique_ptr<gasxml_node<xml_node_t>>(
        new gasxml_node<xml_node_t>(xml_node_t(NODE_T_ROOT, root_node_name.c_str())));
    tree_traversal(xml_root_node_.get(), gas_root_node_.get());
  }
  
// xml_nd - temp node of xml document
// gas_nd - corresponding node of gas_tree(class XMLReader)
  // template <class gasxml_node>
  void tree_traversal(pugi::xml_node *xml_nd, gasxml_node<xml_node_t> *gasxml_nd) {
    if (strncmp(xml_nd->name(), "parameter", strlen("parameter")) != 0) {
      init_subnode(xml_nd, gasxml_nd);
    } else {
      init_parameters(xml_nd, gasxml_nd);
    }
  }
  
  // обход вглубь
  // void init_subnode(pugi::xml_node *xml_nd, gasmix_node *gasmix_nd);
  void init_subnode(pugi::xml_node *xml_nd, gasxml_node<xml_node_t> *gasxml_nd) {
    pugi::xml_node nx_xml_nd = xml_nd->first_child();
    gasxml_nd->first_child = std::unique_ptr<gasxml_node<xml_node_t>>(
        new gasxml_node<xml_node_t>(xml_node_t(nx_xml_nd.name(),
        nx_xml_nd.attribute("name").value())));
    tree_traversal(&nx_xml_nd, gasxml_nd->first_child.get());
    while (true) {
      if (nx_xml_nd == xml_nd->last_child())
        break;
      nx_xml_nd = nx_xml_nd.next_sibling();
      gasxml_nd->first_child->siblings.push_back(std::unique_ptr<gasxml_node<xml_node_t>>(
         new gasxml_node<xml_node_t>(xml_node_t(nx_xml_nd.name(),
         nx_xml_nd.attribute("name").value()))));
      tree_traversal(&nx_xml_nd, gasxml_nd->first_child->siblings.back().get());
    }
  }
  
  // template <class gasxml_node>
  void init_parameters(pugi::xml_node *&xml_nd, gasxml_node<xml_node_t> *gasxml_nd) {
    gasxml_nd->SetValue(xml_nd->text().as_string());
    gasxml_nd->SetName(xml_nd->attribute("name").value());
    gasxml_nd->first_child = nullptr;
  }

public:
  static XMLReader<xml_node_t> *Init(std::string gas_xml_file) {
    reset_error();
    if (gas_xml_file.empty()) {
      set_error_code(ERR_INIT_T | ERR_INIT_NULLP_ST);
      return nullptr;
    }
    // check file exist
    FILE *f = fopen(gas_xml_file.c_str(), "r");
    if (!f) {
      set_error_code(ERR_FILEIO_T | ERR_FILE_IN_ST);
      set_error_message(std::string(
          std::string("cannot open file: ") + gas_xml_file).c_str());
      return nullptr;
    }
    fclose(f);
    return new XMLReader<xml_node_t>(gas_xml_file);
  }
  
  std::string GetRootName() {
    return gas_root_node_->GetName();
  }
  
  error_t GetValueByPath(const std::vector<std::string> &xml_path,
      std::string *outstr) const {
    const gasxml_node<xml_node_t> *tmp_gas_node = gas_root_node_.get();
    for (const auto &x : xml_path) {
      // check ypuself and sublings
      tmp_gas_node = tmp_gas_node->search_child_by_name(x);
      if (!tmp_gas_node) {
        return 1;
      }
    }
    *outstr = tmp_gas_node->GetValue();
    return 0;
  } 
};
#endif  // !_CORE__SUBROUTINS__XML_READER_H_
