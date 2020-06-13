/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_tables.h"

#include <map>

#include <assert.h>


#define tables_pair(x, y) { x, y }

static std::map<db_table, std::string> str_tables =
    std::map<db_table, std::string> {
  tables_pair(db_table::table_model_info, "model_info"),
  tables_pair(db_table::table_calculation_info, "calculation_info"),
  tables_pair(db_table::table_calculation_state_log, "calculation_state_log"),
};

std::string get_table_name(db_table dt) {
  auto x = str_tables.find(dt);
  return (x != str_tables.end()) ? x->second: "";
}

db_table get_table_code(const std::string &str) {
  for (const auto &x: str_tables)
    if (x.second == str)
      return x.first;
  return db_table::table_undefiend;
}

std::string get_id_column_name(db_table dt) {
  std::string name = "";
  switch (dt) {
    case db_table::table_model_info:
      name = TABLE_FIELD_NAME(MI_MODEL_ID);
      break;
    case db_table::table_calculation_info:
      name = TABLE_FIELD_NAME(CI_CALCULATION_ID);
      break;
    case db_table::table_calculation_state_log:
      name = TABLE_FIELD_NAME(CSL_LOG_ID);
      break;
    case db_table::table_undefiend:
      break;
  }
  return name;
}

