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
  models_configuration GetConfiguration() const;
  db_parameters GetDBConfiguration() const;

private:
  ConfigurationByFile(XMLReader<config_node> *xml_doc);
  void init_parameters();
  void init_dbparameters();

private:
  std::unique_ptr<XMLReader<config_node>> xml_doc_;
  models_configuration config_;
  db_parameters db_params_;
};

#endif  // !_CORE__SUBROUTINS__CONFIGURATION_BY_FILE_H_
