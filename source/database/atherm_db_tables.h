/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * atherm_db_tables *
 *   Здесь прописаны данные таблиц проекта asp_therm
 * ===================================================================
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__ATHERM_DB_TABLES_H_
#define _DATABASE__ATHERM_DB_TABLES_H_

#include "db_tables.h"
#include "models_configurations.h"

#include <string>

using namespace asp_db;

/* коды столбцов в БД */
// table codes
#define MODELINFO_TABLE 0x00010000
#define CALCULATIONINFO_TABLE 0x00020000
#define CALCULATIONSTATE_TABLE 0x00030000

// column codes
#define MI_MODEL_ID (MODELINFO_TABLE | 0x0001)
#define MI_MODEL_TYPE (MODELINFO_TABLE | 0x0002)
#define MI_MODEL_SUBTYPE (MODELINFO_TABLE | 0x0003)
#define MI_VERS_MAJOR (MODELINFO_TABLE | 0x0004)
#define MI_VERS_MINOR (MODELINFO_TABLE | 0x0005)
#define MI_SHORT_INFO (MODELINFO_TABLE | 0x0006)

#define CI_CALCULATION_ID (CALCULATIONINFO_TABLE | 0x0001)
#define CI_MODEL_INFO_ID (CALCULATIONINFO_TABLE | 0x0002)
#define CI_DATE (CALCULATIONINFO_TABLE | 0x0003)
#define CI_TIME (CALCULATIONINFO_TABLE | 0x0004)
// todo: добавить другую таблицу, заменить поле на ссылку на таблицу
#define CI_GASMIX_FILE (CALCULATIONINFO_TABLE | 0x0005)

#define CSL_LOG_ID (CALCULATIONSTATE_TABLE | 0x0001)
#define CSL_INFO_ID (CALCULATIONSTATE_TABLE | 0x0002)
#define CSL_VOLUME (CALCULATIONSTATE_TABLE | 0x0003)
#define CSL_PRESSURE (CALCULATIONSTATE_TABLE | 0x0004)
#define CSL_TEMPERATURE (CALCULATIONSTATE_TABLE | 0x0005)
#define CSL_HEAT_CV (CALCULATIONSTATE_TABLE | 0x0006)
#define CSL_HEAT_CP (CALCULATIONSTATE_TABLE | 0x0007)
#define CSL_INTERNAL_ENERGY (CALCULATIONSTATE_TABLE | 0x0008)
#define CSL_ENTHALPY (CALCULATIONSTATE_TABLE | 0x0009)
#define CSL_ADIABATIC (CALCULATIONSTATE_TABLE | 0x0010)
#define CSL_BETA_KR (CALCULATIONSTATE_TABLE | 0x0011)
#define CSL_ENTROPY (CALCULATIONSTATE_TABLE | 0x0012)
#define CSL_STATE_PHASE (CALCULATIONSTATE_TABLE | 0x0013)

/* имена столбцов в БД в формате 'FIELD_CODE_DEFINE ## _NAME' */
// column names
#define MI_MODEL_ID_NAME "model_id"
#define MI_MODEL_TYPE_NAME "model_type"
#define MI_MODEL_SUBTYPE_NAME "model_subtype"
#define MI_VERS_MAJOR_NAME "vers_major"
#define MI_VERS_MINOR_NAME "vers_minor"
#define MI_SHORT_INFO_NAME "short_info"

#define CI_CALCULATION_ID_NAME "calculation_id"
#define CI_MODEL_INFO_ID_NAME "model_info_id"
#define CI_DATE_NAME "date"
#define CI_TIME_NAME "time"
#define CI_GASMIX_FILE_NAME "gasmixfile"

#define CSL_LOG_ID_NAME "calculation_log_id"
#define CSL_INFO_ID_NAME "calculation_info_id"
#define CSL_VOLUME_NAME "volume"
#define CSL_PRESSURE_NAME "pressure"
#define CSL_TEMPERATURE_NAME "temperature"
#define CSL_HEAT_CV_NAME "heat_capacity_vol"
#define CSL_HEAT_CP_NAME "heat_capacity_pres"
#define CSL_INTERNAL_ENERGY_NAME "internal_energy"
#define CSL_ENTHALPY_NAME "enthalpy"
#define CSL_ADIABATIC_NAME "adiabatic"
#define CSL_BETA_KR_NAME "beta_kr"
#define CSL_ENTROPY_NAME "entropy"
#define CSL_STATE_PHASE_NAME "state_phase"

/*
struct model_info;
struct calculation_info;
struct calculation_state_log;
*/

enum atherm_db_tables {
  /** \brief Заглушка */
  table_undefiend = UNDEFINED_TABLE,
  /** \brief Информация о ревизии уравнения состояния */
  table_model_info = MODELINFO_TABLE >> 16,
  /** \brief Информация о расчёте */
  table_calculation_info = CALCULATIONINFO_TABLE >> 16,
  /** \brief лог расчёта */
  table_calculation_state_log = CALCULATIONSTATE_TABLE >> 16,
};

namespace table_fields_setup {
extern const db_fields_collection model_info_fields;
extern const db_fields_collection calculation_info_fields;
extern const db_fields_collection calculation_state_log_fields;
}  // namespace table_fields_setup

/** \brief */
class AthermDBTables final : public IDBTables {
 public:
  std::string GetTableName(db_table t) const override;
  const db_fields_collection* GetFieldsCollection(db_table t) const override;
  db_table StrToTableCode(const std::string& tname) const override;
  std::string GetIdColumnName(db_table dt) const override;
  // db_ref_collection RefCollectionByCode(db_table table) const override;
  const db_table_create_setup& CreateSetupByCode(db_table dt) const override;
};

/* Специализация шаблонов базового класса таблиц */
/* GetTableName */
template <>
std::string IDBTables::GetTableName<model_info>() const;
template <>
std::string IDBTables::GetTableName<calculation_info>() const;
template <>
std::string IDBTables::GetTableName<calculation_state_log>() const;
/* GetTableCode */
template <>
db_table IDBTables::GetTableCode<model_info>() const;
template <>
db_table IDBTables::GetTableCode<calculation_info>() const;
template <>
db_table IDBTables::GetTableCode<calculation_state_log>() const;

/* setInsertValues */
/** \brief Собрать вектор 'values' значений столбцов БД,
 *   по переданным строкам model_info */
template <>
void IDBTables::setInsertValues<>(db_query_insert_setup* src,
                                  const model_info& select_data) const;
/** \brief Собрать вектор 'values' значений столбцов БД,
 *   по переданным строкам calculation_info */
template <>
void IDBTables::setInsertValues<calculation_info>(
    db_query_insert_setup* src,
    const calculation_info& select_data) const;
/** \brief Собрать вектор 'values' значений столбцов БД,
 *   по переданным строкам calculation_state_log */
template <>
void IDBTables::setInsertValues<calculation_state_log>(
    db_query_insert_setup* src,
    const calculation_state_log& select_data) const;

/* SetSelectData */
/** \brief Записать в out_vec строки model_info из данных values_vec,
 *   полученных из БД
 * \note Обратная операция для db_query_insert_setup::setValues */
template <>
void IDBTables::SetSelectData<model_info>(
    db_query_select_result* src,
    std::vector<model_info>* out_vec) const;
/** \brief Записать в out_vec строки calculation_info из данных values_vec,
 *   полученных из БД */
template <>
void IDBTables::SetSelectData<calculation_info>(
    db_query_select_result* src,
    std::vector<calculation_info>* out_vec) const;
/** \brief Записать в out_vec строки calculation_state_log из данных values_vec,
 *   полученных из БД */
template <>
void IDBTables::SetSelectData<calculation_state_log>(
    db_query_select_result* src,
    std::vector<calculation_state_log>* out_vec) const;

#endif  // !_DATABASE__ATHERM_DB_TABLES_H_
