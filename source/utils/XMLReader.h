/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__XMLREADER_H
#define UTILS__XMLREADER_H

#include "common.h"
#include "file_structs.h"
#include "ErrorWrap.h"
#include "Logging.h"

// pugixml library (http://pugixml.org).
// pugixml is Copyright (C) 2006-2018 Arseny Kapoulkine.
#include "pugixml.hpp"

#include <memory>
#include <string>
#include <vector>

#include <stdint.h>
#include <string.h>


template <class xml_node_t>
class gasxml_node {
  typedef std::unique_ptr<gasxml_node<xml_node_t>> xml_node_ptr;
  typedef std::vector<xml_node_ptr> sibling_vec;
  
public:
  std::unique_ptr<gasxml_node<xml_node_t>> first_child;
  sibling_vec siblings;
  xml_node_t node;
              
public:
  gasxml_node(xml_node_t node)
    : node(node) {}
  
  const gasxml_node<xml_node_t> *ChildByName(
      const std::string &name) const {
    gasxml_node<xml_node_t> *child = nullptr;
    if (first_child != nullptr) {
      if (first_child->GetName() == name) {
        child = first_child.get();
      } else {
    #if defined(READERS_TEST)
        Logging::Append(io_loglvl::debug_logs, "Search for " +
            name + " but get %s" + first_child->GetName());
    #endif  // READERS_TEST
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

/* todo: не игнорировать рутноду, передавать вектор с ней */
/** \brief Класс парсинга xml файлов */
template <class node_t>
class XMLReader {
public:
  static XMLReader<node_t> *Init(std::string gas_xml_file) {
    merror_t err = ERROR_SUCCESS_T;
    XMLReader<node_t> *reader = nullptr;
    if (gas_xml_file.empty()) {
      err = ERROR_INIT_NULLP_ST;
      Logging::Append(err, "имя xml-файла пусто");
    }
    if (err == ERROR_SUCCESS_T) {
      if (!is_exist(gas_xml_file)) {
        err = ERROR_FILE_IN_ST;
        Logging::Append(err, std::string(
            std::string("cannot open file: ") + gas_xml_file).c_str());
      } else {
        reader = new XMLReader<node_t>(gas_xml_file);
        if (reader) {
          if (reader->GetErrorCode() != ERROR_SUCCESS_T) {
            Logging::Append(io_loglvl::debug_logs, reader->GetErrorMessage());
            delete reader;
            reader = nullptr;
          }
        }
      }
    }
    return reader;
  }

  static std::string GetFilenameExtension() {
    return ".xml";
  }

  /* todo: remove(replace with GetSource()) */
  std::string GetFileName() const {
    return gas_xml_file_;
  }

  merror_t GetErrorCode() const {
    return error_.GetErrorCode();
  }

  /* todo: remove(replace with LogError()) */
  std::string GetErrorMessage() const {
    return error_.GetMessage();
  }

  /* todo: remove it */
  std::string GetRootName() const {
    return gas_root_node_->GetName();
  }
  /** \brief Получить параметр по переданному пути
    * \note Функция обобщённого обхода
    * \warning outstr придёт с пробелами, если они есть в xml,
    *   алсо путь принимается без рут ноды */
  merror_t GetValueByPath(const std::vector<std::string> &xml_path,
      std::string *outstr) const {
    const gasxml_node<node_t> *tmp_gas_node = gas_root_node_.get();
    if (!tmp_gas_node)
      return ERROR_SUCCESS_T;
    for (const auto &x : xml_path) {
      // check ypuself and sublings
      tmp_gas_node = tmp_gas_node->ChildByName(x);
      if (!tmp_gas_node) {
    #if defined(READERS_TEST)
        std::string strpath = "";
        for (const auto &strnode : xml_path)
          strpath += strnode + " --> ";
        Logging::Append(io_loglvl::debug_logs, strpath);
    #endif  // READERS_TEST
        return FILE_LAST_OBJECT;
      }
    }
    *outstr = tmp_gas_node->GetValue();
    return ERROR_SUCCESS_T;
  }

private:
  XMLReader(std::string gas_xml_file)
    : error_(ERROR_SUCCESS_T), gas_xml_file_(gas_xml_file),
      gas_root_node_(nullptr) {
    xml_doc_.load_file(gas_xml_file.c_str());
    xml_root_node_ = std::unique_ptr<pugi::xml_node>(
        new pugi::xml_node());
    std::string root_node_name = node_t::get_root_name();
    *xml_root_node_ = xml_doc_.child(root_node_name.c_str());
    gas_root_node_  = std::unique_ptr<gasxml_node<node_t>>(
        new gasxml_node<node_t>(
        node_t(NODE_T_ROOT, root_node_name.c_str())));
    tree_traversal(xml_root_node_.get(), gas_root_node_.get());
  }
/** \brief Обход ноды дерева
  * \param xml_nd - нода xml файла в реализации pugixml
  * \param gasxml_nd - соответствующая нода в gas_tree(class XMLReader)
  */
  void tree_traversal(pugi::xml_node *xml_nd,
      gasxml_node<node_t> *gasxml_nd) {
    if (!is_leafnode(xml_nd)) {
      init_subnode(xml_nd, gasxml_nd);
    } else {
      init_leafnode(xml_nd, gasxml_nd);
    }
  }
/** \brief Инициализация внутренней ноды
  * \param xml_nd - нода xml файла в реализации pugixml
  * \param gasxml_nd - соответствующая нода в gas_tree(class XMLReader)
  */
  void init_subnode(pugi::xml_node *pugi_nd,
      gasxml_node<node_t> *gasxml_nd) {
    pugi::xml_node nx_xml_nd = pugi_nd->first_child();
    node_type nt = node_t::get_node_type(nx_xml_nd.name());
    if (nt == NODE_T_UNDEFINED) {
      std::string estr = "get undefined xml node while parsing, node name: "
          + std::string(nx_xml_nd.name()) + "\n";
      error_.SetError(ERROR_FILE_IN_ST, estr);
      return;
    }
    gasxml_nd->first_child = std::unique_ptr<gasxml_node<node_t>>(
        new gasxml_node<node_t>(node_t(nt,
        nx_xml_nd.attribute("name").value())));
    tree_traversal(&nx_xml_nd, gasxml_nd->first_child.get());
    while (true) {
      if (nx_xml_nd == pugi_nd->last_child())
        break;
      nx_xml_nd = nx_xml_nd.next_sibling();
      nt = node_t::get_node_type(nx_xml_nd.name());
      gasxml_nd->first_child->siblings.push_back(
         std::unique_ptr<gasxml_node<node_t>>(new gasxml_node<node_t>(
         node_t(nt, nx_xml_nd.attribute("name").value()))));
      tree_traversal(&nx_xml_nd, gasxml_nd->first_child->siblings.back().get());
    }
  }
/** \brief Инициализация внешней ноды(листа)
  * \param xml_nd - нода xml файла в реализации pugixml
  * \param gasxml_nd - соответствующая нода в gas_tree(class XMLReader)
  */
  void init_leafnode(pugi::xml_node *&xml_nd,
      gasxml_node<node_t> *gasxml_nd) {
    gasxml_nd->SetValue(xml_nd->text().as_string());
    gasxml_nd->SetName(xml_nd->attribute("name").value());
    gasxml_nd->first_child = nullptr;
  }
/** \brief Проверить является ли нода листом
  * \param - имя pugi ноды
  */
  bool is_leafnode(pugi::xml_node *xml_nd /*const char *node_name*/) {
    return (strncmp(xml_nd->name(), "parameter", strlen("parameter")) == 0);
  }

private:
  ErrorWrap error_;
  /* todo: свапнуть стрингу на урл */
  /** \brief имя xml файла */
  std::string gas_xml_file_;
  /** \brief корень дерева из обёрток над классом-параметром xml_node_t */
  std::unique_ptr<gasxml_node<node_t>> gas_root_node_;
  /** \brief pugi интерпретация документа */
  pugi::xml_document xml_doc_;
  /** \brief корень pugi дерева элементов */
  std::unique_ptr<pugi::xml_node> xml_root_node_;
};

#endif  // !UTILS__XMLREADER_H
