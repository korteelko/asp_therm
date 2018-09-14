#ifndef _CORE__SUBROUTINS__FILE_READING_H_
#define _CORE__SUBROUTINS__FILE_READING_H_

#include "gas_description.h"
#include "gas_mix_init.h"
#include "xmlreader.h"

#include <memory>
#include <string>
#include <vector>

struct gas_mix_file {
  const char *filename;
  const double part;
};

bool operator< (const gas_mix_file &lg, const gas_mix_file &rg);

/// class init gas_patameters by_file
class XmlFile {
  XmlFile(const XmlFile &) = delete;
  XmlFile &operator=(const XmlFile &) = delete;

private:
  XMLReader *xml_doc_;

private:
  XmlFile(XMLReader *xml_doc);

public:
  static XmlFile *Init(const std::string filename);

  std::unique_ptr<const_parameters> GetConstParameters();
  std::unique_ptr<dyn_parameters> GetDynParameters();
  // get formula and image url
  // XXX GetData();
  ~XmlFile();
};

class GasMix {
  GasMix(const GasMix &) = delete;
  GasMix &operator=(const GasMix &) = delete;

private:
  GasMix(std::vector<gas_mix_file> parts);

  int check_input(const std::vector<gas_mix_file> &parts);
  const_dyn_parameters *init_pars(const char *filename);

public:
  static GasMix *Init(const std::vector<gas_mix_file> &parts);

  std::unique_ptr<const_parameters> GetConstParameters();
  std::unique_ptr<dyn_parameters> GetDynParameters();
};

#endif  // _CORE__SUBROUTINS__FILE_READING_H_
