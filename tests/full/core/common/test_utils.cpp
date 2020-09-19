#include "configuration_strtpl.h"
#include "ErrorWrap.h"
#include "file_structs.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <iostream>

#include <assert.h>


using namespace asp_db;

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

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
