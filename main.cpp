/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "atherm_db_tables.h"
#include "calculation_by_file.h"
#include "db_connection_manager.h"
#include "db_queries_setup.h"
#include "gas_by_file.h"
#include "gasmix_by_file.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"
#include "models_configurations.h"
#include "models_creator.h"
#include "program_state.h"

#include <filesystem>
#include <vector>

#include <assert.h>


// #define JSON_READER_DEBUG
// #define MODELS_DEBUG
#define RK2_DEBUG
// #define PR_DEBUG  // доделать для смесей
#define RKS_DEBUG
// #define NG_GOST_DEBUG
// #define DATABASE_DEBUG
#define CALCULATION_DEBUG

#define INPUT_P_T  3000000, 350
#define NEW_PARAMS 500000, 250

namespace fs = std::filesystem;

// maybe set to:
const fs::path xml_data_dir = "../data";
const fs::path xml_gases_dir = "../data/gases";
const fs::path xml_calculations_dir = "../data/calculation";
const fs::path xml_methane = "methane.xml";
const fs::path xml_ethane  = "ethane.xml";
const fs::path xml_propane = "propane.xml";

const fs::path xml_gasmix = "../gasmix_inp_example.xml";
const fs::path xml_calculation = "calculation/calculation_setup_iso.xml";
const fs::path xml_configuration = "../../configuration.xml";
const fs::path json_test = "../../tests/full/utils/data/test_json.json";

static model_str rk2_str(rg_model_id(rg_model_t::REDLICH_KWONG,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "debugi rk2");
static model_str rks_str(rg_model_id(rg_model_t::REDLICH_KWONG,
    MODEL_RK_SUBTYPE_SOAVE), 1, 0, "debugi rks");
static model_str pr_str(rg_model_id(rg_model_t::PENG_ROBINSON,
    MODEL_SUBTYPE_DEFAULT), 1, 0, "debugi pr");

static fs::path cwd;


/**
 * \brief Функция отладки считывания и инициализации
 *   входных данных для расчётов
 * */
int test_calculation_setup() {
  std::string data_dir = (cwd / xml_data_dir).string();
  std::shared_ptr<file_utils::FileURLRoot> root_shp(
      new file_utils::FileURLRoot(file_utils::url_t::fs_path, data_dir));
  calculation_setup cs(root_shp);
  CalculationSetupBuilder builder(&cs);

  auto path = root_shp->CreateFileURL(xml_calculation.string());
  std::unique_ptr<ReaderSample<pugi::xml_node, calculation_node<pugi::xml_node>,
      CalculationSetupBuilder>> rs(ReaderSample<pugi::xml_node,
      calculation_node<pugi::xml_node>, CalculationSetupBuilder>::Init(&path, &builder));
  return (rs) ? rs->InitData() : 11;
}

int test_calculation_init() {
  int res = 0;
  std::string data_dir = (cwd / xml_data_dir).string();
  std::shared_ptr<file_utils::FileURLRoot> root_shp(
      new file_utils::FileURLRoot(file_utils::url_t::fs_path, data_dir));

  /* todo: naming??? */
  CalculationSetup CS(root_shp, xml_calculation.string());
  if (!(res = CS.GetError())) {
    ProgramState &ps = ProgramState::Instance();
    AthermDBTables adb;
    DBConnectionManager dbm(&adb);
    // модели не дописаны
    CS.Calculate();
    dbm.ResetConnectionParameters(
        ps.GetDatabaseConfiguration());
    if (is_status_ok(dbm.CheckConnection()))
      CS.AddToDatabase(&dbm);
    else
      res = 12;
  }
  return res;
}


int test_database_with_models(DBConnectionManager &dbm) {
  int res = 0;
  std::string filename = (cwd / xml_gases_dir / xml_gasmix).string();
#if defined(RK2_DEBUG)
  std::unique_ptr<modelGeneral> mk(
      ModelsCreator::GetCalculatingModel(rk2_str, NULL, filename, INPUT_P_T));
  model_info mi {.short_info = mk->GetModelShortInfo()};
  // dbm.SaveModelInfo(mi);
  // todo: это стандартный сетап на добавление так что его можно
  //   сохранить и использовать
  mi.initialized = mi.f_full & (~mi.f_model_id);
  dbm.SaveSingleRow(mi, nullptr);

  /* select test */
  auto mi2 = model_info::GetDefault();
  mi2.short_info.model_type.type = rg_model_t::REDLICH_KWONG;
  mi2.short_info.model_type.subtype = MODEL_SUBTYPE_DEFAULT;
  mi2.initialized = model_info::f_model_type | model_info::f_model_subtype;
  std::vector<model_info> r;
  dbm.SelectRows(mi, &r);
  r.clear();

  mk.reset(ModelsCreator::GetCalculatingModel(rks_str, NULL, filename, INPUT_P_T));
  model_info mis {.short_info = mk->GetModelShortInfo()};
  // dbm.SaveModelInfo(mi);
  // todo: это стандартный сетап на добавление так что его можно
  //   сохранить и использовать
  mis.initialized = mis.f_full & (~mis.f_model_id);
  dbm.SaveSingleRow(mis, nullptr);

  dbm.SelectRows(mi2, &r);
  dbm.DeleteRows(mi2);

  std::string str = "Тестовая строка";
  model_info mi3 = model_info::GetDefault();
  mi3.short_info.short_info = str;
  mi3.initialized = mi.f_full & (~mi.f_model_id);
  dbm.SaveSingleRow(mi3, nullptr);

  /* select */
  std::vector<model_info> r1;
  dbm.SelectRows(mi3, &r1);

  /* add calculation_info */
  calculation_info ci;
  if (r1.size()) {
    ci.model_id = r1[0].id;
    ci.initialized = ci.f_model_id;
    ci.initialized |= ci.f_date | ci.f_time;
    dbm.SaveSingleRow(ci, nullptr);
  }
  /* add state_log */
  std::vector<calculation_info> rc;
  dbm.SelectRows(ci, &rc);
  if (rc.size()) {
    calculation_state_log log;
    log.info_id = rc[0].id;
    log.initialized = log.f_calculation_info_id;
    /* dyn base */
    auto v = 0.0024;
    auto p = 2500000;
    log.dyn_pars.parm.volume = v;
    log.initialized |= log.f_vol;
    log.dyn_pars.parm.pressure = p;
    log.initialized |= log.f_pres;
    log.dyn_pars.parm.temperature = 197;
    log.initialized |= log.f_temp;
    /* dyn */
    log.dyn_pars.heat_cap_vol = 1245;
    log.initialized |= log.f_dcv;
    log.dyn_pars.internal_energy = 985;
    log.initialized |= log.f_din;
    log.state_phase = stateToString[(int)state_phase::NOT_SET];
    log.initialized |= log.f_state_phase;
    std::vector<calculation_state_log> logs;
    std::generate_n(std::back_insert_iterator<std::vector<calculation_state_log>>
        (logs), 5, [&log](){ return log; });
    for (auto &x: logs) {
      v += 0.0003;
      p -= 1272;
      x.dyn_pars.parm.volume = v;
      x.dyn_pars.parm.pressure = p;
    }
    dbm.SaveVectorOfRows(logs);
  }

  /* delete */
  if (r1.size()) {
    model_info mi_del = model_info::GetDefault();
    mi_del.id = r1[0].id;
    mi_del.initialized = mi_del.f_model_id;
    dbm.DeleteRows(mi_del);
  }
#endif  // RK2_TEST
  return res;
}

int test_database() {
  std::string filename = (cwd / xml_gases_dir / xml_configuration).string();
  ProgramState &ps = ProgramState::Instance();
  AthermDBTables adb;
  // merror_t err = ps.ResetConfigFile(filename);
  // if (err)
  //   std::cerr << "update config error: " << hex2str(err) << std::endl;
  // db_parameters p = ps.GetDatabaseConfiguration();
  DBConnectionManager dbm(&adb);
  dbm.ResetConnectionParameters(
      ps.GetDatabaseConfiguration());
  auto st = dbm.CheckConnection();
  if (st == STATUS_HAVE_ERROR) {
    std::cerr << "error during connection check: "
        << dbm.GetErrorCode() << std::endl;
    std::cerr << "  message: " << dbm.GetErrorMessage() << std::endl;
    return 1;
  }
  std::vector<db_table> tables { table_model_info,
      table_calculation_info, table_calculation_state_log };
  for (const auto &x: tables) {
    if (!dbm.IsTableExists(x)) {
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
  ps.ReloadConfiguration((cwd / xml_gases_dir / xml_configuration).string());
  merror_t e = ps.GetErrorCode();
  if (e) {
    std::cerr << "program state bida " << e << std::endl;
    std::cerr << "Current dir is " << cwd << std::endl;
  }
  return e;
}

int test_models() {
  std::vector<std::unique_ptr<modelGeneral>> test_vec;
  std::string filename = (cwd / xml_gases_dir / xml_gasmix).string();
#if defined(RK2_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rk2_str, NULL, filename, INPUT_P_T)));
#endif  // RK2_TEST
#if defined(RKS_DEBUG)
  test_vec.push_back(std::unique_ptr<modelGeneral>(
      ModelsCreator::GetCalculatingModel(rks_str, NULL, filename, INPUT_P_T)));
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
  std::vector<gasmix_component_info> xml_files = std::vector<gasmix_component_info> {
    gasmix_component_info("metane", (cwd / xml_gases_dir / xml_methane).string(), 0.988),
    // add more (summ = 1.00)
    gasmix_component_info("ethane", (cwd / xml_gases_dir / xml_ethane).string(), 0.009),
    gasmix_component_info("propane", (cwd / xml_gases_dir / xml_propane).string(), 0.003)
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
  ProgramState::Instance().SetProgramDirs(
      file_utils::FileURLRoot(file_utils::url_t::fs_path, cwd.string()),
      file_utils::FileURLRoot(file_utils::url_t::fs_path, cwd / xml_calculations_dir));
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

  #if defined(CALCULATION_DEBUG)
    if (test_calculation_init()) return 5;;
  #endif  // CALCULATION_DEBUG
    Logging::Append(io_loglvl::debug_logs, "Ни одной ошибки не заметил");
  //*/
  }

  return 0;
}
