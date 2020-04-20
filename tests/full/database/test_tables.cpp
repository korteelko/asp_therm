#include "common.h"
#include "db_connection.h"
#include "db_connection_manager.h"

#include "gtest/gtest.h"

#include <iostream>
#include <filesystem>
#include <numeric>

#include <assert.h>


/** \brief Тестинг работы с БД */
class DatabaseTablesTest: public ::testing::Test {
protected:
  DatabaseTablesTest() {
    db_par_.is_dry_run = false;
    db_par_.supplier = db_client::POSTGRESQL;
    db_par_.name = "africae";
    db_par_.username = "jorge";
    db_par_.password = "my_pass";
    db_par_.host = "127.0.0.1";
    db_par_.port = 5432;
    EXPECT_TRUE(is_status_aval(dbm_.ResetConnectionParameters(db_par_)));
    if (dbm_.GetErrorCode()) {
      std::cerr << dbm_.GetErrorCode();
      EXPECT_TRUE(false);
    }
  }

  ~DatabaseTablesTest() {}

protected:
  db_parameters db_par_;
  DBConnectionManager dbm_;
};

TEST_F(DatabaseTablesTest, TableExists) {
  ASSERT_TRUE(dbm_.GetErrorCode() == ERROR_SUCCESS_T);
  std::vector<db_table> tables { db_table::table_model_info,
      db_table::table_calculation_info, db_table::table_calculation_state_log };
  for (const auto &x : tables) {
    if (!dbm_.IsTableExist(x)) {
      dbm_.CreateTable(x);
      ASSERT_TRUE(dbm_.IsTableExist(x));
    }
  }
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
