#include "common.h"
// #include "db_connection.h"
// #include "db_connection_manager.h"
#include "db_queries_setup.h"
#include "models_configurations.h"

#include "gtest/gtest.h"

#include <iostream>
#include <filesystem>
#include <numeric>

#include <assert.h>

TEST(db_where_tree, model_info) {
  model_info mi = model_info::GetDefault();
  /* empty(for 'select' it is all data) */
  mi.initialized = mi.f_empty;
  std::unique_ptr<db_where_tree> wt(db_where_tree::Init(mi));
  EXPECT_TRUE(wt == nullptr);
  mi.id = 101;
  mi.initialized = mi.f_model_id;
  wt.reset(db_where_tree::Init(mi));
  EXPECT_EQ(trim_str(wt->GetString()), db_condition_node::DataToStr(
      db_type::type_int, MI_MODEL_ID, std::to_string(mi.id)));
  mi.short_info.short_info = "Тестовая \"строка\"";
  mi.initialized |= mi.f_short_info;
  wt.reset(db_where_tree::Init(mi));
  /* строка стандартными методами */
  std::string where_str1 = db_condition_node::DataToStr(
      db_type::type_int, MI_MODEL_ID, std::to_string(mi.id));
  where_str1 += " AND ";
  where_str1 += db_condition_node::DataToStr(
      db_type::type_char_array, MI_SHORT_INFO, mi.short_info.short_info);
  /* строка в обычном представлении */
  std::string where_str2 = std::string(MI_MODEL_ID )+ " = 101 AND " +
      MI_SHORT_INFO + " = 'Тестовая \"строка\"'";
  EXPECT_EQ(trim_str(wt->GetString()), where_str1);
  EXPECT_EQ(trim_str(wt->GetString()), where_str2);
}

TEST(db_where_tree, calculation_info) {
  calculation_info ci;
  /* empty(for 'select' it is all data) */
  ci.initialized = ci.f_empty;
  std::unique_ptr<db_where_tree> wt(db_where_tree::Init(ci));
  EXPECT_TRUE(wt == nullptr);
  ci.id = 102;
  ci.initialized = ci.f_calculation_info_id;
  wt.reset(db_where_tree::Init(ci));
  EXPECT_EQ(trim_str(wt->GetString()), db_condition_node::DataToStr(
      db_type::type_int, CI_CALCULATION_ID, std::to_string(ci.id)));
  ci.SetDate("1934/03/09");
  ci.initialized |= ci.f_date;
  wt.reset(db_where_tree::Init(ci));
  /* строка стандартными методами */
  std::string where_str1 = db_condition_node::DataToStr(
      db_type::type_int, CI_CALCULATION_ID, std::to_string(ci.id));
  where_str1 += " AND ";
  where_str1 += db_condition_node::DataToStr(
      db_type::type_date, CI_DATE, ci.GetDate());
  /* строка в обычном представлении */
  std::string where_str2 = std::string(CI_CALCULATION_ID)+ " = 102 AND " +
      CI_DATE + " = 1934/03/09";
  EXPECT_EQ(trim_str(wt->GetString()), where_str1);
  EXPECT_EQ(trim_str(wt->GetString()), where_str2);

  ci.SetTime("12:42");
  ci.initialized |= ci.f_time;
  wt.reset(db_where_tree::Init(ci));
  std::string where_str3 = where_str2 + " AND " + CI_TIME + "12:42";
}

TEST(db_where_tree, calculation_state_log) {
  calculation_state_log log;
  log.initialized = log.f_empty;
  std::unique_ptr<db_where_tree> wt(db_where_tree::Init(log));
  EXPECT_TRUE(wt == nullptr);
  log.id = 103;
  log.initialized = log.f_calculation_state_log_id;
  wt.reset(db_where_tree::Init(log));
  EXPECT_EQ(trim_str(wt->GetString()), db_condition_node::DataToStr(
      db_type::type_int, CSL_LOG_ID, std::to_string(log.id)));
  /* base */
  log.dyn_pars.parm.volume = 0.002;
  log.initialized |= log.f_vol;
  wt.reset(db_where_tree::Init(log));
  std::string w1 = db_condition_node::DataToStr(
      db_type::type_int, CSL_LOG_ID, std::to_string(log.id));
  w1 += " AND ";
  w1 += db_condition_node::DataToStr(
      db_type::type_int, CSL_VOLUME, std::to_string(log.dyn_pars.parm.volume));
  EXPECT_EQ(trim_str(wt->GetString()), w1);
}
