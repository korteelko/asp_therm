/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_defines.h"

#include <assert.h>


std::string db_client_to_string(db_client client) {
  std::string name = "";
  switch (client) {
    case db_client::NOONE:
      name = "dummy connection";
      break;
    case db_client::POSTGRESQL:
      name = "postgresql";
      break;
    default:
      assert(0);
      name = "assert";
  }
  return name;
}

std::string get_table_name(db_table dt) {
  std::string name = "";
  switch (dt) {
    case db_table::table_model_info:
      name = "model_info";
      break;
    case db_table::table_calculation_info:
      name = "calculation_info";
      break;
    case db_table::table_calculation_state_log:
      name = "calculation_state_log";
      break;
    default:
      assert(0 && "нужно добавить больше типов таблиц");
  }
  return name;
}
