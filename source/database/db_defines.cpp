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
#include "Logging.h"

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


/* db_variable */
db_variable::db_variable(std::string fname, db_var_type type,
    db_variable_flags flags, int len)
  : fname(fname), type(type), flags(flags), len(len) {}

merror_t db_variable::CheckYourself() const {
  merror_t ew = ERROR_SUCCESS_T;
  if (fname.empty()) {
    ew = ERROR_DB_VARIABLE;
    Logging::Append(io_loglvl::err_logs, "пустое имя поля таблицы бд" +
        STRING_DEBUG_INFO);
  } else if (type == db_var_type::type_char_array &&
      (!flags.is_array || len < 1)) {
    ew = ERROR_DB_VARIABLE;
    Logging::Append(io_loglvl::err_logs,
        "установки поля char array не соответствуют ожиданиям :(" +
        STRING_DEBUG_INFO);
  }
  return ew;
}


db_reference::db_reference(const std::string &fname, db_table ftable,
    const std::string &f_fname, bool is_fkey, db_reference_act on_del,
    db_reference_act on_upd)
  : fname(fname), foreign_table(ftable), foreign_fname(f_fname),
    is_foreign_key(is_fkey), has_on_delete(false), delete_method(on_del),
    update_method(on_upd) {
  if (delete_method != db_reference_act::ref_act_empty)
    has_on_delete = true;
  if (update_method != db_reference_act::ref_act_empty)
    has_on_update = true;
}

merror_t db_reference::CheckYourself() const {
  merror_t error = ERROR_SUCCESS_T;
  if (!fname.empty()) {
    if (!foreign_fname.empty()) {
      // перестраховались
      if (has_on_delete && delete_method == db_reference_act::ref_act_empty) {
        error = ERROR_DB_VARIABLE;
        Logging::Append(io_loglvl::err_logs, "несоответсвие метода удаления "
            "для ссылки. Поле: " + fname + ". Внешнее поле: " + foreign_fname);
      }
      if (has_on_update && update_method == db_reference_act::ref_act_empty) {
        error = ERROR_DB_VARIABLE;
        Logging::Append(io_loglvl::err_logs, "несоответсвие метода обновления "
            "для ссылки. Поле: " + fname + ". Внешнее поле: " + foreign_fname);
      }
    } else {
      error = ERROR_DB_VARIABLE;
      Logging::Append(io_loglvl::err_logs, "пустое имя поля для "
          "внешней таблицы бд\n");
    }
  } else {
    error = ERROR_DB_VARIABLE;
      Logging::Append(io_loglvl::err_logs, "пустое имя поля таблицы бд\n");
  }
  return error;
}

/* this is for postgresql, what about anothers? */
std::string db_reference::GetReferenceActString(db_reference_act act) {
  switch (act) {
    case db_reference_act::ref_act_empty:
      return "";
    case db_reference_act::ref_act_cascade:
      return "CASCADE";
    case db_reference_act::ref_act_restrict:
      return "RESTRICT";
  }
  return "";
}
