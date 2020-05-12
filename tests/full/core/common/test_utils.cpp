#include "common.h"
#include "merror_codes.h"

#include "ErrorWrap.h"
#include "Logging.h"

#include "gtest/gtest.h"

#include <filesystem>


namespace fs = std::filesystem;

TEST(merror_codes, init) {
  EXPECT_EQ(GetCustomErrorMsg(0x1111), nullptr);
  EXPECT_NE(GetCustomErrorMsg(ERROR_SUCCESS_T), nullptr);
  EXPECT_NE(GetCustomErrorMsg(ERROR_GENERAL_T), nullptr);
}

TEST(ErrorWrap, init) {
  ErrorWrap ew;
  /* конструктор по умолчанию */
  EXPECT_EQ(ew.GetErrorCode(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ew.IsLogged());
  EXPECT_EQ(ew.GetMessage(), "");

  /* накопить изменения */
  ew.SetError(ERROR_GENERAL_T);
  EXPECT_EQ(ew.GetErrorCode(), ERROR_GENERAL_T);
  EXPECT_EQ(ew.GetMessage(), "");

  ew.SetErrorMessage("Тест ошибки");
  EXPECT_EQ(ew.GetMessage(), "Тест ошибки");

  ew.LogIt();
  EXPECT_TRUE(ew.IsLogged());

  /* reset */
  ew.Reset();
  EXPECT_EQ(ew.GetErrorCode(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ew.IsLogged());
  EXPECT_EQ(ew.GetMessage(), "");
}

TEST(Logging, Full) {
  /* обычное логирование */
  EXPECT_EQ(Logging::InitDefault(), ERROR_SUCCESS_T);
  logging_cfg lcfg(io_loglvl::err_logs, "testolog", true);
  ASSERT_EQ(Logging::ResetInstance(lcfg), ERROR_SUCCESS_T);
  Logging::ClearLogfile();
  try {
    EXPECT_EQ(fs::file_size("testolog"), 0);
  } catch (fs::filesystem_error &e) {
    std::cerr << e.what() << std::endl;
    ASSERT_TRUE(false);
  }
  std::stringstream sstr;
  sstr << "Нелогируемая ошибка";
  Logging::Append(io_loglvl::debug_logs, sstr);
  EXPECT_EQ(fs::file_size("testolog"), 0);
  Logging::Append(sstr);
  EXPECT_NE(fs::file_size("testolog"), 0);
  /* логирование ошибки */
  Logging::ClearLogfile();
  EXPECT_EQ(Logging::GetErrorCode(), ERROR_SUCCESS_T);
  ErrorWrap ew(ERROR_FILE_LOGGING_ST, "Тест логирования ошибки");
  ew.LogIt(io_loglvl::err_logs);
  EXPECT_TRUE(ew.IsLogged());
  EXPECT_NE(fs::file_size("testolog"), 0);
}
