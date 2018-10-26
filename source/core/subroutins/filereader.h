#ifndef _CORE__SUBROUTINS__FILE_READING_H_
#define _CORE__SUBROUTINS__FILE_READING_H_

#include "gas_description.h"
#include "gas_mix_init.h"
#include "xmlreader.h"

#include <memory>
#include <string>
#include <vector>

class XmlFile {
  XmlFile(const XmlFile &) = delete;
  XmlFile &operator=(const XmlFile &) = delete;

private:
  XMLReader *xml_doc_;

private:
  XmlFile(XMLReader *xml_doc);

public:
  static XmlFile *Init(const std::string filename);
  std::shared_ptr<const_parameters> GetConstParameters();
  std::shared_ptr<dyn_parameters> GetDynParameters();
  ~XmlFile();
};

class GasMix {
  GasMix(const GasMix &) = delete;
  GasMix &operator=(const GasMix &) = delete;

private:
  bool is_valid_;
  std::shared_ptr<parameters_mix> prs_mix_;

private:
  static int check_input(const std::vector<gas_mix_file> &parts);
  GasMix(const std::vector<gas_mix_file> &parts);
  std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
      init_pars(const std::string filename);

public:
  static GasMix *Init(const std::vector<gas_mix_file> &parts);
  std::shared_ptr<parameters_mix> GetParameters();
};

#endif  // !_CORE__SUBROUTINS__FILE_READING_H_
