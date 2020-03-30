/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "target_sys.h"

#include "db_connection_manager.h"
#include "gas_by_file.h"
#include "gasmix_by_file.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"
#include "models_configurations.h"
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

#define RK2_TEST
// #define PR_TEST  // доделать для смесей
#define RKS_TEST
// #define NG_GOST_TEST
// #define DATABASE_TEST

#define INPUT_P_T  3000000, 350
#define NEW_PARAMS 500000, 250

const std::string xml_path = "/../../asp_therm/data/gases/";
const std::string xml_methane = "methane.xml";
const std::string xml_ethane  = "ethane.xml";
const std::string xml_propane = "propane.xml";

const std::string xml_gasmix = "../gasmix_inp_example.xml";
const std::string xml_configuration = "../../configuration.xml";

static model_str rk2_str(rg_model_id(rg_model_t::REDLICH_KWONG,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "debugi rk2");
static model_str rks_str(rg_model_id(rg_model_t::REDLICH_KWONG,
    MODEL_RK_SUBTYPE_SOAVE), 1, 0, "debugi rks");
static model_str pr_str(rg_model_id(rg_model_t::PENG_ROBINSON,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "debugi pr");

int test_database() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::string filename = std::string(cwd) + xml_path + xml_configuration;
  ProgramState &ps = ProgramState::Instance();
  // merror_t err = ps.ResetConfigFile(filename);
  // if (err)
  //   std::cerr << "update config error: " << hex2str(err) << std::endl;
  // db_parameters p = ps.GetDatabaseConfiguration();
  DBConnectionManager dbm;
  dbm.ResetConnectionParameters(
      ps.GetDatabaseConfiguration());
  auto st = dbm.CheckConnection();
  if (st == STATUS_HAVE_ERROR) {
    std::cerr << "error during connection check: "
        << dbm.GetErrorCode() << std::endl;
    std::cerr << "  message: " << dbm.GetErrorMessage() << std::endl;
    return 1;
  }
  bool exists = false;
  std::cerr << " table model info exists: " <<
      (exists = dbm.IsTableExist(db_table::table_model_info)) << std::endl;
  if (!exists) {
    st = dbm.CreateTable(db_table::table_model_info);
  }
  if (st == STATUS_HAVE_ERROR) {
    std::cerr << "error during create table: "
        << dbm.GetErrorCode() << std::endl;
  }
  return ps.GetErrorCode();
}

int test_program_configuration() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::cerr << "test_program_configuration start\n";
  ProgramState &ps = ProgramState::Instance();
  ps.ReloadConfiguration(std::string(cwd) + xml_path + xml_configuration);
  merror_t e = ps.GetErrorCode();
  if (e)
    std::cerr << "program state bida " << e;
  return e;
}

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
      ModelsCreator::GetCalculatingModel(rk2_str, filename, INPUT_P_T)));
#endif  // RK2_TEST
#if defined(RKS_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rks_str, filename, INPUT_P_T)));
#endif  // RKS_TEST
#if defined(PR_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(pr_str, filename, INPUT_P_T)));
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
    std::cerr << "\n" << (*calc_mod)->GetModelShortInfo().short_info << "\n";
    std::cerr << (*calc_mod)->ConstParametersString() << std::flush;
    std::cerr << modelGeneral::sParametersStringHead() << std::flush;
    std::cerr << (*calc_mod)->ParametersString() << std::flush;
    std::cerr << "Now we will set new params\n" << std::flush;
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
    gasmix_file("metane", std::string(cwd) + xml_path + xml_methane, 0.988),
    // add more (summ = 1.00)
    gasmix_file("ethane", std::string(cwd) + xml_path + xml_ethane, 0.009),
    gasmix_file("propane", std::string(cwd) + xml_path + xml_propane, 0.003)
  };
#if defined(RK2_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rk2_str, xml_files, INPUT_P_T)));
#endif  // RK2_TEST
#if defined(RKS_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rks_str, xml_files, INPUT_P_T)));
#endif  // RKS_TEST
#if defined(PR_TEST)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(pr_str, xml_files, INPUT_P_T)));
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
    std::cerr << "\n" << (*calc_mod)->GetModelShortInfo().short_info << "\n";
    std::cerr << (*calc_mod)->ConstParametersString() << std::flush;
    std::cerr << modelGeneral::sParametersStringHead() << std::flush;
    std::cerr << (*calc_mod)->ParametersString() << std::flush;
    std::cerr << "Now we will set new params\n" << std::flush;
    (*calc_mod)->SetVolume(NEW_PARAMS);
    std::cerr << (*calc_mod)->ParametersString() << std::flush;
  }
  return 0;
}

int main() {
  if (!test_program_configuration()) {
    Logging::Append(io_loglvl::debug_logs, "Запускаю тесты сборки");
    //*
    if (test_models()) return 1;
    if (test_models_mix()) return 2;
  #if defined(DATABASE_TEST)
    if (test_database()) return 3;
  #endif  // DATABASE_TEST
    Logging::Append(io_loglvl::debug_logs, "Ни одной ошибки не заметил");
  //*/
  }

  return 0;
}
