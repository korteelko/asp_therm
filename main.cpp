#include "target_sys.h"

#include "gas_by_file.h"
#include "gasmix_by_file.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"
#include "models_creator.h"
#include "xml_reader.h"

#include <vector>

#include <assert.h>

#if defined (_OS_NIX)
#  include <unistd.h>
#elif defined(_OS_WIN)
#  include <direct.h>
#  define getcwd _getcwd
#endif  // _OS

/// gas example input
///  gas   |vol,m^3/kg| p,Pa*10^7 |  T,K  |molmass,kg/mol| adiabat,1| cv, J/(kg*K)| acentric factor,1
/// methan  0.00617     4.641       190.66     16.043       ~1.3      ~1750           0.011
/// ethane  0.004926    4.871       305.33     30.07        ~1.22     ~1750           0.089
/// propane 0.004545    4.255       369.9      44.097       ~1.13     ~1750           0.153

// #define RK2_TEST
// #define PR_TEST
#define NG_GOST_TEST

#define INPUT_P_T  3000000, 350
#define NEW_PARAMS 500000, 250

#ifdef _IDE_VSCODE
const std::string xml_path = "/../asp_therm/data/gases/";
#else
const std::string xml_path = "/../../asp_therm/data/gases/";
#endif  // IDE_VSCODE
const std::string xml_methane = "methane.xml";
const std::string xml_ethane  = "ethane.xml";
const std::string xml_propane = "propane.xml";

const std::string xml_gasmix = "gasmix_inp_example.xml";

int test_models() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::vector<std::unique_ptr<modelGeneral>> test_vec;
  std::string filename = std::string(cwd) + xml_path + xml_gasmix;
#if defined(RK2_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rg_model_t::REDLICH_KWONG2, filename,
      INPUT_P_T)));
#endif  // RK2_TEST
#if defined(PR_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rg_model_t::PENG_ROBINSON, filename,
      INPUT_P_T)));
#endif  // PR_TEST
#if defined(NG_GOST_TEST)
  ng_gost_mix ngg = ng_gost_mix {
      ng_gost_component{GAS_TYPE_METHANE, 0.965},
      ng_gost_component{GAS_TYPE_ETHANE, 0.018},
      ng_gost_component{GAS_TYPE_PROPANE, 0.0045},
      ng_gost_component{GAS_TYPE_N_BUTANE, 0.001},
      ng_gost_component{GAS_TYPE_ISO_BUTANE, 0.001},
      ng_gost_component{GAS_TYPE_N_PENTANE, 0.0003},
      ng_gost_component{GAS_TYPE_ISO_PENTANE, 0.0005},
      ng_gost_component{GAS_TYPE_HEXANE, 0.0007},
      ng_gost_component{GAS_TYPE_NITROGEN, 0.003},
      ng_gost_component{GAS_TYPE_CARBON_DIOXIDE, 0.006}
  };
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(
      rg_model_t::NG_GOST, ngg, INPUT_P_T)));
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(
      rg_model_t::NG_GOST, filename, INPUT_P_T)));
#endif  // NG_GOST_TEST
  for (auto calc_mod = test_vec.rbegin();
      calc_mod != test_vec.rend(); calc_mod++) {
    if (*calc_mod == nullptr) {
      std::cerr << "        --------------          " << std::endl;
      continue;
    }
    std::cerr << (*calc_mod)->ConstParametersString() << std::flush;
    std::cerr << modelGeneral::sParametersStringHead() << std::flush;
    std::cerr << (*calc_mod)->ParametersString() << std::flush;
    std::cerr << "Now we will set volume for p=30e6, t=300 \n" << std::flush;
    (*calc_mod)->SetVolume(NEW_PARAMS);
    std::cerr << (*calc_mod)->ParametersString() << std::flush;
  }
  return 0;
}

int test_models_mix() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::vector<std::unique_ptr<modelGeneral>> test_vec;
  std::vector<gasmix_file> xml_files = std::vector<gasmix_file> {
    gasmix_file(std::string(cwd) + xml_path + xml_methane, 0.988),
    // add more (summ = 1.00)
    gasmix_file(std::string(cwd) + xml_path + xml_ethane, 0.009),
    gasmix_file(std::string(cwd) + xml_path + xml_propane, 0.003)
  };
#if defined(RK2_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rg_model_t::REDLICH_KWONG2, xml_files,
      INPUT_P_T)));
#endif  // RK2_TEST
#if defined(PR_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rg_model_t::PENG_ROBINSON, xml_files,
      INPUT_P_T)));
#endif  // PR_TEST
#if defined(NG_GOST_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(
      rg_model_t::NG_GOST, xml_files, INPUT_P_T)));
#endif  // NG_GOST_TEST
  for (auto calc_mod = test_vec.rbegin();
      calc_mod != test_vec.rend(); calc_mod++) {
    if (*calc_mod == nullptr) {
      std::cerr << "        --------------          " << std::endl;
      continue;
    }
    std::cerr << (*calc_mod)->ConstParametersString() << std::flush;
    std::cerr << modelGeneral::sParametersStringHead() << std::flush;
    std::cerr << (*calc_mod)->ParametersString() << std::flush;
    std::cerr << "Now we will set new params \n" << std::flush;
    (*calc_mod)->SetVolume(NEW_PARAMS);
    std::cerr << (*calc_mod)->ParametersString() << std::flush;
  }
  return 0;
}

int main() {
  if (test_models()) {
    std::cerr << "test_models()" << get_error_message();
    return 1;
  }
  std::cerr << "\n\n\n" << get_error_message();
  if (test_models_mix()) {
    std::cerr << "test_models_mix()" << get_error_message();
    return 2;
  }
  return 0;
}
