#include "common.h"
#include "db_connection.h"
#include "db_connection_manager.h"
#include "models_configurations.h"

#include "gtest/gtest.h"

#include <iostream>
#include <filesystem>
#include <numeric>

#include <assert.h>


/** \brief Тестинг работы с БД */
class DatabaseTablesTest: public ::testing::Test {
protected:
  DatabaseTablesTest() {
    db_parameters db_par;
    db_par.is_dry_run = false;
    db_par.supplier = db_client::POSTGRESQL;
    db_par.name = "africae";
    db_par.username = "jorge";
    db_par.password = "my_pass";
    db_par.host = "127.0.0.1";
    db_par.port = 5432;
    EXPECT_TRUE(is_status_aval(dbm_.ResetConnectionParameters(db_par)));
    if (dbm_.GetErrorCode()) {
      std::cerr << dbm_.GetErrorCode();
      EXPECT_TRUE(false);
    }
  }
  ~DatabaseTablesTest() {}

protected:
  /** \brief Менеджер подключения к БД */
  DBConnectionManager dbm_;
};

TEST_F(DatabaseTablesTest, TableExists) {
  ASSERT_TRUE(dbm_.GetErrorCode() == ERROR_SUCCESS_T);
  std::vector<db_table> tables { db_table::table_model_info,
      db_table::table_calculation_info, db_table::table_calculation_state_log };
  for (const auto &x: tables) {
    if (!dbm_.IsTableExist(x)) {
      dbm_.CreateTable(x);
      ASSERT_TRUE(dbm_.IsTableExist(x));
    }
  }
}

/** \brief Тест на добавлени строки к таблице моделей */
TEST_F(DatabaseTablesTest, InsertModelInfo) {
  std::string str = "Тестовая строка";
  /* insert */
  model_info mi = model_info::GetDefault();
  mi.short_info.short_info = str;
  mi.initialized = mi.f_full & (~mi.f_model_id);
  auto st = dbm_.SaveModelInfo(mi);
  std::cerr << "save status: " << st << std::endl;
  ASSERT_TRUE(is_status_ok(st));
  /* select */
  std::vector<model_info> r;
  dbm_.SelectModelInfo(mi, &r);
  ASSERT_EQ(r.size(), 1);
  EXPECT_GT(r[0].id, 0);
  EXPECT_EQ(r[0].short_info.model_type.type, rg_model_t::EMPTY);
  EXPECT_EQ(r[0].short_info.model_type.subtype, MODEL_SUBTYPE_DEFAULT);
  EXPECT_EQ(r[0].short_info.vers_major, 1);
  EXPECT_EQ(r[0].short_info.vers_minor, 0);
  EXPECT_EQ(r[0].short_info.short_info, str);

  /* delete */
  model_info mi_del = model_info::GetDefault();
  mi_del.id = r[0].id;
  mi_del.initialized = mi_del.f_model_id;
  st = dbm_.DeleteModelInfo(mi_del);
  ASSERT_TRUE(is_status_ok(st));
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
