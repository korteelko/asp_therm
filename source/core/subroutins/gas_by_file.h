/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__GAS_BY_FILE_H_
#define _CORE__SUBROUTINS__GAS_BY_FILE_H_

#include "gas_description.h"
#include "gasmix_init.h"
#include "xml_reader.h"

#include <memory>
#include <string>

/*
 * taks maybe add json format
 */
class ComponentByFile {
  ComponentByFile(const ComponentByFile &) = delete;
  ComponentByFile &operator=(const ComponentByFile &) = delete;

public:
  static ComponentByFile *Init(const std::string &filename);
  std::shared_ptr<const_parameters> GetConstParameters();
  std::shared_ptr<dyn_parameters> GetDynParameters();

private:
  ComponentByFile(XMLReader<gas_node> *xml_doc);
  void set_gas_name();

private:
  std::unique_ptr<XMLReader<gas_node>> xml_doc_;
  gas_t gas_name_;
};

#endif  // !_CORE__SUBROUTINS__GAS_BY_FILE_H_
