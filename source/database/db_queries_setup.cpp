/**
 * asp_therm - implementation of real gas equations of state
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_queries_setup.h"

#include "models_configurations.h"

#include <algorithm>

#include <assert.h>


#define MI_MODEL_ID "model_id"
#define MI_MODEL_TYPE "model_type"
#define MI_MODEL_SUBTYPE "model_subtype"
#define MI_VERS_MAJOR "vers_major"
#define MI_VERS_MINOR "vers_minor"
#define MI_SHORT_INFO "short_info"

#define CI_CALCULATION_ID "calculation_id"
#define CI_MODEL_INFO_ID "model_info_id"
#define CI_DATE "date"
#define CI_TIME "time"

#define CSL_LOG_ID "calculation_log_id"
#define CSL_INFO_ID "calculation_info_id"
#define CSL_VOLUME "volume"
#define CSL_PRESSURE "pressure"
#define CSL_TEMPERATURE "temperature"
#define CSL_HEAT_CV "heat_capacity_vol"
#define CSL_HEAT_CP "heat_capacity_pres"
#define CSL_INTERNAL_ENERGY "internal_energy"
#define CSL_BETA_KR "beta_kr"
#define CSL_ENTHALPY "enthalpy"
#define CSL_STATE_PHASE "state_phase"
namespace table_fields_setup {
/** \brief Сетап таблицы БД хранения данных о модели */
/* SQL_QUERY:
TABLE MODEL_INFO (
  model_id autoinc,
  model_type uint NOT NULL,
  model_subtype uint,
  vers_major uint not NULL,
  vers_minor uint,
  short_info string,
  PRIMARY KEY (model_id)
); */
static const db_fields_collection model_info_fields = {
  db_variable(MI_MODEL_ID, db_type::type_autoinc, {.is_primary_key = true,
      .is_unique = true, .can_be_null = false}),
  db_variable(MI_MODEL_TYPE, db_type::type_int, {.can_be_null = false,
      .is_complex_unique = true}),
  db_variable(MI_MODEL_SUBTYPE, db_type::type_int, {.is_complex_unique = true}),
  db_variable(MI_VERS_MAJOR, db_type::type_int, {.can_be_null = false,
      .is_complex_unique = true}),
  db_variable(MI_VERS_MINOR, db_type::type_int, {.is_complex_unique = true}),
  db_variable(MI_SHORT_INFO, db_type::type_text, {})
};
static const db_table_create_setup model_info_create_setup(
    db_table::table_model_info, model_info_fields);

/* SQL_QUERY:
TABLE CALCULATION_INFO (
  calculation_id autoinc,
  model_info_id int,
  date date,
  time time,
  PRIMARY KEY (calculation_id),
  FOREIGN KEY (model_info_id) REFERENCES model_info(model_id)
); */
static const db_fields_collection calculation_info_fields = {
  db_variable(CI_CALCULATION_ID, db_type::type_autoinc, {.is_primary_key = true,
      .is_unique = true, .can_be_null = false}),
  // reference to model_info(fk)
  db_variable(CI_MODEL_INFO_ID, db_type::type_int, {.is_reference = true,
      .can_be_null = false, .is_complex_unique = true}),
  db_variable(CI_DATE, db_type::type_date, {.can_be_null = false,
      .is_complex_unique = true}),
  db_variable(CI_TIME, db_type::type_time, {.can_be_null = false,
      .is_complex_unique = true})
};
static const db_ref_collection calculation_info_references = {
  db_reference(CI_MODEL_INFO_ID, db_table::table_model_info, MI_MODEL_ID, true)
};
static const db_table_create_setup calculation_info_create_setup(
    db_table::table_calculation_info, calculation_info_fields);

/* SQL_QUERY:
TABLE CALCULATION_STATE_LOG (
  calcaultion_log_id int,
  calcaultion_info_id int,
  volume real,
  pressure real,
  temperature real,
  heat_capac_vol real,
  heat_capac_pres real,
  internal_energy real,
  beta_kr real,
  enthalpy real
  state_phase char(12)
  PRIMARY KEY (calculation_log_id, calculation_info_id),
  FOREIGN KEY (calculation_info_id)
      REFERENCES calculation_info(calculation_id) ON DELETE CASCADE
); */
static const db_fields_collection calculation_state_log_fields = {
  db_variable(CSL_LOG_ID, db_type::type_autoinc, {.is_primary_key = true,
      .is_unique = true, .can_be_null = false}),
  db_variable(CSL_INFO_ID, db_type::type_int, {.is_primary_key = true,
      .is_reference = true, .can_be_null = false}),
  db_variable(CSL_VOLUME, db_type::type_real, {}),
  db_variable(CSL_PRESSURE, db_type::type_real, {}),
  db_variable(CSL_TEMPERATURE, db_type::type_real, {}),
  db_variable(CSL_HEAT_CV, db_type::type_real, {}),
  db_variable(CSL_HEAT_CP, db_type::type_real, {}),
  db_variable(CSL_INTERNAL_ENERGY, db_type::type_real, {}),
  db_variable(CSL_BETA_KR, db_type::type_real, {}),
  db_variable(CSL_ENTHALPY, db_type::type_real, {}),
  db_variable(CSL_STATE_PHASE, db_type::type_char_array, {.is_array = true}, 12)
};
static const db_ref_collection calculation_state_log_references = {
  db_reference(CSL_INFO_ID, db_table::table_calculation_info, CI_CALCULATION_ID,
      true, db_reference::db_reference_act::ref_act_cascade,
      db_reference::db_reference_act::ref_act_cascade)
};
static const db_table_create_setup calculation_state_log_create_setup(
    db_table::table_calculation_state_log, calculation_state_log_fields);

const db_fields_collection *get_fields_collection(db_table dt) {
  const db_fields_collection *result = nullptr;
  switch (dt) {
    case db_table::table_model_info:
      result = &model_info_fields;
      break;
    case db_table::table_calculation_info:
      result = &calculation_info_fields;
      break;
    case db_table::table_calculation_state_log:
      result = &calculation_state_log_fields;
      break;
    default:
      assert(0 && "undef table");
      break;
  }
  return result;
}

}  // namespace table_fields_setup

namespace ns_tfs = table_fields_setup;

/* db_table_create_setup */
db_table_create_setup::db_table_create_setup(db_table table,
    const db_fields_collection &fields)
  : table(table), fields(fields), ref_strings(nullptr) {
  switch (table) {
    case db_table::table_model_info:
      break;
    case db_table::table_calculation_info:
      ref_strings = &ns_tfs::calculation_info_references;
      break;
    case db_table::table_calculation_state_log:
      ref_strings = &ns_tfs::calculation_state_log_references;
      break;
    default:
      assert(0 && "undef table");
      break;
  }
  setupPrimaryKeyString();
  checkReferences();
  if (init_error.GetErrorCode()) {
    init_error.LogIt();
  }
}

void db_table_create_setup::setupPrimaryKeyString() {
  pk_string.fnames.clear();
  for (const auto &field: fields) {
    if (field.flags.is_primary_key)
      pk_string.fnames.push_back(field.fname);
  }
  assert(pk_string.fnames.empty() == false);
}

// todo: too long function
//   split in few method(for example remove 'switch')
void db_table_create_setup::checkReferences() {
  if (ref_strings) {
    for (auto const &tref : *ref_strings) {
      // check fname present in fields
      bool exist = std::find_if(fields.begin(), fields.end(),
          [&tref](const db_variable &v)
              {return v.fname == tref.fname;}) != fields.end();
      if (exist) {
        const db_fields_collection *ffields =
            ns_tfs::get_fields_collection(tref.foreign_table);
        exist = std::find_if(ffields->begin(), ffields->end(),
            [&tref](const db_variable &v)
                {return v.fname == tref.foreign_fname;}) != ffields->end();
        if (!exist) {
          init_error.SetError(ERROR_DB_REFER_FIELD,
              "Неверное имя внешнего поля для reference.\n"
              "Таблица - " + get_table_name(table) + "\n"
              "Внешняя таблица - " + get_table_name(tref.foreign_table) + "\n"
              + STRING_DEBUG_INFO);
          break;
        }
      } else {
        init_error.SetError(ERROR_DB_REFER_FIELD,
            "Неверное собственное имя поля для reference" + STRING_DEBUG_INFO);
        break;
      }
    }
  }
}

/* last update 2020.03.15: добавились флаги моделей, они
 *   учитываются в версии модели */
// static_assert(sizeof(model_info) == 104, "Необходимо перепроверить "
//     "функцию table_model_info() - вероятно изменился формат струтуры данных "
//     "model_info добавьте новые поля, или измените старые");
/** \brief функция собирающая набор полей для
  *   создания таблицы БД model_info информации о модели */
static const db_table_create_setup &table_create_model_info() {
  return ns_tfs::model_info_create_setup;
}

// static_assert(sizeof(calculation_info) == 24, "См static_assert "
//     "для table_cteate_model_info, идея таже");
/** \brief функция собирающая набор полей для
  *   создания таблицы БД calculation_info информации о расчёте */
static const db_table_create_setup &table_create_calculation_info() {
  return ns_tfs::calculation_info_create_setup;
}

// static_assert(sizeof(calculation_state_log) == 112, "См static_assert "
//     "для table_cteate_model_info, идея таже");
/** \brief функция собирающая набор полей для
  *   создания таблицы БД calculation_state_log строку расчёта */
static const db_table_create_setup &table_create_calculation_state_log() {
  return ns_tfs::calculation_state_log_create_setup;
}

/* todo: maybe replace with pointer */
const db_table_create_setup &db_table_create_setup::
    get_table_create_setup(db_table dt) {
  switch (dt) {
    case db_table::table_model_info:
      return table_create_model_info();
    case db_table::table_calculation_info:
      return table_create_calculation_info();
    case db_table::table_calculation_state_log:
      return table_create_calculation_state_log();
  }
  assert(0 && "undef table");
}


/* db_table_select_setup */
db_table_update_setup *db_table_update_setup::Init(db_table table) {
  return new db_table_update_setup(
      table, *ns_tfs::get_fields_collection(table));
}

db_table_update_setup::db_table_update_setup(db_table table,
    const db_fields_collection &fields)
  : table(table), fields(fields) {}

void db_table_update_setup::SetUpdateSetup(
    db_update_t up, model_info &select_data) {
  update_t = up;
  values.clear();
  field_index i = -1;
  if (select_data.initialized & model_info::f_model_type) {
    if ((i = indexByFieldName(MI_MODEL_TYPE)) != field_index_end)
      values.emplace(i, std::to_string(
          (int)select_data.short_info.model_type.type));
    if ((i = indexByFieldName(MI_MODEL_SUBTYPE)) != field_index_end)
      values.emplace(i, std::to_string(
          select_data.short_info.model_type.subtype));
  }
  if (select_data.initialized & model_info::f_vers_major)
    if ((i = indexByFieldName(MI_VERS_MAJOR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.short_info.vers_major));
  if (select_data.initialized & model_info::f_vers_minor)
    if ((i = indexByFieldName(MI_VERS_MINOR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.short_info.vers_minor));
  if (select_data.initialized & model_info::f_short_info)
    if ((i = indexByFieldName(MI_SHORT_INFO)) != field_index_end)
      values.emplace(i, select_data.short_info.short_info);
}
void db_table_update_setup::SetUpdateSetup(
    db_update_t up, calculation_info &select_data) {
  update_t = up;
  values.clear();
  field_index i = -1;
  if (select_data.initialized & calculation_info::f_model_id)
    if (select_data.model != nullptr)
      if ((i = indexByFieldName(CI_MODEL_INFO_ID)) != field_index_end)
        values.emplace(i, std::to_string(select_data.model->id));
  if (select_data.initialized & calculation_info::f_date)
    if ((i = indexByFieldName(CI_DATE)) != field_index_end)
      values.emplace(i, select_data.GetDate());
  if (select_data.initialized & calculation_info::f_time)
    if ((i = indexByFieldName(CI_TIME)) != field_index_end)
      values.emplace(i, select_data.GetTime());
}
void db_table_update_setup::SetUpdateSetup(
    db_update_t up, calculation_state_info &select_data) {
  update_t = up;
  values.clear();
  field_index i = -1;
  if (select_data.initialized & calculation_state_info::f_info)
    if (select_data.calculation != nullptr)
      if ((i = indexByFieldName(CSL_INFO_ID)) != field_index_end)
        values.emplace(i, std::to_string(select_data.calculation->id));
  if (select_data.initialized & calculation_state_info::f_vol)
    if ((i = indexByFieldName(CSL_VOLUME)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.volume));
  if (select_data.initialized & calculation_state_info::f_pres)
    if ((i = indexByFieldName(CSL_PRESSURE)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.pressure));
  if (select_data.initialized & calculation_state_info::f_temp)
    if ((i = indexByFieldName(CSL_TEMPERATURE)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.temperature));
  if (select_data.initialized & calculation_state_info::f_dcv)
    if ((i = indexByFieldName(CSL_HEAT_CV)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.heat_cap_vol));
  if (select_data.initialized & calculation_state_info::f_dcp)
    if ((i = indexByFieldName(CSL_HEAT_CP)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.heat_cap_pres));
  if (select_data.initialized & calculation_state_info::f_din)
    if ((i = indexByFieldName(CSL_INTERNAL_ENERGY)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.internal_energy));
  if (select_data.initialized & calculation_state_info::f_dbk)
    if ((i = indexByFieldName(CSL_BETA_KR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.beta_kr));
  if (select_data.initialized & calculation_state_info::f_enthalpy)
    if ((i = indexByFieldName(CSL_ENTHALPY)) != field_index_end)
      values.emplace(i, std::to_string(select_data.enthalpy));
  if (select_data.initialized & calculation_state_info::f_state_phase)
    if ((i = indexByFieldName(CSL_STATE_PHASE)) != field_index_end)
      values.emplace(i, select_data.state_phase);
}

db_table_update_setup::field_index db_table_update_setup::
    indexByFieldName(const std::string &fname) {
  field_index i = 0;
  for (auto const &x : fields) {
    if (x.fname == fname)
      break;
    ++i;
  }
  return (i == fields.size()) ? -1 : i;
}
