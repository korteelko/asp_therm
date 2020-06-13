/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_queries_setup.h"

#include "models_configurations.h"
#include "Logging.h"

#include <algorithm>
#include <stack>

#include <assert.h>


namespace table_fields_setup {
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
const db_fields_collection model_info_fields = {
  db_variable(TABLE_FIELD_PAIR(MI_MODEL_ID), db_type::type_autoinc,
      { .is_primary_key = true, .is_unique = true, .can_be_null = false }),
  db_variable(TABLE_FIELD_PAIR(MI_MODEL_TYPE), db_type::type_int,
      { .can_be_null = false }),
  db_variable(TABLE_FIELD_PAIR(MI_MODEL_SUBTYPE), db_type::type_int, {}),
  db_variable(TABLE_FIELD_PAIR(MI_VERS_MAJOR), db_type::type_int,
      { .can_be_null = false }),
  db_variable(TABLE_FIELD_PAIR(MI_VERS_MINOR), db_type::type_int, {}),
  db_variable(TABLE_FIELD_PAIR(MI_SHORT_INFO), db_type::type_text, {})
};
static const db_table_create_setup::uniques_container mi_uniques =
    { { TABLE_FIELD_NAME(MI_MODEL_TYPE), TABLE_FIELD_NAME(MI_MODEL_SUBTYPE),
    TABLE_FIELD_NAME(MI_VERS_MAJOR), TABLE_FIELD_NAME(MI_VERS_MINOR) } };
static const db_table_create_setup model_info_create_setup(
    db_table::table_model_info, model_info_fields, mi_uniques);

/* SQL_QUERY:
TABLE CALCULATION_INFO (
  calculation_id autoinc,
  model_info_id int,
  date date,
  time time,
  UNIQUE(model_info_d, date, time),
  PRIMARY KEY (calculation_id),
  FOREIGN KEY (model_info_id) REFERENCES model_info(model_id)
); */
const db_fields_collection calculation_info_fields = {
  db_variable(TABLE_FIELD_PAIR(CI_CALCULATION_ID), db_type::type_autoinc,
      { .is_primary_key = true, .is_unique = true, .can_be_null = false }),
  // reference to model_info(fk)
  db_variable(TABLE_FIELD_PAIR(CI_MODEL_INFO_ID), db_type::type_int,
      { .is_reference = true, .can_be_null = false }),
  db_variable(TABLE_FIELD_PAIR(CI_DATE), db_type::type_date,
      { .can_be_null = false }),
  db_variable(TABLE_FIELD_PAIR(CI_TIME), db_type::type_time,
      { .can_be_null = false })
};
static const db_table_create_setup::uniques_container ci_uniques = {
    { { TABLE_FIELD_NAME(CI_MODEL_INFO_ID), TABLE_FIELD_NAME(CI_DATE),
    TABLE_FIELD_NAME(CI_TIME) } }
};
static const db_ref_collection calculation_info_references = {
  db_reference(TABLE_FIELD_NAME(CI_MODEL_INFO_ID), db_table::table_model_info,
      TABLE_FIELD_NAME(MI_MODEL_ID), true)
};
static const db_table_create_setup calculation_info_create_setup(
    db_table::table_calculation_info, calculation_info_fields, ci_uniques);

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
const db_fields_collection calculation_state_log_fields = {
  db_variable(TABLE_FIELD_PAIR(CSL_LOG_ID), db_type::type_autoinc, {.is_primary_key = true,
      .is_unique = true, .can_be_null = false}),
  db_variable(TABLE_FIELD_PAIR(CSL_INFO_ID), db_type::type_int, {.is_primary_key = true,
      .is_reference = true, .can_be_null = false}),
  db_variable(TABLE_FIELD_PAIR(CSL_VOLUME), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_PRESSURE), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_TEMPERATURE), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_HEAT_CV), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_HEAT_CP), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_INTERNAL_ENERGY), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_BETA_KR), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_ENTHALPY), db_type::type_real, {}),
  db_variable(TABLE_FIELD_PAIR(CSL_STATE_PHASE), db_type::type_char_array, {.is_array = true}, 12)
};
static const db_ref_collection calculation_state_log_references = {
  db_reference(TABLE_FIELD_NAME(CSL_INFO_ID), db_table::table_calculation_info,
      TABLE_FIELD_NAME(CI_CALCULATION_ID), true,
      db_reference_act::ref_act_cascade, db_reference_act::ref_act_cascade)
};
static const db_table_create_setup::uniques_container sci_uniques = {};
static const db_table_create_setup calculation_state_log_create_setup(
    db_table::table_calculation_state_log,
    calculation_state_log_fields, sci_uniques);

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


/* db_save_point */
db_save_point::db_save_point(const std::string &_name)
  : name("") {
  std::string tmp = trim_str(_name);
  if (!tmp.empty()) {
    if (std::isalpha(tmp[0]) || tmp[0] == '_') {
      // проверить наличие пробелов и знаков разделения
      auto sep_pos = std::find_if_not(tmp.begin(), tmp.end(),
          [](char c) { return std::iswalnum(c) || c == '_'; });
      if (sep_pos == tmp.end())
        name = tmp;
    }
  }
  if (name.empty()) {
    Logging::Append(ERROR_DB_SAVE_POINT,
        "Ошибка инициализации параметров точки сохранения состояния БД\n"
        "Строка инициализации: " + _name);
  }
}

std::string db_save_point::GetString() const {
  return name;
}

/* db_condition_tree */
db_condition_node::db_condition_node(db_operator_t db_operator)
  : data({db_type::type_empty, "", ""}), db_operator(db_operator),
    is_leafnode(false) {}

db_condition_node::db_condition_node(db_type t, const std::string &fname,
    const std::string &data)
  : data({t, fname, data}), db_operator(db_operator_t::op_empty), is_leafnode(true) {}

db_condition_node::~db_condition_node() {}

std::string db_condition_node::DataToStr(db_type t, const std::string &f,
    const std::string &v) {
  return (t == db_type::type_char_array || t == db_type::type_text) ?
      f + " = '" + v + "'": f + " = " + v;
}

std::string db_condition_node::GetString(DataToStrF dts) const {
  std::string l, r;
  std::string result;
  visited = true;
  // если поддеревьев нет, собрать строку
  if (db_operator == db_operator_t::op_empty)
    return dts(data.type, data.field_name, data.field_value);
    //return data.field_name + " = " + data.field_value;
  if (left)
    if (!left->visited)
      l = left->GetString(dts);
  if (rigth)
    if (!rigth->visited)
      r = rigth->GetString(dts);
  switch (db_operator) {
    case db_operator_t::op_is:
      result = l +  " IS " + r;
      break;
    case db_operator_t::op_not:
      result = l +  " IS NOT " + r;
      break;
    case db_operator_t::op_in:
      result = l +  " IN " + r;
      break;
    case db_operator_t::op_like:
      result = l +  " LIKE " + r;
      break;
    case db_operator_t::op_between:
      result = l +  " BETWEEN " + r;
      break;
    case db_operator_t::op_and:
      result = l +  " AND " + r;
      break;
    case db_operator_t::op_or:
      result = l +  " OR " + r;
      break;
    case db_operator_t::op_eq:
      result = l +  " = " + r;
      break;
    case db_operator_t::op_ne:
      result = l +  " != " + r;
      break;
    case db_operator_t::op_ge:
      result = l +  " >= " + r;
      break;
    case db_operator_t::op_gt:
      result = l +  " > " + r;
      break;
    case db_operator_t::op_le:
      result = l +  " <= " + r;
      break;
    case db_operator_t::op_lt:
      result = l +  " < " + r;
      break;
    case db_operator_t::op_empty:
      assert(0 && "check program DNA - it's case unavailable");
      break;
    // case db_operator_t::op_eq:
  }
  return result;
}

/* db_table_create_setup */
db_table_create_setup::db_table_create_setup(db_table table)
  : table(table) {}

db_table_create_setup::db_table_create_setup(db_table table,
    const db_fields_collection &fields, const uniques_container &unique_constrains)
  : table(table), fields(fields), unique_constrains(unique_constrains) {
  switch (table) {
    case db_table::table_model_info:
      break;
    case db_table::table_calculation_info:
      ref_strings = ns_tfs::calculation_info_references;
      break;
    case db_table::table_calculation_state_log:
      ref_strings = ns_tfs::calculation_state_log_references;
      break;
    default:
      assert(0 && "undef table");
      break;
  }
  setupPrimaryKeyString();
  checkReferences();
  if (error.GetErrorCode()) {
    error.LogIt();
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

void db_table_create_setup::checkReferences() {
  if (!ref_strings.empty()) {
    for (auto const &tref: ref_strings) {
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
          error.SetError(ERROR_DB_REFER_FIELD,
              "Неверное имя внешнего поля для reference.\n"
              "Таблица - " + get_table_name(table) + "\n"
              "Внешняя таблица - " + get_table_name(tref.foreign_table) + "\n"
              + STRING_DEBUG_INFO);
          break;
        }
      } else {
        error.SetError(ERROR_DB_REFER_FIELD,
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

std::map<db_table_create_setup::compare_field, bool>
     db_table_create_setup::Compare(const db_table_create_setup &r) {
  std::map<db_table_create_setup::compare_field, bool> result;
  result.emplace(cf_table, r.table == table);
  // fields
  result.emplace(cf_fields, db_table_create_setup::IsSame(fields, r.fields));

  // pk_string
  result.emplace(cf_pk_string, db_table_create_setup::IsSame(
      pk_string.fnames, r.pk_string.fnames));

  // unique_constrains
  result.emplace(cf_unique_constrains, db_table_create_setup::IsSame(
      unique_constrains, r.unique_constrains));

  // ref_strings
  result.emplace(cf_unique_constrains, db_table_create_setup::IsSame(
      ref_strings, r.ref_strings));
  return result;
}

/* db_table_query_basesetup */
db_query_basesetup::db_query_basesetup(
    db_table table, const db_fields_collection &fields)
  : table(table), fields(fields) {}

db_query_basesetup::field_index db_query_basesetup::
    IndexByFieldId(db_variable_id fid) {
  field_index i = 0;
  for (auto const &x: fields) {
    if (x.fid == fid)
      break;
    ++i;
  }
  return (i == fields.size()) ? db_query_basesetup::field_index_end : i;
}

/* db_table_insert_setup */
db_query_insert_setup::db_query_insert_setup(db_table _table,
    const db_fields_collection &_fields)
  : db_query_basesetup(_table, _fields) {}

/* todo: сделать шаблон, или вынести в шабло */
db_query_insert_setup *db_query_insert_setup::Init(
    const std::vector<model_info> &insert_data) {
  if (haveConflict(insert_data))
    return nullptr;
  db_query_insert_setup *ins_setup = new db_query_insert_setup(
      db_table::table_model_info, *ns_tfs::get_fields_collection(
      db_table::table_model_info));
  if (ins_setup)
    for (const auto &x: insert_data)
      ins_setup->setValues(x);
  return ins_setup;
}

db_query_insert_setup *db_query_insert_setup::Init(
    const std::vector<calculation_info> &insert_data) {
  if (haveConflict(insert_data))
    return nullptr;
  db_query_insert_setup *ins_setup = new db_query_insert_setup(
      db_table::table_calculation_info, *ns_tfs::get_fields_collection(
      db_table::table_calculation_info));
  if (ins_setup)
    for (const auto &x: insert_data)
      ins_setup->setValues(x);
  return ins_setup;
}

db_query_insert_setup *db_query_insert_setup::Init(
    const std::vector<calculation_state_log> &insert_data) {
  if (haveConflict(insert_data))
    return nullptr;
  db_query_insert_setup *ins_setup = new db_query_insert_setup(
      db_table::table_calculation_state_log, *ns_tfs::get_fields_collection(
      db_table::table_calculation_state_log));
  if (ins_setup)
    for (const auto &x: insert_data)
      ins_setup->setValues(x);
  return ins_setup;
}

size_t db_query_insert_setup::RowsSize() const {
  return values_vec.size();
}

void db_query_insert_setup::setValues(const model_info &select_data) {
  if (select_data.initialized == model_info::f_empty)
    return;
  row_values values;
  field_index i;
  if (select_data.initialized & model_info::f_model_id)
    if ((i = IndexByFieldId(MI_MODEL_ID)) != field_index_end)
      values.emplace(i, std::to_string(select_data.id));
  if (select_data.initialized & model_info::f_model_type)
    if ((i = IndexByFieldId(MI_MODEL_TYPE)) != field_index_end)
      values.emplace(i, std::to_string(
          (int)select_data.short_info.model_type.type));
  if (select_data.initialized & model_info::f_model_subtype)
    if ((i = IndexByFieldId(MI_MODEL_SUBTYPE)) != field_index_end)
      values.emplace(i, std::to_string(
          select_data.short_info.model_type.subtype));
  if (select_data.initialized & model_info::f_vers_major)
    if ((i = IndexByFieldId(MI_VERS_MAJOR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.short_info.vers_major));
  if (select_data.initialized & model_info::f_vers_minor)
    if ((i = IndexByFieldId(MI_VERS_MINOR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.short_info.vers_minor));
  if (select_data.initialized & model_info::f_short_info)
    if ((i = IndexByFieldId(MI_SHORT_INFO)) != field_index_end)
      values.emplace(i, select_data.short_info.short_info);
  values_vec.emplace_back(values);
}

void db_query_insert_setup::setValues(const calculation_info &select_data) {
  if (select_data.initialized == calculation_info::f_empty)
    return;
  row_values values;
  field_index i;
  if (select_data.initialized & calculation_info::f_calculation_info_id)
    if ((i = IndexByFieldId(CI_CALCULATION_ID)) != field_index_end)
      values.emplace(i, std::to_string(select_data.id));
  if (select_data.initialized & calculation_info::f_model_id)
    if ((i = IndexByFieldId(CI_MODEL_INFO_ID)) != field_index_end)
      values.emplace(i, std::to_string(select_data.model_id));
  if (select_data.initialized & calculation_info::f_date)
    if ((i = IndexByFieldId(CI_DATE)) != field_index_end)
      values.emplace(i, select_data.GetDate());
  if (select_data.initialized & calculation_info::f_time)
    if ((i = IndexByFieldId(CI_TIME)) != field_index_end)
      values.emplace(i, select_data.GetTime());
  values_vec.emplace_back(values);
}

void db_query_insert_setup::setValues(const calculation_state_log &select_data) {
  if (select_data.initialized == calculation_state_log::f_empty)
    return;
  row_values values;
  field_index i;
  if (select_data.initialized & calculation_state_log::f_calculation_state_log_id)
    if ((i = IndexByFieldId(CSL_LOG_ID)) != field_index_end)
      values.emplace(i, std::to_string(select_data.id));
  if (select_data.initialized & calculation_state_log::f_calculation_info_id)
    if ((i = IndexByFieldId(CSL_INFO_ID)) != field_index_end)
      values.emplace(i, std::to_string(select_data.info_id));
  if (select_data.initialized & calculation_state_log::f_vol)
    if ((i = IndexByFieldId(CSL_VOLUME)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.volume));
  if (select_data.initialized & calculation_state_log::f_pres)
    if ((i = IndexByFieldId(CSL_PRESSURE)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.pressure));
  if (select_data.initialized & calculation_state_log::f_temp)
    if ((i = IndexByFieldId(CSL_TEMPERATURE)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.temperature));
  if (select_data.initialized & calculation_state_log::f_dcv)
    if ((i = IndexByFieldId(CSL_HEAT_CV)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.heat_cap_vol));
  if (select_data.initialized & calculation_state_log::f_dcp)
    if ((i = IndexByFieldId(CSL_HEAT_CP)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.heat_cap_pres));
  if (select_data.initialized & calculation_state_log::f_din)
    if ((i = IndexByFieldId(CSL_INTERNAL_ENERGY)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.internal_energy));
  if (select_data.initialized & calculation_state_log::f_dbk)
    if ((i = IndexByFieldId(CSL_BETA_KR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.beta_kr));
  if (select_data.initialized & calculation_state_log::f_enthalpy)
    if ((i = IndexByFieldId(CSL_ENTHALPY)) != field_index_end)
      values.emplace(i, std::to_string(select_data.enthalpy));
  if (select_data.initialized & calculation_state_log::f_state_phase)
    if ((i = IndexByFieldId(CSL_STATE_PHASE)) != field_index_end)
      values.emplace(i, select_data.state_phase);
  values_vec.emplace_back(values);
}

/* db_table_select_setup */
db_query_select_setup::db_query_select_setup(db_table _table,
    const db_fields_collection &_fields)
  : db_query_basesetup(_table, _fields) {}

db_query_select_setup *db_query_select_setup::Init(db_table _table) {
  return new db_query_select_setup(_table, *ns_tfs::get_fields_collection(_table));
}

/* db_table_update_setup */
db_query_update_setup *db_query_update_setup::Init(db_table _table) {
  return new db_query_update_setup(_table, *ns_tfs::get_fields_collection(_table));
}

db_query_update_setup::db_query_update_setup(db_table _table,
    const db_fields_collection &_fields)
  : db_query_select_setup(_table, _fields) {}

/* db_table_select_result */
db_query_select_result::db_query_select_result(
    const db_query_select_setup &setup)
  : db_query_basesetup(setup) {}

void db_query_select_result::SetData(std::vector<model_info> *out_vec) {
  for (auto &row: values_vec) {
    model_info mi = model_info::GetDefault();
    for (auto &col: row) {
      if (isFieldName(TABLE_FIELD_NAME(MI_MODEL_ID), fields[col.first])) {
        mi.id = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_model_id;
      } else if (isFieldName(TABLE_FIELD_NAME(MI_MODEL_TYPE), fields[col.first])) {
        mi.short_info.model_type.type = (rg_model_t)std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_model_type;
      } else if (isFieldName(TABLE_FIELD_NAME(MI_MODEL_SUBTYPE), fields[col.first])) {
        mi.short_info.model_type.subtype = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_model_subtype;
      } else if (isFieldName(TABLE_FIELD_NAME(MI_VERS_MAJOR), fields[col.first])) {
        mi.short_info.vers_major = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_vers_major;
      } else if (isFieldName(TABLE_FIELD_NAME(MI_VERS_MINOR), fields[col.first])) {
        mi.short_info.vers_minor = std::atoi(col.second.c_str());
        mi.initialized |= model_info::f_vers_minor;
      } else if (isFieldName(TABLE_FIELD_NAME(MI_SHORT_INFO), fields[col.first])) {
        mi.short_info.short_info = col.second.c_str();
        mi.initialized |= model_info::f_short_info;
      }
    }
    if (mi.initialized != mi.f_empty)
      out_vec->push_back(std::move(mi));
  }
}

void db_query_select_result::SetData(
    std::vector<calculation_info> *out_vec) {
  for (auto &row: values_vec) {
    calculation_info ci;
    for (auto &col: row) {
      if (isFieldName(TABLE_FIELD_NAME(CI_CALCULATION_ID), fields[col.first])) {
        ci.id = std::atoi(col.second.c_str());
        ci.initialized |= calculation_info::f_calculation_info_id;
      } else if (isFieldName(TABLE_FIELD_NAME(CI_MODEL_INFO_ID), fields[col.first])) {
        ci.model_id = std::atoi(col.second.c_str());
        ci.initialized |= calculation_info::f_model_id;
      } else if (isFieldName(TABLE_FIELD_NAME(CI_DATE), fields[col.first])) {
        ci.SetDate(col.second);
      } else if (isFieldName(TABLE_FIELD_NAME(CI_TIME), fields[col.first])) {
        ci.SetTime(col.second);
      }
    }
    if (ci.initialized != ci.f_empty)
      out_vec->push_back(std::move(ci));
  }
}

void db_query_select_result::SetData(
    std::vector<calculation_state_log> *out_vec) {
  for (auto &row: values_vec) {
    calculation_state_log cl;
    for (auto &col: row) {
      if (isFieldName(TABLE_FIELD_NAME(CSL_LOG_ID), fields[col.first])) {
        cl.id = std::atoi(col.second.c_str());
        cl.initialized |= calculation_state_log::f_calculation_state_log_id;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_INFO_ID), fields[col.first])) {
        cl.info_id = std::atoi(col.second.c_str());
        cl.initialized |= calculation_state_log::f_calculation_info_id;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_VOLUME), fields[col.first])) {
        cl.dyn_pars.parm.volume = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_vol;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_PRESSURE), fields[col.first])) {
        cl.dyn_pars.parm.pressure = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_pres;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_TEMPERATURE), fields[col.first])) {
        cl.dyn_pars.parm.temperature = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_temp;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_HEAT_CV), fields[col.first])) {
        cl.dyn_pars.heat_cap_vol = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_dcv;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_HEAT_CP), fields[col.first])) {
        cl.dyn_pars.heat_cap_pres = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_dcp;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_INTERNAL_ENERGY), fields[col.first])) {
        cl.dyn_pars.internal_energy = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_din;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_BETA_KR), fields[col.first])) {
        cl.dyn_pars.beta_kr = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_dbk;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_ENTHALPY), fields[col.first])) {
        cl.enthalpy = std::atof(col.second.c_str());
        cl.initialized |= calculation_state_log::f_enthalpy;
      } else if (isFieldName(TABLE_FIELD_NAME(CSL_STATE_PHASE), fields[col.first])) {
        cl.state_phase = col.second;
        cl.initialized |= calculation_state_log::f_state_phase;
      }
    }
    if (cl.initialized != cl.f_empty)
      out_vec->push_back(std::move(cl));
  }
}

/* db_where_tree */
db_where_tree *db_where_tree::Init(const model_info &where) {
  return db_where_tree::init(where);
}
db_where_tree *db_where_tree::Init(const calculation_info &where) {
  return db_where_tree::init(where);
}
db_where_tree *db_where_tree::Init(const calculation_state_log &where) {
  return db_where_tree::init(where);
}

db_where_tree::~db_where_tree() {
  for (auto x: source_)
    delete x;
  source_.clear();
}

db_where_tree::db_where_tree()
  : root_(nullptr) {}

std::string db_where_tree::GetString(db_condition_node::DataToStrF dts) const {
  if (root_) {
    for_each (source_.begin(), source_.end(),
        [](db_condition_node *c) { c->visited = false; });
    return root_->GetString(dts);
  }
  return "";
}

std::vector<db_condition_node *>::iterator db_where_tree::TreeBegin() {
  return source_.begin();
}

std::vector<db_condition_node *>::iterator db_where_tree::TreeEnd() {
  return source_.end();
}

void db_where_tree::construct() {
  std::stack<db_condition_node *> st;
  for (auto nd = source_.begin(); nd != source_.end(); ++nd) {
    if ((*nd)->IsOperator()) {
      auto top1 = st.top();
      st.pop();
      auto top2 = st.top();
      st.pop();
      (*nd)->rigth = top1;
      (*nd)->left = top2;
      st.push(*nd);
    } else {
      st.push(*nd);
    }
  }
  root_ = st.top();
}
