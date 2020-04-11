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
#include "program_state.h"
#include "JSONReader.h"

#include <filesystem>
#include <vector>

#include <assert.h>


#define JSON_READER_DEBUG
//#define MODELS_DEBUG
#define RK2_DEBUG
//#define PR_DEBUG  // доделать для смесей
#define RKS_DEBUG
//#define NG_GOST_DEBUG
//#define DATABASE_DEBUG

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

/** \brief тестовая структура-параметр шаблона XMLReader */
struct test_node {
  // static const std::array<std::string, 3> node_t_list;
  struct first {
    std::string f, s, ff;
    float t;
  };
  struct second {
    std::string f;
    int s, t;
  };
public:
  // node_type config_node_type;
  rj::Value *source;
  std::string parent_name;
  std::string name;
  std::vector<std::string> subnodes;
  bool have_subnodes;

public:
  test_node()
    : name("") {}

  std::string GetName() const { return name; }

  bool IsLeafNode() const { return have_subnodes; }

  void SetParentData(test_node *parent) {
    parent_name = parent->GetName();
  }

  merror_t InitData(rj::Value *src) {
    if (!src)
      return ERROR_INIT_NULLP_ST;
    source = src;
    merror_t error = ERROR_SUCCESS_T;
    if (source->HasMember("type")) {
      rj::Value &tmp_nd = source->operator[]("type");
      name = tmp_nd.GetString();
      set_subnodes();
    }
    if (name.empty()) {
      error = ERROR_JSON_FORMAT_ST;
    }
    return error;
  }

  std::string GetParameter(const std::string &name) {
    std::string value = "";
    rj::Value::MemberIterator it = source->FindMember("data");
    if (it != source->MemberEnd()) {
      rj::Value &data = it->value;
      if (data.HasMember(name.c_str())) {
        rj::Value &par = data[name.c_str()];
        if (par.IsString()) {
          value = par.GetString();
        } else if (par.IsInt()) {
          value = std::to_string(par.GetInt());
        } else if (par.IsDouble()) {
          value = std::to_string(par.GetDouble());
        }
      }
    }
    return value;
  }

  merror_t GetFirst(first *f) {
    if (name == "first" && f) {
      f->f = GetParameter("f");
      f->s = GetParameter("s");
      f->ff = GetParameter("ff");
      f->t = std::stof(GetParameter("t"));
      return ERROR_SUCCESS_T;
    }
    return ERROR_GENERAL_T;
  }

  merror_t GetSecond(second *s) {
    if (name == "second" && s) {
      s->f = GetParameter("f");
      s->s = std::atoi(GetParameter("s").c_str());
      s->t = std::atoi(GetParameter("t").c_str());
      return ERROR_SUCCESS_T;
    }
    return ERROR_GENERAL_T;
  }

  /** \brief Записать имена узлов, являющихся
    *   контейнерами других объектов */
  void SetContent(std::vector<std::string> *s) {
    s->clear();
    s->insert(s->end(), subnodes.cbegin(), subnodes.cend());
  }

  static std::string get_root_name() {
    // return node_t_list[NODE_T_ROOT];
    return "test";
  }
  static node_type get_node_type(std::string type) {
    (void) type;
    //for (uint32_t i = 0; i < node_t_list.size(); ++i)
    //  if (node_t_list[i] == type)
    //    return i;
    return NODE_T_UNDEFINED;
  }

private:
  /** \brief проверить наличие подузлов
    * \note просто посмотрим тип этого узла,
    *   а для случая придумаю что-нибудь */
  void set_subnodes() {
    subnodes.clear();
    if (name == "test") {
      have_subnodes = true;
      subnodes.push_back("data");
    } if (name == "first") {
      have_subnodes = true;
    } if (name == "second") {
      have_subnodes = true;
    } else {
      have_subnodes = false;
    }
  }
};


int test_json() {
  file_utils::FileURLRoot file_c(file_utils::SetupURL(
      file_utils::url_t::fs_path, cwd / xml_path));
  auto path =  file_c.CreateFileURL(json_test.string());
  std::unique_ptr<JSONReader<test_node>> jr(JSONReader<test_node>::Init(&path));
  if (jr) {
    if (!jr->GetErrorCode()) {
      std::vector<std::string> path_emp;
      std::string res = "";
      if (jr->GetValueByPath(path_emp, &res))
        std::cerr << "vector of paths empty";
      /* вытянуть обычный параметр */
      std::vector<std::string> path_f = {"first", "t"};
      if (jr->GetValueByPath(path_f, &res))
        std::cerr << "vector of paths error";
    }
  } else {
    std::cerr << "bad init";
    return ERROR_GENERAL_T;
  }
  return ERROR_SUCCESS_T;
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
