/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__GASMIX_BY_FILE_H_
#define _CORE__SUBROUTINS__GASMIX_BY_FILE_H_

#include "common.h"
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

public:
  static GasMixComponentsFile *Init(
      rg_model_t mn, const std::string &filename);
  std::shared_ptr<parameters_mix> GetMixParameters();
  std::shared_ptr<ng_gost_mix> GetGostMixParameters();

private:
  GasMixComponentsFile(rg_model_t mn, XMLReader<gasmix_node> *xml_doc);
  void init_components();
  void setup_gasmix_files(const std::string &gasmix_dir);
  void setup_gost_mix();

private:
  std::unique_ptr<XMLReader<gasmix_node>> xml_doc_;
  /* maybe remove files_handler_ */
  std::unique_ptr<GasMixByFiles> files_handler_;
  /* containers
   * UPD: lol, we can recalculate gasmix_files_ by gost model */
  std::vector<gasmix_file> gasmix_files_;
  rg_model_t model_t_;
  ErrorWrap error_;
};


class GasMixByFiles {
  GasMixByFiles(const GasMixByFiles &) = delete;
  GasMixByFiles &operator=(const GasMixByFiles &) = delete;

public:
  static GasMixByFiles *Init(const std::vector<gasmix_file> &parts);
  std::shared_ptr<parameters_mix> GetMixParameters();
  std::shared_ptr<ng_gost_mix> GetGostMixParameters();

private:
  static merror_t check_input(const std::vector<gasmix_file> &parts);
  GasMixByFiles(const std::vector<gasmix_file> &parts);
  std::pair<std::shared_ptr<const_parameters>, std::shared_ptr<dyn_parameters>>
      init_pars(double part, const std::string &filename);

public:
  static ErrorWrap init_error;

private:
  std::shared_ptr<parameters_mix> prs_mix_;
  std::shared_ptr<ng_gost_mix> gost_mix_;
  ErrorWrap error_;
  bool is_valid_;
};

#endif  // !_CORE__SUBROUTINS__GASMIX_BY_FILE_H_
