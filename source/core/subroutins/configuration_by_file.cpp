/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "configuration_by_file.h"
#include "configuration_strtpl.h"

#include <set>

#include <assert.h>

namespace  {
std::set<std::string> config_params = std::set<std::string> {
    STRTPL_CONFIG_DEBUG_MODE, STRTPL_CONFIG_RK_SOAVE_MOD,
    STRTPL_CONFIG_PR_BINARYCOEFS, STRTPL_CONFIG_INCLUDE_ISO_20765,
    STRTPL_CONFIG_LOG_LEVEL, STRTPL_CONFIG_LOG_FILE, STRTPL_CONFIG_DATABASE
};
std::set<std::string> config_database = std::set<std::string> {
    STRTPL_CONFIG_DB_DRY_RUN, STRTPL_CONFIG_DB_CLIENT, STRTPL_CONFIG_DB_NAME,
    STRTPL_CONFIG_DB_USERNAME, STRTPL_CONFIG_DB_PASSWORD, STRTPL_CONFIG_DB_HOST,
    STRTPL_CONFIG_DB_PORT
};
}  // unnamed namespace

ConfigurationByFile::ConfigurationByFile(
    XMLReader<config_node> *xml_doc) : xml_doc_(xml_doc) {
  if (xml_doc) {
    init_parameters();
  } else {
    error_.SetError(ERROR_INIT_T,
        "Инициализация XMLReader для файла конфигурации");
    error_.LogIt();
  }
}

ConfigurationByFile *ConfigurationByFile::Init(
    const std::string &filename) {
  XMLReader<config_node> *xml_doc = XMLReader<config_node>::Init(filename);
  if (xml_doc == nullptr)
    return nullptr;
  return new ConfigurationByFile(xml_doc);
}

merror_t ConfigurationByFile::init_parameters() {
  merror_t error = ERROR_SUCCESS_T;
  std::vector<std::string> xml_path(1);
  std::string tmp_str = "";
  for (const auto &param : config_params) {
    xml_path[0] = param;
    if (param == STRTPL_CONFIG_DATABASE) {
      error = init_dbparameters();
    } else {
      xml_doc_->GetValueByPath(xml_path, &tmp_str);
      error = configuration_.SetConfigurationParameter(param, tmp_str);
    }
    if (error)
      break;
  }
  if (error) {
    error_.SetError(error,
        "Error during configfile reading: " + xml_path[0]);
    error_.LogIt();
  }
  return error;
}

merror_t ConfigurationByFile::init_dbparameters() {
  merror_t error = ERROR_SUCCESS_T;
  std::vector<std::string> xml_path = std::vector<std::string> {
      STRTPL_CONFIG_DATABASE, ""};
  std::string tmp_str = "";
  for (const auto &param : config_database) {
    xml_path[1] = param;
    xml_doc_->GetValueByPath(xml_path, &tmp_str);
    error = db_parameters_.SetConfigurationParameter(param, tmp_str);
    if (error)
      break;
  }
  if (error) {
    error_.SetError(error,
        "Ошибка обработки параметра файла конфигурации БД: " + xml_path[1]);
    error_.LogIt();
  }
  return error;
}

models_configuration ConfigurationByFile::GetConfiguration() const {
  return configuration_;
}

db_parameters ConfigurationByFile::GetDBConfiguration() const {
  return db_parameters_;
}

const ErrorWrap &ConfigurationByFile::GetErrorWrap() const {
  return error_;
}
