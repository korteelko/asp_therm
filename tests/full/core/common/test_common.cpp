#include "calculation_setup.h"
#include "ErrorWrap.h"
#include "merror_codes.h"
#include "program_state.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <string>

namespace fs = std::filesystem;

static fs::path cwd;
static fs::path asp_therm_root = "../../../tests/full/utils/data/";
static std::string conf_file = "test_configuration.xml";
static std::string calc_file = "test_calculation.xml";


/**
 * \brief Класс для тестинга CalculationSetup
 * */
class CalculationSetupProxy {
public:
  CalculationSetupProxy(std::shared_ptr<file_utils::FileURLRoot> &root,
      const std::string &filepath)
    : cs(CalculationSetup(root, filepath)) {}

  const calculation_setup &GetSetup() const { return *cs.init_data_; }

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
      f << "    <mixfile name=\"gostmix1\"> ../../data/gost/gostmix1.xml </mixfile>\n";
      f << "    <mixfile name=\"gostmix2\"> ../../data/gost/gostmix2.xml </mixfile>\n";
      f << "    <mixfile name=\"gostmix3\"> ../../data/gost/gostmix3.xml </mixfile>\n";
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

/** \brief Инициализация состояния программы */
TEST_F(ProgramStateTest, InitCheck) {
  ASSERT_TRUE(is_status_ok(status));
  ProgramState &state = ProgramState::Instance();
  ASSERT_TRUE(state.IsInitialized());
  EXPECT_TRUE(state.IsDebugMode());
  EXPECT_TRUE(state.IsDryRunDBConn());
  const auto config = state.GetConfiguration();
}

/** \brief Инициализация состояния программы */
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
TEST_F(ProgramStateTest, CalculationSetup) {
  CalculationSetupProxy cs(data_root_p_, calc_file);

  // todo:
  //zdesya
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
