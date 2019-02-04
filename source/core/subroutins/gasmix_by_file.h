#ifndef _CORE__SUBROUTINS__GASMIX_BY_FILE_H_
#define _CORE__SUBROUTINS__GASMIX_BY_FILE_H_

#include "gas_by_file.h"
#include "gas_description.h"
#include "gasmix_init.h"
#include "xml_reader.h"

#include <memory>
#include <string>
#include <vector>

class GasMixByFiles;

class GasMixComponentsFile {
  GasMixComponentsFile(const GasMixComponentsFile &) = delete;
  GasMixComponentsFile &operator=(const GasMixComponentsFile &) = delete;
  
private:
  std::unique_ptr<GasMixByFiles> files_handler_;
  XMLReader<gasmix_node> *xml_doc_;
  std::vector<gasmix_file> gasmix_files_;

private:
  GasMixComponentsFile(XMLReader<gasmix_node> *xml_doc);
  void init_components();
  
public:
  static GasMixComponentsFile *Init(const std::string &filename);
  std::shared_ptr<parameters_mix> GetParameters();
  
  ~GasMixComponentsFile();
};

class GasMixByFiles {
  GasMixByFiles(const GasMixByFiles &) = delete;
  GasMixByFiles &operator=(const GasMixByFiles &) = delete;

private:
  bool is_valid_;
  std::shared_ptr<parameters_mix> prs_mix_;

private:
  static int check_input(const std::vector<gasmix_file> &parts);
  GasMixByFiles(const std::vector<gasmix_file> &parts);
  std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
      init_pars(const std::string &filename);

public:
  static GasMixByFiles *Init(const std::vector<gasmix_file> &parts);
  std::shared_ptr<parameters_mix> GetParameters();
};

#endif  // !_CORE__SUBROUTINS__GASMIX_BY_FILE_H_
