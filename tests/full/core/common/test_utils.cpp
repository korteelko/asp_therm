#include "common.h"
#include "merror_codes.h"

#include "ErrorWrap.h"
#include "Logging.h"
#include "FileURL.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <iostream>

#include <assert.h>


using namespace file_utils;
namespace fs = std::filesystem;

TEST(merror_codes, init) {
  EXPECT_EQ(GetCustomErrorMsg(0x1111), nullptr);
  EXPECT_NE(GetCustomErrorMsg(ERROR_SUCCESS_T), nullptr);
  EXPECT_NE(GetCustomErrorMsg(ERROR_GENERAL_T), nullptr);
}

TEST(ErrorWrap, Full) {
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

TEST(FileURL, Full) {
  std::string td = "test_dir";
  std::string tf = "test_file";
  fs::path test_dir(td);
  if (!fs::is_directory(test_dir)) {
    ASSERT_TRUE(fs::create_directory(test_dir));
  }
  SetupURL setup(url_t::fs_path, td);
  EXPECT_EQ(setup.GetURLType(), url_t::fs_path);
  EXPECT_EQ(setup.GetFullPrefix(), td);
  std::fstream file(td + "/" + tf, std::ios_base::out);
  ASSERT_TRUE(file.is_open());
  file.close();
  FileURLRoot uroot(setup);
  ASSERT_TRUE(uroot.IsInitialized());

  /* file_url */
  FileURL ufile = uroot.CreateFileURL(tf);
  EXPECT_EQ(ufile.GetError(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ufile.IsInvalidPath());
  EXPECT_EQ(ufile.GetURL(), "test_dir/test_file");
  ufile.SetError(ERROR_FILE_OUT_ST, "Тест ошибки");
  EXPECT_TRUE(ufile.IsInvalidPath());

  EXPECT_TRUE(fs::remove_all(test_dir));
}
