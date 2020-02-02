#include "configuration_by_file.h"

#include <array>

#include <assert.h>

namespace  {
std::array<std::string, 5> config_params = {
  "debug_mode", "pseudocritical", "include_iso_20765",
  "log_level", "database"
};

std::array<std::string, 6> config_database = {
  "client", "name", "username", "password", "host", "port"
};

std::string database_group = "database";
}  // unnamed namespace

ConfigurationByFile::ConfigurationByFile(XMLReader<config_node> *xml_doc)
  : xml_doc_(xml_doc) {
  if (xml_doc)
    init_parameters();
}

ConfigurationByFile *ConfigurationByFile::Init(
    const std::string &filename) {
  XMLReader<config_node> *xml_doc = XMLReader<config_node>::Init(filename);
  if (xml_doc == nullptr)
    return nullptr;
  return new ConfigurationByFile(xml_doc);
}

void ConfigurationByFile::init_parameters() {
  assert(0);
  std::vector<std::string> xml_path(1);
  for(const auto &param: config_params) {
    xml_path[0] = param;
  }
  //for (size_t i = 0; i < config_params.size(); ++i) {
  //  xml_path[0] = config_params[i];
  //}
}

void ConfigurationByFile::init_dbparameters() {
  assert(0);
  std::vector<std::string> xml_path(2);
}

models_configuration ConfigurationByFile::GetConfiguration() const {
  return config_;
}

db_parameters ConfigurationByFile::GetDBConfiguration() const {
  return db_params_;
}
