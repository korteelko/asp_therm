#include "configuration_strtpl.h"
#include "ErrorWrap.h"
#include "file_structs.h"
#include "FileURL.h"
#include "Logging.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <iostream>

#include <assert.h>


using namespace file_utils;
using namespace asp_db;
namespace fs = std::filesystem;

TEST(ErrorWrap, Full) {
  ErrorWrap ew;
  /* конструктор по умолчанию */
  EXPECT_EQ(ew.GetErrorCode(), ERROR_SUCCESS_T);
  EXPECT_FALSE(ew.IsLogged());
  EXPECT_EQ(ew.GetMessage(), "");

  /* накопить изменения */
  ew.SetError(ERROR_PAIR_DEFAULT(ERROR_GENERAL_T));
  EXPECT_EQ(ew.GetErrorCode(), ERROR_GENERAL_T);
  EXPECT_FALSE(ew.GetMessage() == "");

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
  Logging::ClearLogfile();
  Logging::Append(io_loglvl::err_logs, std::string("Нелогируемая ошибка"));
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

TEST(Subroutins, StringToParam) {
  bool b = false;
  db_client db = db_client::NOONE;
  int i = 0;
  io_loglvl ll = io_loglvl::no_log;

  /* available conversations */
  /*   bool */
  EXPECT_TRUE(set_bool(STRTPL_BOOL_TRUE, &b) == ERROR_SUCCESS_T);
  EXPECT_EQ(b, true);
  EXPECT_TRUE(set_bool(STRTPL_BOOL_FALSE, &b) == ERROR_SUCCESS_T);
  EXPECT_EQ(b, false);

  /* db_client */
  EXPECT_TRUE(set_db_client(STRTPL_DB_CLIENT_NOONE, &db) == ERROR_SUCCESS_T);
  EXPECT_EQ(db, db_client::NOONE);
  EXPECT_TRUE(set_db_client(STRTPL_DB_CLIENT_POSTGRESQL, &db) == ERROR_SUCCESS_T);
  EXPECT_EQ(db, db_client::POSTGRESQL);

  /* int */
  EXPECT_TRUE(set_int("42", &i) == ERROR_SUCCESS_T);
  EXPECT_EQ(i, 42);

  /* log_levels */
  EXPECT_TRUE(set_loglvl(STRTPL_LOG_LEVEL_NO_LOGS, &ll) == ERROR_SUCCESS_T);
  EXPECT_EQ(ll, io_loglvl::no_log);
  EXPECT_TRUE(set_loglvl(STRTPL_LOG_LEVEL_ERR, &ll) == ERROR_SUCCESS_T);
  EXPECT_EQ(ll, io_loglvl::err_logs);
  EXPECT_TRUE(set_loglvl(STRTPL_LOG_LEVEL_WARN, &ll) == ERROR_SUCCESS_T);
  EXPECT_EQ(ll, io_loglvl::warn_logs);
  EXPECT_TRUE(set_loglvl(STRTPL_LOG_LEVEL_DEBUG, &ll) == ERROR_SUCCESS_T);
  EXPECT_EQ(ll, io_loglvl::debug_logs);

  /* no available */
  const std::string e = "no available";
  EXPECT_FALSE(set_bool(e, &b) == ERROR_SUCCESS_T);
  EXPECT_FALSE(set_db_client(e, &db) == ERROR_SUCCESS_T);
  EXPECT_FALSE(set_int(e, &i) == ERROR_SUCCESS_T);
  EXPECT_FALSE(set_int("2147483648", &i) == ERROR_SUCCESS_T);
  EXPECT_FALSE(set_loglvl(e, &ll) == ERROR_SUCCESS_T);
}
