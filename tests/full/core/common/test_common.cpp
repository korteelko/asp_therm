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

const unsigned int error_types[] = {
  ERROR_SUCCESS_T,
  ERROR_GENERAL_T,
  ERROR_FILEIO_T,
  ERROR_CALCULATE_T,
  ERROR_STRING_T,
  ERROR_INIT_T,
  ERROR_STRTPL_T,
  ERROR_DATABASE_T
};
#define error_types_size sizeof(error_types)/sizeof(error_types[0])


/** \brief Проверим работу дефолтных сообщений групп */
TEST(defaultErrorMessages, MessagesExists) {
  EXPECT_EQ(GetCustomErrorMsg(0x1111), nullptr);
  // Сообщения групп ошибок
  for (size_t i = 0; i < error_types_size; ++i)
    EXPECT_TRUE(GetCustomErrorMsg(error_types[i]) != nullptr);
  EXPECT_FALSE(GetCustomErrorMsg(ERROR_MASK_TYPE) != nullptr);
  // Существование нетривиальных сообщений ошибок
  for (size_t i = 2; i < error_types_size; ++i) {
    // Для примера первое сообщение
    EXPECT_TRUE(GetCustomErrorMsg(error_types[i] | 0x0100) != nullptr);
    // Ошибка
    EXPECT_TRUE(GetCustomErrorMsg(error_types[i] | 0xff00) == nullptr);
  }
}

/** \brief Класс тестов состояния программы */
class ProgramStateTest: public ::testing::Test {
protected:
  ProgramStateTest() {
    ProgramState &state = ProgramState::Instance();
    fs::path root = cwd / asp_therm_root;
    if (createConfFile()) {
      file_utils::FileURLRoot r = file_utils::FileURLRoot(
          file_utils::url_t::fs_path, root.string());
      EXPECT_TRUE(r.IsInitialized());
      state.SetWorkDir(r);
      EXPECT_TRUE(is_status_ok(state.GetStatus()));
      state.ReloadConfiguration(conf_file);
      status = state.GetStatus();
      if (!state.IsInitialized()) {
        EXPECT_TRUE(false);
        std::cerr << "code error:" << state.GetErrorCode() <<  " \n"
            << state.GetErrorMessage() << "\n";
        state.LogError();
      }
    } else {
      EXPECT_TRUE(false);
      std::cerr << "code error:" << state.GetErrorCode() <<  " \n"
          << state.GetErrorMessage() << "\n";
      state.LogError();
    }
  }

  bool createConfFile() {
    bool success = false;
    fs::path p = cwd / asp_therm_root / conf_file;
    auto f = std::fstream(p, std::ios_base::out);
    if (f.is_open()) {
      f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      f << "<program_config name=\"test_common\">\n";
      f << "  <parameter name=\"debug_mode\"> true </parameter>\n";
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

protected:
  mstatus_t status = STATUS_DEFAULT;
};

/** \brief Инициализация состояния программы */
TEST_F(ProgramStateTest, InitCheck) {
  ASSERT_TRUE(is_status_ok(status));
  ProgramState &state = ProgramState::Instance();
  EXPECT_TRUE(state.IsInitialized());
  EXPECT_TRUE(state.IsDebugMode());
  EXPECT_TRUE(state.IsDryRunDBConn());
}

int main(int argc, char **argv) {
  cwd = fs::path(argv[0]).parent_path();
  fs::current_path(cwd);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
