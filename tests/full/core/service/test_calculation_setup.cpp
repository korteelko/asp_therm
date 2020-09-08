#include "atherm_db_tables.h"
#include "calculation_setup.h"
#include "Common.h"
#include "ErrorWrap.h"
#include "gas_defines.h"
#include "merror_codes.h"
#include "model_peng_robinson.h"
#include "program_state.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <string>


#define par_input(p, t) \
    parameters {.volume = 0.0, .pressure = p, .temperature = t}

namespace fs = std::filesystem;

static fs::path calculation_filename = "test_calculation.xml";
static fs::path config_filename = "test_config.xml";

static std::string path_gost1 = "calculation/gost_test/gostmix1.xml";
static std::string path_gost2 = "calculation/gost_test/gostmix2.xml";
static std::string path_gost3 = "calculation/gost_test/gostmix3.xml";

/**
 * \brief Класс для тестинга CalculationSetup
 * */
class CalculationSetupProxy {
public:
  CalculationSetupProxy(std::shared_ptr<file_utils::FileURLRoot> &root,
      const std::string &filepath)
    : cs(CalculationSetup(root, filepath)) {}

  CalculationSetup &GetSetup() { return cs; }
  calculation_setup *GetInitData() { return cs.init_data_.get(); }
  std::vector<parameters> &GetPoints() { return cs.points_; }
  std::map<std::string, std::shared_ptr<CalculationSetup::gasmix_models_map>> &
  GetGamixes() { return cs.gasmixes_; }

public:
  CalculationSetup cs;
};

/**
 * \brief Класс тестов сетапа расчётов
 * */
class CalculationSetupTest: public ::testing::Test {
protected:
  CalculationSetupTest() {
    data_root_p_.reset(new file_utils::FileURLRoot(
        file_utils::url_t::fs_path, "./../../../data"));
    if (initCalculations())
      csp_ptr.reset(new CalculationSetupProxy(data_root_p_, calculation_filename.string()));
  }

  bool initCalculations() {
    bool success = false;
    fs::path p = data_root_p_->GetRootURL().GetURL() / calculation_filename;
    auto f = std::fstream(p, std::ios_base::out);

    if (f.is_open()) {
      f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      f << "<calc_setup name=\"example\">\n";
      f << "  <models> PRb, GOST, ISO </models>\n";
      f << "  <gasmix_files>\n";
      f << "    <!-- mixtures from gost-30319 root dir is 'data' -->\n";
      f << "    <mixfile name=\"gostmix1\">" << path_gost1 << "</mixfile>\n";
      f << "    <mixfile name=\"gostmix2\">" << path_gost2 << "</mixfile>\n";
      f << "    <mixfile name=\"gostmix3\">" << path_gost3 << "</mixfile>\n";
      f << "  </gasmix_files>\n";
      f << "  <points>\n";
      f << "    <!-- [p] = Pa, [t] = K -->\n";
      f << "    <point p=\"100000\" t=\"250.0\"/>\n";
      f << "    <point p=\"5000000\" t=\"350.0\"/>\n";
      f << "    <point p=\"30000000\" t=\"300.0\"/>\n";
      f << "  </points>\n";
      f << "</calc_setup>\n";
      f.close();
      success = true;
    }
    return success;
  }

  bool initConfiguration() {
    bool success = false;
    config_file = data_root_p_->GetRootURL().GetURL() / config_filename;
    auto f = std::fstream(config_file, std::ios_base::out);
    if (f.is_open()) {
      f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      f << "<program_config name=\"test_common\">\n";
      f << "  <parameter name=\"debug_mode\"> true </parameter>\n";
      f << "  <parameter name=\"rk_orig_mod\"> true </parameter>\n";
      f << "  <parameter name=\"rk_soave_mod\"> false </parameter>\n";
      f << "  <parameter name=\"pr_binary_coefs\"> true </parameter>\n";
      f << "  <parameter name=\"include_iso_20765\"> true </parameter>\n";
      f << "  <parameter name=\"log_level\"> debug </parameter>\n";
      f << "  <parameter name=\"log_file\"> test_log </parameter>\n";
      f << "  <group name=\"database\">\n";
      f << "    <parameter name=\"dry_run\"> false </parameter>\n";
      f << "    <parameter name=\"client\"> postgresql </parameter>\n";
      f << "    <parameter name=\"name\"> africae </parameter>\n";
      f << "    <parameter name=\"username\"> jorge </parameter>\n";
      f << "    <parameter name=\"password\"> my_pass </parameter>\n";
      f << "    <parameter name=\"host\"> 127.0.0.1 </parameter>\n";
      f << "    <parameter name=\"port\"> 5432 </parameter>\n";
      f << "  </group>\n";
      f << "</program_config>\n";
      f.close();
      success = true;
    }
    return success;
  }

protected:
  /**
   * \brief Корневая директория дополнительных данных программы,
   *  для инициализации смесей, расчётов, т.п.
   * */
  std::shared_ptr<file_utils::FileURLRoot> data_root_p_;
  /**
   * \brief Путь к файлу конфигурации
   * */
  fs::path config_file;
  /**
   * \brief Прокся на сетап расчётов
   */
  std::shared_ptr<CalculationSetupProxy> csp_ptr = nullptr;
};
/**
 * \brief Инициалиазция сетапов расчёта
 * */
TEST_F(CalculationSetupTest, calculation_setup_init) {
  ASSERT_NE(csp_ptr, nullptr);
  CalculationSetup &setup = csp_ptr->GetSetup();
  EXPECT_TRUE(setup.GetError() == ERROR_SUCCESS_T);
  calculation_setup *idp = csp_ptr->GetInitData();
  ASSERT_NE(idp, nullptr);

  // models
  ASSERT_EQ(idp->models.size(), 3);
  EXPECT_EQ(idp->models[0], rg_model_id(
      rg_model_t::PENG_ROBINSON, MODEL_PR_SUBTYPE_BINASSOC));
  EXPECT_EQ(idp->models[1], rg_model_id(
      rg_model_t::NG_GOST, MODEL_SUBTYPE_DEFAULT));
  EXPECT_EQ(idp->models[2], rg_model_id(
      rg_model_t::NG_GOST, MODEL_GOST_SUBTYPE_ISO_20765));

  // files
  ASSERT_EQ(idp->gasmix_files.size(), 3);
  auto p1 = data_root_p_->CreateFileURL(path_gost1).GetURL();
  EXPECT_TRUE(is_exist(p1));
  auto p2 = data_root_p_->CreateFileURL(path_gost2).GetURL();
  EXPECT_TRUE(is_exist(p2));
  auto p3 = data_root_p_->CreateFileURL(path_gost3).GetURL();
  EXPECT_TRUE(is_exist(p3));

  // points
  std::function<bool(const parameters &, const parameters &)> comp_p =
      [] (const parameters &l, const parameters &r) {
      return is_equal(l.volume, r.volume) && is_equal(l.pressure, r.pressure) &&
          is_equal(l.temperature, r.temperature);
  };
  auto points = csp_ptr->GetPoints();
  ASSERT_EQ(points.size(), 3);
  EXPECT_TRUE(comp_p(points[0], par_input(100000.0, 250.0)));
  EXPECT_TRUE(comp_p(points[1], par_input(5000000.0, 350.0)));
  EXPECT_TRUE(comp_p(points[2], par_input(30000000.0, 300.0)));
}
/**
 * \brief Проверка взаимодействия с базой данных
 * */
TEST_F(CalculationSetupTest, DatabaseTest) {
  CalculationSetup &setup = csp_ptr->GetSetup();
  setup.Calculate();
  ASSERT_TRUE(setup.GetError() == ERROR_SUCCESS_T);
  initConfiguration();
  ProgramState &ps = ProgramState::Instance();
  ps.SetProgramDirs(*data_root_p_, *data_root_p_);
  ASSERT_TRUE(is_exist(config_file));
  ASSERT_EQ(ps.ReloadConfiguration(config_filename), ERROR_SUCCESS_T);
  AthermDBTables adb;
  DBConnectionManager dbm(&adb);
  dbm.ResetConnectionParameters(
      ps.GetDatabaseConfiguration());
  ASSERT_TRUE(is_status_ok(dbm.CheckConnection()));
  setup.AddToDatabase(&dbm);
  ASSERT_TRUE(setup.GetError() == ERROR_SUCCESS_T);
  auto &gmap = csp_ptr->GetGamixes();
  // проверим просто что в базе данных появились такие строки
  //   их валидность проверяется в модуле БД
  for (auto &models_map: gmap) {
    ASSERT_NE(models_map.second.get(), nullptr);
    auto vec_mi = models_map.second->GetModelInfo();
    for (auto &mi: vec_mi) {
      std::vector<model_info> selected;
      dbm.SelectRows(mi, &selected);
      // в model_info уникальный комплекс есть
      EXPECT_EQ(selected.size(), 1);
    }
    auto vec_ci = models_map.second->GetCalculationInfo();
    for (auto &ci: vec_ci) {
      std::vector<calculation_info> selected;
      dbm.SelectRows(ci, &selected);
      // в calculation_info тоже уникальный комплекс есть
      EXPECT_EQ(selected.size(), 1);
    }
    /*
     * Такс, с вектором состояний пока не очень понятно как работать,
     *   скорее всего стоит проверять после стабилизации по моделям
     * */
    /*
    auto vec_result = models_map.second->GetCalculationResult();
    for (auto &state_log: vec_result) {
      std::vector<calculation_state_log> selected;
      dbm.SelectRows(state_log, &selected);
      // в calculation_info тоже уникальный комплекс есть
      EXPECT_GT(selected.size(), 0);
    } */
  }
  // auto calcinfo = setup.GetCalculationInfo();
  // auto modelinfo = setup.Get
}
