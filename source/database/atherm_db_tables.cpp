/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "atherm_db_tables.h"

#include "asp_db/db_connection_manager.h"
#include "models_configurations.h"

#include <map>
#include <memory>

#include <assert.h>

#define tables_pair(x, y) \
  { x, y }

namespace table_fields_setup {
static std::map<db_table, std::string> str_tables =
    std::map<db_table, std::string>{
        tables_pair(table_model_info, "model_info"),
        tables_pair(table_calculation_info, "calculation_info"),
        tables_pair(table_calculation_state_log, "calculation_state_log"),
    };

/** \brief Сетап таблицы БД хранения данных о модели
 * \note В семантике PostgreSQL */
/* SQL_QUERY:
TABLE MODEL_INFO (
  model_id autoinc,
  model_type uint NOT NULL,
  model_subtype uint,
  vers_major uint not NULL,
  vers_minor uint,
  short_info string,
  UNIQUE(model_type, model_subtype, vers_major, vers_minor),
  PRIMARY KEY (model_id)
); */
//
// db_variable::db_variable_flags(
//    {{"is_primary_key", true}, {"can_be_null", false}})),
const db_fields_collection model_info_fields = {
    db_variable(TABLE_FIELD_PAIR(MI_MODEL_ID),
                db_variable_type::type_autoinc,
                db_variable::db_variable_flags(
                    {{"is_primary_key", true}, {"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(MI_MODEL_TYPE),
                db_variable_type::type_int,
                db_variable::db_variable_flags({{"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(MI_MODEL_SUBTYPE),
                db_variable_type::type_int,
                db_variable::db_variable_flags{}),
    db_variable(TABLE_FIELD_PAIR(MI_VERS_MAJOR),
                db_variable_type::type_int,
                db_variable::db_variable_flags({{"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(MI_VERS_MINOR),
                db_variable_type::type_int,
                db_variable::db_variable_flags{}),
    db_variable(TABLE_FIELD_PAIR(MI_SHORT_INFO),
                db_variable_type::type_text,
                db_variable::db_variable_flags{})};
static const db_table_create_setup::uniques_container mi_uniques = {
    {TABLE_FIELD_NAME(MI_MODEL_TYPE), TABLE_FIELD_NAME(MI_MODEL_SUBTYPE),
     TABLE_FIELD_NAME(MI_VERS_MAJOR), TABLE_FIELD_NAME(MI_VERS_MINOR)}};
static const db_table_create_setup model_info_create_setup(table_model_info,
                                                           model_info_fields,
                                                           mi_uniques,
                                                           nullptr);

/* SQL_QUERY:
TABLE CALCULATION_INFO (
  calculation_id autoinc,
  model_info_id int,
  date date,
  time time,
  text gasmix_file,
  UNIQUE(model_info_d, date, time),
  PRIMARY KEY (calculation_id),
  FOREIGN KEY (model_info_id) REFERENCES model_info(model_id)
); */
const db_fields_collection calculation_info_fields = {
    db_variable(TABLE_FIELD_PAIR(CI_CALCULATION_ID),
                db_variable_type::type_autoinc,
                db_variable::db_variable_flags(
                    {{"is_primary_key", true}, {"can_be_null", false}})),
    // reference to model_info(fk)
    db_variable(TABLE_FIELD_PAIR(CI_MODEL_INFO_ID),
                db_variable_type::type_int,
                db_variable::db_variable_flags(
                    {{"is_reference", true}, {"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(CI_DATE),
                db_variable_type::type_date,
                db_variable::db_variable_flags({{"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(CI_TIME),
                db_variable_type::type_time,
                db_variable::db_variable_flags({{"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(CI_GASMIX_FILE),
                db_variable_type::type_text,
                db_variable::db_variable_flags({}))};
static const db_table_create_setup::uniques_container ci_uniques = {
    {{TABLE_FIELD_NAME(CI_MODEL_INFO_ID), TABLE_FIELD_NAME(CI_DATE),
      TABLE_FIELD_NAME(CI_TIME), TABLE_FIELD_NAME(CI_GASMIX_FILE)}}};
static const std::shared_ptr<db_ref_collection> calculation_info_references(
    new db_ref_collection{db_reference(TABLE_FIELD_NAME(CI_MODEL_INFO_ID),
                                       table_model_info,
                                       TABLE_FIELD_NAME(MI_MODEL_ID),
                                       true)});
static const db_table_create_setup calculation_info_create_setup(
    table_calculation_info,
    calculation_info_fields,
    ci_uniques,
    calculation_info_references);

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
  enthalpy real,
  adiabatic real,
  beta_kr real,
  entropy real,
  state_phase char(12)
  PRIMARY KEY (calculation_log_id, calculation_info_id),
  FOREIGN KEY (calculation_info_id)
      REFERENCES calculation_info(calculation_id) ON DELETE CASCADE
); */
const db_fields_collection calculation_state_log_fields = {
    db_variable(TABLE_FIELD_PAIR(CSL_LOG_ID),
                db_variable_type::type_autoinc,
                db_variable::db_variable_flags(
                    {{"is_primary_key", true}, {"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(CSL_INFO_ID),
                db_variable_type::type_int,
                db_variable::db_variable_flags({{"is_primary_key", true},
                                                {"is_reference", true},
                                                {"can_be_null", false}})),
    db_variable(TABLE_FIELD_PAIR(CSL_VOLUME), db_variable_type::type_real, {}),
    db_variable(TABLE_FIELD_PAIR(CSL_PRESSURE),
                db_variable_type::type_real,
                db_variable::db_variable_flags({})),
    db_variable(TABLE_FIELD_PAIR(CSL_TEMPERATURE),
                db_variable_type::type_real,
                db_variable::db_variable_flags({})),
    db_variable(TABLE_FIELD_PAIR(CSL_HEAT_CV), db_variable_type::type_real, {}),
    db_variable(TABLE_FIELD_PAIR(CSL_HEAT_CP), db_variable_type::type_real, {}),
    db_variable(TABLE_FIELD_PAIR(CSL_INTERNAL_ENERGY),
                db_variable_type::type_real,
                {}),
    db_variable(TABLE_FIELD_PAIR(CSL_ENTHALPY),
                db_variable_type::type_real,
                {}),
    db_variable(TABLE_FIELD_PAIR(CSL_ADIABATIC),
                db_variable_type::type_real,
                {}),
    db_variable(TABLE_FIELD_PAIR(CSL_BETA_KR), db_variable_type::type_real, {}),
    db_variable(TABLE_FIELD_PAIR(CSL_ENTROPY), db_variable_type::type_real, {}),
    db_variable(TABLE_FIELD_PAIR(CSL_STATE_PHASE),
                db_variable_type::type_char_array,
                db_variable::db_variable_flags({{"is_array", true}}),
                12)};
static const std::shared_ptr<db_ref_collection>
    calculation_state_log_references = std::shared_ptr<db_ref_collection>(
        new db_ref_collection{db_reference(TABLE_FIELD_NAME(CSL_INFO_ID),
                                           table_calculation_info,
                                           TABLE_FIELD_NAME(CI_CALCULATION_ID),
                                           true,
                                           db_reference_act::ref_act_cascade,
                                           db_reference_act::ref_act_cascade)});
static const db_table_create_setup::uniques_container sci_uniques = {};
static const db_table_create_setup calculation_state_log_create_setup(
    table_calculation_state_log,
    calculation_state_log_fields,
    sci_uniques,
    calculation_state_log_references);
}  // namespace table_fields_setup

namespace ns_tfs = table_fields_setup;

/* last update 2020.03.15: добавились флаги моделей, они
 *   учитываются в версии модели */
// static_assert(sizeof(model_info) == 104, "Необходимо перепроверить "
//     "функцию table_model_info() - вероятно изменился формат струтуры данных "
//     "model_info добавьте новые поля, или измените старые");
/** \brief функция собирающая набор полей для
 *   создания таблицы БД model_info информации о модели */
static const db_table_create_setup& table_create_model_info() {
  return ns_tfs::model_info_create_setup;
}

// static_assert(sizeof(calculation_info) == 24, "См static_assert "
//     "для table_cteate_model_info, идея таже");
/** \brief функция собирающая набор полей для
 *   создания таблицы БД calculation_info информации о расчёте */
static const db_table_create_setup& table_create_calculation_info() {
  return ns_tfs::calculation_info_create_setup;
}

// static_assert(sizeof(calculation_state_log) == 112, "См static_assert "
//     "для table_cteate_model_info, идея таже");
/** \brief функция собирающая набор полей для
 *   создания таблицы БД calculation_state_log строку расчёта */
static const db_table_create_setup& table_create_calculation_state_log() {
  return ns_tfs::calculation_state_log_create_setup;
}

std::string AthermDBTables::GetTableName(db_table t) const {
  auto x = ns_tfs::str_tables.find(t);
  return (x != ns_tfs::str_tables.end()) ? x->second : "";
}

const db_fields_collection* AthermDBTables::GetFieldsCollection(
    db_table dt) const {
  const db_fields_collection* result = nullptr;
  switch (dt) {
    case table_model_info:
      result = &ns_tfs::model_info_fields;
      break;
    case table_calculation_info:
      result = &ns_tfs::calculation_info_fields;
      break;
    case table_calculation_state_log:
      result = &ns_tfs::calculation_state_log_fields;
      break;
    case table_undefiend:
    default:
      throw DBException(ERROR_DB_TABLE_EXISTS, "Неизвестный код таблицы");
      break;
  }
  return result;
}

db_table AthermDBTables::StrToTableCode(const std::string& tname) const {
  for (const auto& x : ns_tfs::str_tables)
    if (x.second == tname)
      return x.first;
  return table_undefiend;
}

std::string AthermDBTables::GetIdColumnName(db_table dt) const {
  std::string name = "";
  switch (dt) {
    case table_model_info:
      name = TABLE_FIELD_NAME(MI_MODEL_ID);
      break;
    case table_calculation_info:
      name = TABLE_FIELD_NAME(CI_CALCULATION_ID);
      break;
    case table_calculation_state_log:
      name = TABLE_FIELD_NAME(CSL_LOG_ID);
      break;
    case table_undefiend:
      break;
  }
  return name;
}

const db_table_create_setup& AthermDBTables::CreateSetupByCode(
    db_table dt) const {
  switch (dt) {
    case table_model_info:
      return table_create_model_info();
    case table_calculation_info:
      return table_create_calculation_info();
    case table_calculation_state_log:
      return table_create_calculation_state_log();
    case table_undefiend:
    default:
      throw DBException(ERROR_DB_TABLE_EXISTS, "Неизвестный код таблицы");
  }
}

/*
db_ref_collection AthermDBTables::RefCollectionByCode(db_table table) {
  switch (table) {
    case table_model_info:
      return db_ref_collection();
      break;
    case table_calculation_info:
      return ns_tfs::calculation_info_references;
      break;
    case table_calculation_state_log:
      return ns_tfs::calculation_state_log_references;
      break;
    case table_undefiend:
    default:
      throw db_exception(ERROR_DB_TABLE_EXISTS, "Неизвестный код таблицы");
      break;
  }
}
*/

template <>
std::string IDBTables::GetTableName<model_info>() const {
  auto x = ns_tfs::str_tables.find(table_model_info);
  return (x != ns_tfs::str_tables.end()) ? x->second : "";
}
template <>
std::string IDBTables::GetTableName<calculation_info>() const {
  auto x = ns_tfs::str_tables.find(table_calculation_info);
  return (x != ns_tfs::str_tables.end()) ? x->second : "";
}
template <>
std::string IDBTables::GetTableName<calculation_state_log>() const {
  auto x = ns_tfs::str_tables.find(table_calculation_state_log);
  return (x != ns_tfs::str_tables.end()) ? x->second : "";
}

template <>
db_table IDBTables::GetTableCode<model_info>() const {
  return table_model_info;
}
template <>
db_table IDBTables::GetTableCode<calculation_info>() const {
  return table_calculation_info;
}
template <>
db_table IDBTables::GetTableCode<calculation_state_log>() const {
  return table_calculation_state_log;
}

template <>
void IDBTables::setInsertValues<model_info>(
    db_query_insert_setup* src,
    const model_info& select_data) const {
  if (select_data.initialized == model_info::f_empty)
    return;
  db_query_basesetup::row_values values;
  db_query_basesetup::field_index i;
  insert_macro(model_info::f_model_id, MI_MODEL_ID,
               std::to_string(select_data.id));
  insert_macro(model_info::f_model_type, MI_MODEL_TYPE,
               std::to_string((int)select_data.short_info.model_type.type));
  insert_macro(model_info::f_model_subtype, MI_MODEL_SUBTYPE,
               std::to_string(select_data.short_info.model_type.subtype));
  insert_macro(model_info::f_vers_major, MI_VERS_MAJOR,
               std::to_string(select_data.short_info.vers_major));
  insert_macro(model_info::f_vers_minor, MI_VERS_MINOR,
               std::to_string(select_data.short_info.vers_minor));
  insert_macro(model_info::f_short_info, MI_SHORT_INFO,
               select_data.short_info.short_info);
  src->values_vec.emplace_back(values);
}

template <>
void IDBTables::setInsertValues<calculation_info>(
    db_query_insert_setup* src,
    const calculation_info& select_data) const {
  if (select_data.initialized == calculation_info::f_empty)
    return;
  db_query_basesetup::row_values values;
  db_query_basesetup::field_index i;
  if (select_data.model != nullptr)
    if (select_data.model->id >= 0
        && (i = src->IndexByFieldId(CI_MODEL_INFO_ID))
               != db_query_basesetup::field_index_end)
      values.emplace(i, std::to_string(select_data.model->id));

  insert_macro(calculation_info::f_calculation_info_id, CI_CALCULATION_ID,
               std::to_string(select_data.id));
  insert_macro(calculation_info::f_model_id, CI_MODEL_INFO_ID,
               std::to_string(select_data.model_id));
  insert_macro(calculation_info::f_date, CI_DATE, select_data.GetDate());
  insert_macro(calculation_info::f_time, CI_TIME, select_data.GetTime());
  insert_macro(calculation_info::f_gasmix, CI_GASMIX_FILE,
               select_data.gasmix_file);
  src->values_vec.emplace_back(values);
}

template <>
void IDBTables::setInsertValues<calculation_state_log>(
    db_query_insert_setup* src,
    const calculation_state_log& select_data) const {
  if (select_data.initialized == calculation_state_log::f_empty)
    return;
  db_query_basesetup::row_values values;
  db_query_basesetup::field_index i;
  if (select_data.calculation != nullptr)
    if (select_data.calculation->id >= 0
        && (i = src->IndexByFieldId(CSL_INFO_ID))
               != db_query_basesetup::field_index_end)
      values.emplace(i, std::to_string(select_data.calculation->id));

  insert_macro(calculation_state_log::f_calculation_state_log_id, CSL_LOG_ID,
               std::to_string(select_data.id));
  insert_macro(calculation_state_log::f_calculation_info_id, CSL_INFO_ID,
               std::to_string(select_data.info_id));
  insert_macro(calculation_state_log::f_vol, CSL_VOLUME,
               std::to_string(select_data.dyn_pars.parm.volume));
  insert_macro(calculation_state_log::f_pres, CSL_PRESSURE,
               std::to_string(select_data.dyn_pars.parm.pressure));
  insert_macro(calculation_state_log::f_temp, CSL_TEMPERATURE,
               std::to_string(select_data.dyn_pars.parm.temperature));
  insert_macro(calculation_state_log::f_dcv, CSL_HEAT_CV,
               std::to_string(select_data.dyn_pars.heat_cap_vol));
  insert_macro(calculation_state_log::f_dcp, CSL_HEAT_CP,
               std::to_string(select_data.dyn_pars.heat_cap_pres));
  insert_macro(calculation_state_log::f_din, CSL_INTERNAL_ENERGY,
               std::to_string(select_data.dyn_pars.internal_energy));
  insert_macro(calculation_state_log::f_denthalpy, CSL_ENTHALPY,
               std::to_string(select_data.dyn_pars.enthalpy));
  insert_macro(calculation_state_log::f_dadiabatic, CSL_ADIABATIC,
               std::to_string(select_data.dyn_pars.adiabatic));
  insert_macro(calculation_state_log::f_dbk, CSL_BETA_KR,
               std::to_string(select_data.dyn_pars.beta_kr));
  insert_macro(calculation_state_log::f_dentropy, CSL_ENTROPY,
               std::to_string(select_data.dyn_pars.entropy));
  insert_macro(calculation_state_log::f_state_phase, CSL_STATE_PHASE,
               select_data.state_phase);
  src->values_vec.emplace_back(values);
}

template <>
void IDBTables::SetSelectData<model_info>(
    db_query_select_result* src,
    std::vector<model_info>* out_vec) const {
  for (auto& row : src->values_vec) {
    model_info mi = model_info::GetDefault();
    for (auto& col : row) {
      if (src->isFieldName(TABLE_FIELD_NAME(MI_MODEL_ID),
                           src->fields[col.first])) {
        mi.id = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_model_id;
      } else if (src->isFieldName(TABLE_FIELD_NAME(MI_MODEL_TYPE),
                                  src->fields[col.first])) {
        mi.short_info.model_type.type =
            (rg_model_t)std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_model_type;
      } else if (src->isFieldName(TABLE_FIELD_NAME(MI_MODEL_SUBTYPE),
                                  src->fields[col.first])) {
        mi.short_info.model_type.subtype = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_model_subtype;
      } else if (src->isFieldName(TABLE_FIELD_NAME(MI_VERS_MAJOR),
                                  src->fields[col.first])) {
        mi.short_info.vers_major = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_vers_major;
      } else if (src->isFieldName(TABLE_FIELD_NAME(MI_VERS_MINOR),
                                  src->fields[col.first])) {
        mi.short_info.vers_minor = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_vers_minor;
      } else if (src->isFieldName(TABLE_FIELD_NAME(MI_SHORT_INFO),
                                  src->fields[col.first])) {
        mi.short_info.short_info = col.second.c_str();
        mi.initialized |= model_info::f_short_info;
      }
    }
    if (mi.initialized != mi.f_empty)
      out_vec->push_back(std::move(mi));
  }
}

template <>
void IDBTables::SetSelectData<calculation_info>(
    db_query_select_result* src,
    std::vector<calculation_info>* out_vec) const {
  for (auto& row : src->values_vec) {
    calculation_info ci;
    for (auto& col : row) {
      if (src->isFieldName(TABLE_FIELD_NAME(CI_CALCULATION_ID),
                           src->fields[col.first])) {
        ci.id = std::atoi(col.second.c_str());
        ci.initialized |= calculation_info::f_calculation_info_id;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CI_MODEL_INFO_ID),
                                  src->fields[col.first])) {
        ci.model_id = std::atoi(col.second.c_str());
        ci.initialized |= calculation_info::f_model_id;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CI_DATE),
                                  src->fields[col.first])) {
        ci.SetDate(col.second);
      } else if (src->isFieldName(TABLE_FIELD_NAME(CI_TIME),
                                  src->fields[col.first])) {
        ci.SetTime(col.second);
      } else if (src->isFieldName(TABLE_FIELD_NAME(CI_GASMIX_FILE),
                                  src->fields[col.first])) {
        ci.SetGasmixFile(col.second);
      }
    }
    if (ci.initialized != ci.f_empty)
      out_vec->push_back(std::move(ci));
  }
}

template <>
void IDBTables::SetSelectData<calculation_state_log>(
    db_query_select_result* src,
    std::vector<calculation_state_log>* out_vec) const {
  for (auto& row : src->values_vec) {
    calculation_state_log cl;
    for (auto& col : row) {
      if (src->isFieldName(TABLE_FIELD_NAME(CSL_LOG_ID),
                           src->fields[col.first])) {
        cl.id = std::atoi(col.second.c_str());
        cl.initialized |= calculation_state_log::f_calculation_state_log_id;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_INFO_ID),
                                  src->fields[col.first])) {
        cl.info_id = std::atoi(col.second.c_str());
        cl.initialized |= calculation_state_log::f_calculation_info_id;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_VOLUME),
                                  src->fields[col.first])) {
        cl.dyn_pars.parm.volume = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_vol;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_PRESSURE),
                                  src->fields[col.first])) {
        cl.dyn_pars.parm.pressure = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_pres;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_TEMPERATURE),
                                  src->fields[col.first])) {
        cl.dyn_pars.parm.temperature = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_temp;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_HEAT_CV),
                                  src->fields[col.first])) {
        cl.dyn_pars.heat_cap_vol = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_dcv;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_HEAT_CP),
                                  src->fields[col.first])) {
        cl.dyn_pars.heat_cap_pres = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_dcp;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_INTERNAL_ENERGY),
                                  src->fields[col.first])) {
        cl.dyn_pars.internal_energy = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_din;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_ENTHALPY),
                                  src->fields[col.first])) {
        cl.dyn_pars.enthalpy = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_denthalpy;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_ADIABATIC),
                                  src->fields[col.first])) {
        cl.dyn_pars.adiabatic = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_denthalpy;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_BETA_KR),
                                  src->fields[col.first])) {
        cl.dyn_pars.beta_kr = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_dbk;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_ENTROPY),
                                  src->fields[col.first])) {
        cl.dyn_pars.entropy = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_denthalpy;
      } else if (src->isFieldName(TABLE_FIELD_NAME(CSL_STATE_PHASE),
                                  src->fields[col.first])) {
        cl.state_phase = col.second;
        cl.initialized |= calculation_state_log::f_state_phase;
      }
    }
    if (cl.initialized != cl.f_empty)
      out_vec->push_back(std::move(cl));
  }
}
