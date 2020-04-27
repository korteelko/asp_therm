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
#include "inode_imp.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"
#include "models_configurations.h"
#include "models_creator.h"
#include "program_state.h"
#include "JSONReader.h"

#include <filesystem>
#include <vector>

#include <assert.h>


// #define JSON_READER_DEBUG
// #define MODELS_DEBUG
#define RK2_DEBUG
//#define PR_DEBUG  // доделать для смесей
#define RKS_DEBUG
//#define NG_GOST_DEBUG
#define DATABASE_DEBUG

#define INPUT_P_T  3000000, 350
#define NEW_PARAMS 500000, 250

namespace fs = std::filesystem;

// maybe set to:
// const fs::path xml_path = "../data/gases";
const fs::path xml_path = "../../asp_therm/data/gases";
const fs::path xml_methane = "methane.xml";
const fs::path xml_ethane  = "ethane.xml";
const fs::path xml_propane = "propane.xml";

const fs::path xml_gasmix = "../gasmix_inp_example.xml";
const fs::path xml_configuration = "../../configuration.xml";
const fs::path json_test = "../../tests/full/utils/data/test_json.json";

static model_str rk2_str(rg_model_id(rg_model_t::REDLICH_KWONG,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "debugi rk2");
static model_str rks_str(rg_model_id(rg_model_t::REDLICH_KWONG,
    MODEL_RK_SUBTYPE_SOAVE), 1, 0, "debugi rks");
static model_str pr_str(rg_model_id(rg_model_t::PENG_ROBINSON,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "debugi pr");

static fs::path cwd;


int test_json() {
  file_utils::FileURLRoot file_c(file_utils::SetupURL(
      file_utils::url_t::fs_path, cwd / xml_path));
  auto path =  file_c.CreateFileURL(json_test.string());
  std::unique_ptr<JSONReaderSample<json_test_node<>>> jr(
    JSONReaderSample<json_test_node<>>::Init(&path));
  if (jr) {
    if (!jr->GetErrorCode()) {
      jr->InitData();
      std::vector<std::string> path_emp;
      std::string res = "";
      if (jr->GetValueByPath(path_emp, &res))
        std::cerr << "vector of paths empty";
      /* вытянуть обычный параметр */
      std::vector<std::string> path_f = {"first", "t"};
      if (jr->GetValueByPath(path_f, &res))
        std::cerr << "vector of paths error";
      if (!res.empty()) {
        std::cerr << "On json path:";
        for (const auto &x : path_f)
          std::cerr << " " << x;
        std::cerr << "\nwe have: " << res << std::endl;
      }
    }
  } else {
    std::cerr << "bad init" << std::endl;
    return ERROR_GENERAL_T;
  }
  std::cerr << "JSON test finish succesfully\n";
  return ERROR_SUCCESS_T;
}

int test_database_with_models(DBConnectionManager &dbm) {
int res = 0;
  std::string filename = (cwd / xml_path / xml_gasmix).string();
#if defined(RK2_DEBUG)
  std::unique_ptr<modelGeneral> mk(
      ModelsCreator::GetCalculatingModel(rk2_str, filename, INPUT_P_T));
  model_info mi{.short_info = mk->GetModelShortInfo()};
  dbm.SaveModelInfo(mi);
  mi.initialized = mi.f_full;
  dbm.SaveModelInfo(mi);
  /* calculation info */
  // calculation_info ci;

#endif  // RK2_TEST
  return res;
}

int test_database() {
  std::string filename = (cwd / xml_path / xml_configuration).string();
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
  std::vector<db_table> tables { db_table::table_model_info,
      db_table::table_calculation_info, db_table::table_calculation_state_log };
  for (const auto &x : tables) {
    if (!dbm.IsTableExist(x)) {
      if (dbm.GetErrorCode())
        std::cerr << "\nerror ocurred for tableExist command #" << int(x);
      dbm.CreateTable(x);
      if (dbm.GetErrorCode())
        std::cerr << "\nerror ocurred for tableCreate command #" << int(x);
    }
  }
  if (st == STATUS_HAVE_ERROR) {
    std::cerr << "error during create table: "
        << dbm.GetErrorCode() << std::endl;
  }
  if (!ps.GetErrorCode())
    return test_database_with_models(dbm);
  return ps.GetErrorCode();
}

int test_program_configuration() {
  std::cerr << "test_program_configuration start\n";
  ProgramState &ps = ProgramState::Instance();
  ps.ReloadConfiguration((cwd / xml_path / xml_configuration).string());
  merror_t e = ps.GetErrorCode();
  if (e) {
    std::cerr << "program state bida " << e << std::endl;
    std::cerr << "Current dir is " << cwd << std::endl;
  }
  return e;
}

int test_models() {
  std::vector<std::unique_ptr<modelGeneral>> test_vec;
  std::string filename = (cwd / xml_path / xml_gasmix).string();
#if defined(RK2_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rk2_str, filename, INPUT_P_T)));
#endif  // RK2_TEST
#if defined(RKS_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rks_str, filename, INPUT_P_T)));
#endif  // RKS_TEST
#if defined(PR_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(pr_str, filename, INPUT_P_T)));
#endif  // PR_TEST
#if defined(NG_GOST_DEBUG)
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
  std::vector<std::unique_ptr<modelGeneral>> test_vec;
  std::vector<gasmix_file> xml_files = std::vector<gasmix_file> {
    gasmix_file("metane", (cwd / xml_path / xml_methane).string(), 0.988),
    // add more (summ = 1.00)
    gasmix_file("ethane", (cwd / xml_path / xml_ethane).string(), 0.009),
    gasmix_file("propane", (cwd / xml_path / xml_propane).string(), 0.003)
  };
#if defined(RK2_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rk2_str, xml_files, INPUT_P_T)));
#endif  // RK2_TEST
#if defined(RKS_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rks_str, xml_files, INPUT_P_T)));
#endif  // RKS_TEST
#if defined(PR_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(pr_str, xml_files, INPUT_P_T)));
#endif  // PR_TEST
#if defined(NG_GOST_DEBUG)
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

int main(int argc, char *argv[]) {
  cwd = fs::path(argv[0]).parent_path();
  fs::current_path(cwd);
  if (!test_program_configuration()) {
    Logging::Append(io_loglvl::debug_logs, "Запускаю тесты сборки");
    //*
  #if defined(MODELS_DEBUG)
    if (test_models()) return 1;
    if (test_models_mix()) return 2;
  #endif  // MODELS_TEST
  #if defined(DATABASE_DEBUG)
    if (test_database()) return 3;
  #endif  // DATABASE_TEST
  #if defined(JSON_READER_DEBUG)
    if (test_json()) return 4;
  #endif  // JSON_READER
    Logging::Append(io_loglvl::debug_logs, "Ни одной ошибки не заметил");
  //*/
  }

  return 0;
}
