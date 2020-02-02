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
  ~ComponentByFile();

private:
  ComponentByFile(XMLReader<gas_node> *xml_doc);
  void set_gas_name();

private:
  XMLReader<gas_node> *xml_doc_;
  gas_t gas_name_;
};

#endif  // !_CORE__SUBROUTINS__GAS_BY_FILE_H_
