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

#include <stack>

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
static const db_fields_collection model_info_fields = {
  db_variable(MI_MODEL_ID, db_type::type_autoinc, {.is_primary_key = true,
      .is_unique = true, .can_be_null = false}),
  db_variable(MI_MODEL_TYPE, db_type::type_int, {.can_be_null = false,
      .in_unique_constrain = true}),
  db_variable(MI_MODEL_SUBTYPE, db_type::type_int, {.in_unique_constrain = true}),
  db_variable(MI_VERS_MAJOR, db_type::type_int, {.can_be_null = false,
      .in_unique_constrain = true}),
  db_variable(MI_VERS_MINOR, db_type::type_int, {.in_unique_constrain = true}),
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
  UNIQUE(model_info_d, date, time),
  PRIMARY KEY (calculation_id),
  FOREIGN KEY (model_info_id) REFERENCES model_info(model_id)
); */
static const db_fields_collection calculation_info_fields = {
  db_variable(CI_CALCULATION_ID, db_type::type_autoinc, {.is_primary_key = true,
      .is_unique = true, .can_be_null = false}),
  // reference to model_info(fk)
  db_variable(CI_MODEL_INFO_ID, db_type::type_int, {.is_reference = true,
      .can_be_null = false, .in_unique_constrain = true}),
  db_variable(CI_DATE, db_type::type_date, {.can_be_null = false,
      .in_unique_constrain = true}),
  db_variable(CI_TIME, db_type::type_time, {.can_be_null = false,
      .in_unique_constrain = true})
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

/* db_condition_tree */
// db_condition_tree::db_condition_tree()
//   : left(nullptr), rigth(nullptr), data(nullptr), db_operator(db_operator_t::op_empty) {}

db_condition_tree::db_condition_tree(db_operator_t db_operator)
  : data(""), db_operator(db_operator), is_leafnode(false) {}

db_condition_tree::db_condition_tree(const std::string &data)
  : data(data), db_operator(db_operator_t::op_empty), is_leafnode(true) {}

db_condition_tree::~db_condition_tree() {}

std::string db_condition_tree::GetString() const {
  std::string l, r;
  std::string result;
  visited = true;
  // если поддеревьев нет, вернуть строку
  if (db_operator == db_operator_t::op_empty)
    return data;
  if (left)
    if (!left->visited)
      l = left->GetString();
  if (rigth)
    if (!rigth->visited)
      r = rigth->GetString();
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

/* db_table_query_basesetup */
db_query_basesetup::db_query_basesetup(
    db_table table, const db_fields_collection &fields)
  : table(table), fields(fields) {}

db_query_basesetup::field_index db_query_basesetup::
    IndexByFieldName(const std::string &fname) {
  field_index i = 0;
  for (auto const &x : fields) {
    if (x.fname == fname)
      break;
    ++i;
  }
  return (i == fields.size()) ? db_query_basesetup::field_index_end : i;
}

/* db_table_insert_setup */
db_query_insert_setup::db_query_insert_setup(db_table _table,
    const db_fields_collection &_fields)
  : db_query_basesetup(_table, _fields) {
  update_t = INSERT;
}

/* todo: сделать шаблон, или вынести в шабло */
db_query_insert_setup *db_query_insert_setup::Init(
    const std::vector<model_info> &select_data) {
  if (haveConflict(select_data))
    return nullptr;
  db_query_insert_setup *ins_setup = new db_query_insert_setup(
      db_table::table_model_info, *ns_tfs::get_fields_collection(
      db_table::table_model_info));
  if (ins_setup)
    for (const auto &x : select_data)
      ins_setup->setValues(x);
  return ins_setup;
}

db_query_insert_setup *db_query_insert_setup::Init(
    const std::vector<calculation_info> &select_data) {
  if (haveConflict(select_data))
    return nullptr;
  db_query_insert_setup *ins_setup = new db_query_insert_setup(
      db_table::table_calculation_info, *ns_tfs::get_fields_collection(
      db_table::table_calculation_info));
  if (ins_setup)
    for (const auto &x : select_data)
      ins_setup->setValues(x);
  return ins_setup;
}

db_query_insert_setup *db_query_insert_setup::Init(
    const std::vector<calculation_state_log> &select_data) {
  if (haveConflict(select_data))
    return nullptr;
  db_query_insert_setup *ins_setup = new db_query_insert_setup(
      db_table::table_calculation_state_log, *ns_tfs::get_fields_collection(
      db_table::table_calculation_state_log));
  if (ins_setup)
    for (const auto &x : select_data)
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
  if (select_data.initialized & model_info::f_model_type) {
    if ((i = IndexByFieldName(MI_MODEL_TYPE)) != field_index_end)
      values.emplace(i, std::to_string(
          (int)select_data.short_info.model_type.type));
    if ((i = IndexByFieldName(MI_MODEL_SUBTYPE)) != field_index_end)
      values.emplace(i, std::to_string(
          select_data.short_info.model_type.subtype));
  }
  if (select_data.initialized & model_info::f_vers_major)
    if ((i = IndexByFieldName(MI_VERS_MAJOR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.short_info.vers_major));
  if (select_data.initialized & model_info::f_vers_minor)
    if ((i = IndexByFieldName(MI_VERS_MINOR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.short_info.vers_minor));
  if (select_data.initialized & model_info::f_short_info)
    if ((i = IndexByFieldName(MI_SHORT_INFO)) != field_index_end)
      values.emplace(i, select_data.short_info.short_info);
  values_vec.emplace_back(values);
}
void db_query_insert_setup::setValues(const calculation_info &select_data) {
  if (select_data.initialized == calculation_info::f_empty)
    return;
  row_values values;
  field_index i;
  if (select_data.initialized & calculation_info::f_model_id)
    if (select_data.model != nullptr)
      if ((i = IndexByFieldName(CI_MODEL_INFO_ID)) != field_index_end)
        values.emplace(i, std::to_string(select_data.model->id));
  if (select_data.initialized & calculation_info::f_date)
    if ((i = IndexByFieldName(CI_DATE)) != field_index_end)
      values.emplace(i, select_data.GetDate());
  if (select_data.initialized & calculation_info::f_time)
    if ((i = IndexByFieldName(CI_TIME)) != field_index_end)
      values.emplace(i, select_data.GetTime());
  values_vec.emplace_back(values);
}
void db_query_insert_setup::setValues(const calculation_state_log &select_data) {
  if (select_data.initialized == calculation_state_log::f_empty)
    return;
  row_values values;
  field_index i;
  if (select_data.initialized & calculation_state_log::f_info)
    if (select_data.calculation != nullptr)
      if ((i = IndexByFieldName(CSL_INFO_ID)) != field_index_end)
        values.emplace(i, std::to_string(select_data.calculation->id));
  if (select_data.initialized & calculation_state_log::f_vol)
    if ((i = IndexByFieldName(CSL_VOLUME)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.volume));
  if (select_data.initialized & calculation_state_log::f_pres)
    if ((i = IndexByFieldName(CSL_PRESSURE)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.pressure));
  if (select_data.initialized & calculation_state_log::f_temp)
    if ((i = IndexByFieldName(CSL_TEMPERATURE)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.parm.temperature));
  if (select_data.initialized & calculation_state_log::f_dcv)
    if ((i = IndexByFieldName(CSL_HEAT_CV)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.heat_cap_vol));
  if (select_data.initialized & calculation_state_log::f_dcp)
    if ((i = IndexByFieldName(CSL_HEAT_CP)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.heat_cap_pres));
  if (select_data.initialized & calculation_state_log::f_din)
    if ((i = IndexByFieldName(CSL_INTERNAL_ENERGY)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.internal_energy));
  if (select_data.initialized & calculation_state_log::f_dbk)
    if ((i = IndexByFieldName(CSL_BETA_KR)) != field_index_end)
      values.emplace(i, std::to_string(select_data.dyn_pars.beta_kr));
  if (select_data.initialized & calculation_state_log::f_enthalpy)
    if ((i = IndexByFieldName(CSL_ENTHALPY)) != field_index_end)
      values.emplace(i, std::to_string(select_data.enthalpy));
  if (select_data.initialized & calculation_state_log::f_state_phase)
    if ((i = IndexByFieldName(CSL_STATE_PHASE)) != field_index_end)
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

void db_where_tree::construct() {
  std::stack<db_condition_tree *> st;
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

/*
int db_condition_tree::subnodesCount() {
  int s = 0;
  if (left)
    s += 1 + left->subnodesCount();
  if (rigth)
    s += 1 + rigth->subnodesCount();
  return s;
}
*/
