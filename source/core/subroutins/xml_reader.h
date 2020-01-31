#ifndef _CORE__SUBROUTINS__XML_READER_H_
#define _CORE__SUBROUTINS__XML_READER_H_

#include "models_errors.h"
#include "models_logging.h"

// pugixml library (http://pugixml.org).
// pugixml is Copyright (C) 2006-2018 Arseny Kapoulkine.
#include "pugixml.hpp"

#include <memory>
#include <string>
#include <vector>

#include <stdint.h>
#include <string.h>

// xml_file type
// #define GAS_COMPONENT_FILE      0
// #define GASMIX_FILE             1

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
  
  const gasxml_node<xml_node_t> *search_child_by_name(
      std::string name) const {
    gasxml_node<xml_node_t> *child = nullptr;
    if (first_child != nullptr) {
      if (first_child->GetName() == name) {
        child = first_child.get();
      } else {
    #if defined (_DEBUG_SUBROUTINS)
        Logging::Append(io_loglvl::debug_logs, "Search for '%s' but get %s",
            name, first_child->GetName());
    #endif  // _DEBUG_SUBROUTINS
        for (const auto &x : first_child->siblings)
          if (x->GetName() == name) {
            child = x.get();
            break;
          }
      }
    }
    return child;
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

template <class xml_node_t>
class XMLReader {
  merror_t error_;
  std::string gas_xml_file_;
  std::unique_ptr<gasxml_node<xml_node_t>> gas_root_node_;
  pugi::xml_document xml_doc_;
  std::unique_ptr<pugi::xml_node> xml_root_node_;
  std::unique_ptr<pugi::xml_parse_result> xml_parse_result_;

private:
  XMLReader(std::string gas_xml_file)
    : error_(ERR_SUCCESS_T), gas_xml_file_(gas_xml_file),
      gas_root_node_(nullptr) {
    xml_parse_result_ = std::unique_ptr<pugi::xml_parse_result>(
        new pugi::xml_parse_result());  
    *xml_parse_result_ = xml_doc_.load_file(gas_xml_file.c_str());
    xml_root_node_ = std::unique_ptr<pugi::xml_node>(
        new pugi::xml_node());
    std::string root_node_name = xml_node_t::get_root_name();
    *xml_root_node_ = xml_doc_.child(root_node_name.c_str());
    gas_root_node_  = std::unique_ptr<gasxml_node<xml_node_t>>(
        new gasxml_node<xml_node_t>(
        xml_node_t(NODE_T_ROOT, root_node_name.c_str())));
    tree_traversal(xml_root_node_.get(), gas_root_node_.get());
  }

/**
  * \brief Обход ноды дерева
  * \param xml_nd - нода xml файла в реализации pugixml
  * \param gasxml_nd - соответствующая нода в gas_tree(class XMLReader)
  */
  void tree_traversal(pugi::xml_node *xml_nd,
      gasxml_node<xml_node_t> *gasxml_nd) {
    if (!is_leafnode(xml_nd->name())) {
      init_subnode(xml_nd, gasxml_nd);
    } else {
      init_leafnode(xml_nd, gasxml_nd);
    }
  }
  
/**
  * \brief Инициализация внутренней ноды
  * \param xml_nd - нода xml файла в реализации pugixml
  * \param gasxml_nd - соответствующая нода в gas_tree(class XMLReader)
  */
  void init_subnode(pugi::xml_node *xml_nd,
      gasxml_node<xml_node_t> *gasxml_nd) {
    pugi::xml_node nx_xml_nd = xml_nd->first_child();
    node_type nt = xml_node_t::get_node_type(nx_xml_nd.name());
    if (nt == NODE_T_UNDEFINED) {
      std::string estr = "get undefined xml node while parsing, node name: "
          + std::string(nx_xml_nd.name()) + "\n";
      error_ = set_error_message(ERR_INIT_T | ERR_FILE_IN_ST, estr.c_str());
      return;
    }
    gasxml_nd->first_child = std::unique_ptr<gasxml_node<xml_node_t>>(
        new gasxml_node<xml_node_t>(xml_node_t(nt,
        nx_xml_nd.attribute("name").value())));
    tree_traversal(&nx_xml_nd, gasxml_nd->first_child.get());
    while (true) {
      if (nx_xml_nd == xml_nd->last_child())
        break;
      nx_xml_nd = nx_xml_nd.next_sibling();
      nt = xml_node_t::get_node_type(nx_xml_nd.name());
      gasxml_nd->first_child->siblings.push_back(
         std::unique_ptr<gasxml_node<xml_node_t>>(new gasxml_node<xml_node_t>(
         xml_node_t(nt, nx_xml_nd.attribute("name").value()))));
      tree_traversal(&nx_xml_nd, gasxml_nd->first_child->siblings.back().get());
    }
  }
  
/**
  * \brief Инициализация внешней ноды(листа)
  * \param xml_nd - нода xml файла в реализации pugixml
  * \param gasxml_nd - соответствующая нода в gas_tree(class XMLReader)
  */
  void init_leafnode(pugi::xml_node *&xml_nd,
      gasxml_node<xml_node_t> *gasxml_nd) {
    gasxml_nd->SetValue(xml_nd->text().as_string());
    gasxml_nd->SetName(xml_nd->attribute("name").value());
    gasxml_nd->first_child = nullptr;
  }

/**
  * \brief Проверить является ли нода листом
  * \param - имя pugi ноды
  */
  inline bool is_leafnode(const char *node_name) {
    return (strncmp(node_name, "parameter", strlen("parameter")) == 0);
  }

public:
  static XMLReader<xml_node_t> *Init(std::string gas_xml_file) {
    merror_t err = ERR_SUCCESS_T;
    if (gas_xml_file.empty())
      err = set_error_code(ERR_INIT_T | ERR_INIT_NULLP_ST);
    if (err == ERR_SUCCESS_T) {
      // check file exist
      FILE *f = fopen(gas_xml_file.c_str(), "r");
      if (!f)
        err = set_error_message(ERR_FILEIO_T | ERR_FILE_IN_ST, std::string(
            std::string("cannot open file: ") + gas_xml_file).c_str());
      else
        fclose(f);
    }
    reset_error();
    XMLReader<xml_node_t> *reader = new XMLReader<xml_node_t>(gas_xml_file);
    if (reader)
      if (reader->GetError() != ERR_SUCCESS_T) {
        Logging::Append(io_loglvl::debug_logs, get_error_message());
        delete reader;
        reader = nullptr;
      }
    return reader;
  }

  merror_t GetError() {
    return error_;
  }

  std::string GetRootName() {
    return gas_root_node_->GetName();
  }
  
  merror_t GetValueByPath(const std::vector<std::string> &xml_path,
      std::string *outstr) {
    const gasxml_node<xml_node_t> *tmp_gas_node = gas_root_node_.get();
    for (const auto &x : xml_path) {
      // check ypuself and sublings
      tmp_gas_node = tmp_gas_node->search_child_by_name(x);
      if (!tmp_gas_node) {
    #if defined (_DEBUG_SUBROUTINS)
        std::string strpath = "";
        for (const auto &strnode : xml_path)
          strpath += strnode + " --> ";
        Logging::Append(io_loglvl::debug_logs, strpath.c_str());
    #endif  // _DEBUG_SUBROUTINS
        return error_ = XML_LAST_STRING;
      }
    }
    *outstr = tmp_gas_node->GetValue();
    return ERR_SUCCESS_T;
  } 
};
#endif  // !_CORE__SUBROUTINS__XML_READER_H_
