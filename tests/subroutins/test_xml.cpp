/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "test_xml.h"

#include "common.h"
#include "gas_by_file.h"
#include "gasmix_by_file.h"
#include "xml_reader.h"

#include <iostream>
#include <memory>
#include <string>

#include <assert.h>
#include <stdio.h>
//#ifdef _NIX
  #include <unistd.h>
//#endif  // _NIX

static const std::string xml_path = "/../../asp_therm/data/gases/";
static const std::string xml_methane = "methane.xml";
static const std::string xml_gasmix = "../gasmix_inp_example.xml";
static char cwd[512] = {0};

int test_component_init() {
  std::string methane_path = xml_path + xml_methane;
  methane_path = std::string(cwd) + methane_path;
  std::unique_ptr<ComponentByFile> met_xml(
      ComponentByFile::Init(methane_path));
  if (met_xml == nullptr) {
    std::cerr << "Object of XmlFile wasn't created\n";
    return 1;
  }
  auto cp = met_xml->GetConstParameters();
  auto dp = met_xml->GetDynParameters();
  if (cp == nullptr) {
    std::cerr << "const_parameters by xml wasn't created\n";
    return 2;
  }
  if (dp == nullptr) {
    std::cerr << "dyn_parameters by xml wasn't created\n";
    return 2;
  }
  return 0;
}

int test_components_init() {
  std::string gasmix_path = xml_path + xml_gasmix;
  gasmix_path = std::string(cwd) + gasmix_path;
  std::unique_ptr<GasMixComponentsFile> gasmix_comps(
      GasMixComponentsFile::Init(rg_model_t::PENG_ROBINSON, gasmix_path));
  if (gasmix_comps == nullptr) {
    std::cerr << "cannot create xml_components handler\n";
    return 1;
  }
  auto components_parameters = gasmix_comps->GetMixParameters();
  if (components_parameters == nullptr) {
    std::cerr << "cannot initilize parameters of mix\n";
    std::cerr << get_error_message() << std::endl;
    return 1;
  }
  return 0;
}

int run_tests_xml() {
  if (!getcwd(cwd, sizeof(cwd))) {
    std::cerr << "cann't get current dir";
    return 2;
  }
  // int err = test_component_init();  // passed
  // err |= test_components_init();
  int err = test_components_init();
  return err;
}
