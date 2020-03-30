/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__CONFIGURATION_BY_FILE_H_
#define _CORE__SUBROUTINS__CONFIGURATION_BY_FILE_H_

#include "db_connection.h"
#include "models_configurations.h"
#include "xml_reader.h"

#include <memory>
#include <string>

/// класс загрузки файла конфигурации программы
class ConfigurationByFile {
  ConfigurationByFile(const ConfigurationByFile &) = delete;
  ConfigurationByFile &operator= (const ConfigurationByFile &) = delete;

public:
  static ConfigurationByFile *Init(const std::string &filename);

  program_configuration GetConfiguration() const;
  db_parameters GetDBConfiguration() const;
  const ErrorWrap &GetErrorWrap() const;

private:
  ConfigurationByFile(XMLReader<config_node> *xml_doc);
  merror_t init_parameters();
  merror_t init_dbparameters();

private:
  ErrorWrap error_;
  std::unique_ptr<XMLReader<config_node>> xml_doc_;
  program_configuration configuration_;
  db_parameters db_parameters_;
};

#endif  // !_CORE__SUBROUTINS__CONFIGURATION_BY_FILE_H_
