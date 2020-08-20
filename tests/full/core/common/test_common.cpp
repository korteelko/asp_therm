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

namespace fs = std::filesystem;

static fs::path cwd;
static fs::path asp_therm_root = "../../../tests/full/utils/data/";
static std::string conf_file = "test_configuration.xml";
static std::string calc_file = "test_calculation.xml";

static std::string path_gost1 = "../../data/gost/gostmix1.xml";
static std::string path_gost2 = "../../data/gost/gostmix2.xml";
static std::string path_gost3 = "../../data/gost/gostmix3.xml";

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

public:
  CalculationSetup cs;
};


/**
 * \brief Класс тестов состояния программы
 * */
class ProgramStateTest: public ::testing::Test {
protected:
  ProgramStateTest() {
    ProgramState &state = ProgramState::Instance();
    program_root_ = cwd / asp_therm_root;
    if (createConfFile()) {
      data_root_p_.reset(new file_utils::FileURLRoot(
          file_utils::url_t::fs_path, program_root_.string()));
      EXPECT_TRUE(data_root_p_->IsInitialized());
      if (data_root_p_) {
        // state.SetWorkDir(*data_root_p_);
      } else {
        EXPECT_TRUE(false);
      }

      EXPECT_TRUE(is_status_ok(state.GetStatus()));
      state.ReloadConfiguration(conf_file);
      status = state.GetStatus();
      if (!state.IsInitialized()) {
        EXPECT_TRUE(false);
        std::cerr << "code error:" << state.GetErrorCode() <<  " \n"
            << state.GetErrorMessage() << "\n";
        state.LogError();
      }
      if (!initCalculations())
        status = STATUS_NOT;
    } else {
      EXPECT_TRUE(false);
      std::cerr << "code error:" << state.GetErrorCode() <<  " \n"
          << state.GetErrorMessage() << "\n";
      state.LogError();
    }
  }

  bool createConfFile() {
    bool success = false;
    fs::path p = program_root_ / conf_file;
    auto f = std::fstream(p, std::ios_base::out);
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
      f << "    <parameter name=\"dry_run\"> true </parameter>\n";
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

  bool initCalculations() {
    bool success = false;
    fs::path p = program_root_ / calc_file;
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

protected:
  mstatus_t status = STATUS_DEFAULT;
  /**
   * \brief Корневая директория программы, для чтения конфигурационных файлов
   */
  fs::path program_root_;
  /**
   * \brief Корневая директория дополнительных данных программы,
   *  для инициализации смесей, расчётов, т.п.
   */
  std::shared_ptr<file_utils::FileURLRoot> data_root_p_;
};

/**
 * \brief Инициализация состояния программы
 * */
TEST_F(ProgramStateTest, InitCheck) {
  ASSERT_TRUE(is_status_ok(status));
  ProgramState &state = ProgramState::Instance();
  ASSERT_TRUE(state.IsInitialized());
  EXPECT_TRUE(state.IsDebugMode());
  EXPECT_TRUE(state.IsDryRunDBConn());
  const auto config = state.GetConfiguration();
}

/**
 * \brief Инициализация состояния программы
 * */
TEST_F(ProgramStateTest, ModelsInit) {
  ProgramState &state = ProgramState::Instance();
  if (state.IsInitialized()) {
    /* program_configuration */
    auto prog_config = state.GetConfiguration();

    EXPECT_TRUE(prog_config.log_level == io_loglvl::debug_logs);
    EXPECT_TRUE(prog_config.log_file == "test_log");

    /* calculation_configuration */
    auto calc_config = state.GetCalcConfiguration();
    EXPECT_TRUE(calc_config.IsDebug());
    EXPECT_TRUE(calc_config.RK_IsEnableOriginMod());
    EXPECT_FALSE(calc_config.RK_IsEnableSoaveMod());
    EXPECT_TRUE(calc_config.PR_IsEnableByBinaryCoefs());
    EXPECT_TRUE(calc_config.IsEnableISO20765());

    /* db_parameters */
    auto db_config = state.GetDatabaseConfiguration();
    EXPECT_TRUE(db_config.is_dry_run);
    EXPECT_TRUE(db_config.supplier == asp_db::db_client::POSTGRESQL);
    EXPECT_TRUE(db_config.name == "africae");
    EXPECT_TRUE(db_config.username == "jorge");
    EXPECT_TRUE(db_config.password == "my_pass");
    EXPECT_TRUE(db_config.host == "127.0.0.1");
    EXPECT_TRUE(db_config.port == 5432);
  }
}

/**
 * \brief Инициалиазция сетапов расчёта
 * */
TEST_F(ProgramStateTest, calculation_setup_init) {
  CalculationSetupProxy csp(data_root_p_, calc_file);
  CalculationSetup &setup = csp.GetSetup();
  EXPECT_TRUE(setup.GetError() == ERROR_SUCCESS_T);
  calculation_setup *idp = csp.GetInitData();
  ASSERT_NE(idp, nullptr);

  // models
  ASSERT_EQ(idp->models.size(), 3);
  EXPECT_EQ(idp->models[0], rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_PR_SUBTYPE_BINASSOC));
  EXPECT_EQ(idp->models[1], rg_model_id(rg_model_t::NG_GOST, MODEL_SUBTYPE_DEFAULT));
  EXPECT_EQ(idp->models[2], rg_model_id(rg_model_t::NG_GOST, MODEL_GOST_SUBTYPE_ISO_20765));

  // files
  ASSERT_EQ(idp->gasmix_files.size(), 3);
  auto p1 = data_root_p_->CreateFileURL(path_gost1).GetURL();
  EXPECT_TRUE(is_exist(p1));
  EXPECT_EQ(idp->gasmix_files[0], p1);
  auto p2 = data_root_p_->CreateFileURL(path_gost2).GetURL();
  EXPECT_TRUE(is_exist(p2));
  EXPECT_EQ(idp->gasmix_files[1], p2);
  auto p3 = data_root_p_->CreateFileURL(path_gost3).GetURL();
  EXPECT_TRUE(is_exist(p3));
  EXPECT_EQ(idp->gasmix_files[2], p3);

  // points
  std::function<bool(const parameters &, const parameters &)> comp_p =
      [] (const parameters &l, const parameters &r) {
      return is_equal(l.volume, r.volume) && is_equal(l.pressure, r.pressure) &&
          is_equal(l.temperature, r.temperature);
  };
  ASSERT_EQ(idp->points.size(), 3);
  EXPECT_TRUE(comp_p(idp->points[0],
      parameters {.volume = 0.0, .pressure = 100000.0, .temperature = 250.0}));
  EXPECT_TRUE(comp_p(idp->points[1],
      parameters {.volume = 0.0, .pressure = 5000000.0, .temperature = 350.0}));
  EXPECT_TRUE(comp_p(idp->points[2],
      parameters {.volume = 0.0, .pressure = 30000000.0, .temperature = 300.0}));
}

/**
 * \brief Сборка моделей
 * */
TEST_F(ProgramStateTest, DISABLED_AddModels) {
  // calculation_setup cs
}


int main(int argc, char **argv) {
  cwd = fs::path(argv[0]).parent_path();
  fs::current_path(cwd);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
