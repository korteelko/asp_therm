#include "xmlreader.h"

#include "models_errors.h"

#include <map>

#include <assert.h>
#include <stdio.h>
#include <string.h>

// gas_node
gas_node::gas_node(int itype, std::string name)
  : gas_node_t(itype), name_(name), value_("") {}

gas_node::gas_node(int itype, std::string name, std::string value)
  : gas_node_t(itype), name_(name), value_(value) {}

gas_node::gas_node(std::string type, std::string name)
  : gas_node(type, name, "") {}

gas_node::gas_node(std::string type, std::string name, std::string value)
  : name_(name), value_(value) {
  if (type == "section")
    gas_node_t = GAS_NODE_T_SECTION;
  else if (type == "group")
    gas_node_t = GAS_NODE_T_GROUP;
  else if (type == "subgroup")
    gas_node_t = GAS_NODE_T_SUBGROUP;
  else if (type == "parameter")
    gas_node_t = GAS_NODE_T_PARAMETER;
  else
    gas_node_t = GAS_NODE_T_UNDEFINED;
}

const gas_node *gas_node::search_child_by_name(std::string name) const {
  if (first_child_ == nullptr)
    return nullptr;
  if (first_child_->name_ == name)
    return first_child_.get();
  for (const auto &x : first_child_->siblings_)
    if (x->name_ == name)
      return x.get();
  return nullptr;
}

// XMLReader
XMLReader::XMLReader(std::string gas_xml_file)
  : gas_xml_file_(gas_xml_file) {
  xml_parse_result_ = std::unique_ptr<pugi::xml_parse_result>(
      new pugi::xml_parse_result());
  *xml_parse_result_ = xml_doc_.load_file(gas_xml_file.c_str());
  xml_root_node_ = std::unique_ptr<pugi::xml_node>(
      new pugi::xml_node());
  *xml_root_node_ = xml_doc_.child("gas");
  gas_root_node_  = std::unique_ptr<gas_node>(new gas_node(
      GAS_NODE_T_ROOT, "gas"));
  tree_traversal(xml_root_node_.get(), gas_root_node_.get());
}

// static
XMLReader *XMLReader::Init(std::string gas_xml_file) {
  // check input data
  reset_error();
  if (gas_xml_file == "") {
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
  return new XMLReader(gas_xml_file);
}

// xml_nd - temp node of xml document
// gas_nd - corresponding node of gas_tree(class XMLReader)
void XMLReader::tree_traversal(pugi::xml_node *xml_nd, gas_node *gas_nd) {
  if (strncmp(xml_nd->name(), "parameter", strlen("parameter")) != 0) {
    init_subnode(xml_nd, gas_nd);
  } else {
    init_parameter(xml_nd, gas_nd);
  }
}

void XMLReader::init_subnode(pugi::xml_node *xml_nd, gas_node *gas_nd) {
  pugi::xml_node nx_xml_nd = xml_nd->first_child();
  gas_nd->first_child_ = std::unique_ptr<gas_node>(
      new gas_node(nx_xml_nd.name(), nx_xml_nd.attribute("name").value()));
  tree_traversal(&nx_xml_nd, gas_nd->first_child_.get());
  while (true) {
    if (nx_xml_nd == xml_nd->last_child())
      break;
    nx_xml_nd = nx_xml_nd.next_sibling();
    gas_nd->first_child_->siblings_.push_back(std::unique_ptr<gas_node>(
        new gas_node(nx_xml_nd.name(), nx_xml_nd.attribute("name").value())));
    tree_traversal(&nx_xml_nd, gas_nd->first_child_->siblings_.back().get());
  }
}

void XMLReader::init_parameter(pugi::xml_node *xml_nd, gas_node *gas_nd) {
  gas_nd->value_ = xml_nd->text().as_string();
  gas_nd->name_ = xml_nd->attribute("name").value();
  pugi::xml_node nx_xml_nd = xml_nd->first_child();
  gas_nd->first_child_ = std::unique_ptr<gas_node>(new gas_node(
      GAS_NODE_T_PARAMETER, nx_xml_nd.attribute("name").value(), 
      nx_xml_nd.text().as_string()));
  while (true) {
    if (nx_xml_nd == xml_nd->last_child())
      break;
    nx_xml_nd = nx_xml_nd.next_sibling();
    gas_nd->first_child_->siblings_.push_back(
      std::unique_ptr<gas_node>(new gas_node(GAS_NODE_T_PARAMETER,
          nx_xml_nd.attribute("name").value(), nx_xml_nd.text().as_string())));
  }
}

int XMLReader::GetValueByPath(const std::vector<std::string> &xml_path,
    std::string *outstr) const {
  const gas_node *tmp_gas_node = gas_root_node_.get();
  for (const auto &x : xml_path) {
    // check ypuself and sublings
    tmp_gas_node = tmp_gas_node->search_child_by_name(x);
    if (!tmp_gas_node)
      return 1;
  }
  *outstr = tmp_gas_node->value_;
  return 0;
}
