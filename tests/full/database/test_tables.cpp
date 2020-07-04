#include "atherm_common.h"
#include "atherm_db_tables.h"
#include "db_connection.h"
#include "db_connection_manager.h"
#include "models_configurations.h"

#include "gtest/gtest.h"

#include <algorithm>
#include <filesystem>
#include <iostream>
#include <numeric>

#include <assert.h>

AthermDBTables adb_;

/** \brief Тестинг работы с БД */
class DatabaseTablesTest: public ::testing::Test {
protected:
  DatabaseTablesTest()
  : dbm_(&adb_) {
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
  std::vector<db_table> tables { table_model_info,
      table_calculation_info, table_calculation_state_log };
  for (const auto &x: tables) {
    if (!dbm_.IsTableExist(x)) {
      dbm_.CreateTable(x);
      ASSERT_TRUE(dbm_.IsTableExist(x));
    }
  }
}

/** \brief Тест на добавлени строки к таблице моделей */
TEST_F(DatabaseTablesTest, InsertModelInfo) {
  std::string str = "string by gtest";
  /* insert */
  model_info mi = model_info::GetDefault();
  mi.short_info.short_info = str;
  mi.short_info.model_type.subtype = 0xf;
  mi.initialized = mi.f_full & (~mi.f_model_id);
  dbm_.SaveSingleRow(mi);

  /* select */
  std::vector<model_info> r;
  auto st = dbm_.SelectRows(mi, &r);

  ASSERT_TRUE(r.size() > 0);
  EXPECT_GT(r[0].id, 0);
  EXPECT_EQ(r[0].short_info.model_type.type, rg_model_t::EMPTY);
  EXPECT_EQ(r[0].short_info.model_type.subtype, 0xf);
  EXPECT_EQ(r[0].short_info.vers_major, 1);
  EXPECT_EQ(r[0].short_info.vers_minor, 0);
  EXPECT_EQ(r[0].short_info.short_info, str);

  /* delete */
  model_info mi_del = model_info::GetDefault();
  mi_del.id = r[0].id;
  mi_del.initialized = mi_del.f_model_id;
  mi_del.initialized = mi_del.f_short_info;
  st = dbm_.DeleteRows(mi_del);
  ASSERT_TRUE(is_status_ok(st));
}

/** \brief Тест добавления строк в таблицу расчётов */
TEST_F(DatabaseTablesTest, InsertCalculation) {
  model_info mis = model_info::GetDefault();
  mis.short_info.short_info = "Тестовая строка";
  mis.initialized = mis.f_full & (~mis.f_model_id);
  dbm_.SaveSingleRow(mis);

  std::vector<model_info> r;
  model_info mi = model_info::GetDefault();
  mi.short_info.model_type.type = rg_model_t::EMPTY;
  mi.short_info.model_type.subtype = MODEL_SUBTYPE_DEFAULT;
  mi.initialized = mi.f_model_type | mi.f_model_subtype;
  dbm_.SelectRows(mi, &r);
  ASSERT_TRUE(r.size() > 0);

  /* calculation_info */
  calculation_info ci;
  ci.model_id = r[0].id;
  ci.initialized = ci.f_model_id;
  ci.initialized |= ci.f_date | ci.f_time;
  dbm_.SaveSingleRow(ci);

  std::vector<calculation_info> rc;
  ci.initialized = ci.f_model_id;
  dbm_.SelectRows(ci, &rc);
  ASSERT_TRUE(rc.size() > 0);
  ASSERT_TRUE(rc[0].id != -1);

  /* calculation_state_log */
  calculation_state_log log;
  log.info_id = rc[0].id;
  log.initialized = log.f_calculation_info_id;
  /* dyn base */
  auto v = 0.0024;
  auto p = 2500000;
  log.dyn_pars.parm.volume = v;
  log.initialized |= log.f_vol;
  log.dyn_pars.parm.pressure = p;
  log.initialized |= log.f_pres;
  log.dyn_pars.parm.temperature = 197;
  log.initialized |= log.f_temp;
  /* dyn */
  log.dyn_pars.heat_cap_vol = 1245;
  log.initialized |= log.f_dcv;
  log.dyn_pars.internal_energy = 985;
  log.initialized |= log.f_din;
  log.state_phase = stateToString[(int)state_phase::NOT_SET];
  log.initialized |= log.f_state_phase;
  /* vector of calculation_state_log */
  std::vector<calculation_state_log> logs;
  std::generate_n(std::back_insert_iterator<std::vector<calculation_state_log>>
      (logs), 5, [&log](){ return log; });
  for (auto &x: logs) {
    v += 0.0003;
    p -= 1272;
    x.dyn_pars.parm.volume = v;
    x.dyn_pars.parm.pressure = p;
  }
  EXPECT_EQ(dbm_.SaveVectorOfRows(logs), STATUS_OK);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
