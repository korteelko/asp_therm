/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * db_tables *
 *   Здесь прописаны данные таблиц проекта asp_therm
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_TABLES_H_
#define _DATABASE__DB_TABLES_H_

#include <string>


/** \brief Макро на получение имени поля по его id */
#define TABLE_FIELD_NAME(x) x ## _NAME
/** \brief Макро на составление пар полей БД формата
  *   '(field_id, field_name)' */
#define TABLE_FIELD_PAIR(x) x, x ## _NAME

/* коды столбцов в БД */
// table codes
#define UNDEFINED_TABLE         0x00000000
#define MODELINFO_TABLE         0x00010000
#define CALCULATIONINFO_TABLE   0x00020000
#define CALCULATIONSTATE_TABLE  0x00030000

// column codes
#define UNDEFINED_COLUMN        0x00000000
#define MI_MODEL_ID             (MODELINFO_TABLE | 0x0001)
#define MI_MODEL_TYPE           (MODELINFO_TABLE | 0x0002)
#define MI_MODEL_SUBTYPE        (MODELINFO_TABLE | 0x0003)
#define MI_VERS_MAJOR           (MODELINFO_TABLE | 0x0004)
#define MI_VERS_MINOR           (MODELINFO_TABLE | 0x0005)
#define MI_SHORT_INFO           (MODELINFO_TABLE | 0x0006)

#define CI_CALCULATION_ID       (CALCULATIONINFO_TABLE | 0x0001)
#define CI_MODEL_INFO_ID        (CALCULATIONINFO_TABLE | 0x0002)
#define CI_DATE                 (CALCULATIONINFO_TABLE | 0x0003)
#define CI_TIME                 (CALCULATIONINFO_TABLE | 0x0004)

#define CSL_LOG_ID              (CALCULATIONSTATE_TABLE | 0x0001)
#define CSL_INFO_ID             (CALCULATIONSTATE_TABLE | 0x0002)
#define CSL_VOLUME              (CALCULATIONSTATE_TABLE | 0x0003)
#define CSL_PRESSURE            (CALCULATIONSTATE_TABLE | 0x0004)
#define CSL_TEMPERATURE         (CALCULATIONSTATE_TABLE | 0x0005)
#define CSL_HEAT_CV             (CALCULATIONSTATE_TABLE | 0x0006)
#define CSL_HEAT_CP             (CALCULATIONSTATE_TABLE | 0x0007)
#define CSL_INTERNAL_ENERGY     (CALCULATIONSTATE_TABLE | 0x0008)
#define CSL_BETA_KR             (CALCULATIONSTATE_TABLE | 0x0009)
#define CSL_ENTHALPY            (CALCULATIONSTATE_TABLE | 0x0010)
#define CSL_STATE_PHASE         (CALCULATIONSTATE_TABLE | 0x0011)

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

#define CSL_LOG_ID_NAME "calculation_log_id"
#define CSL_INFO_ID_NAME "calculation_info_id"
#define CSL_VOLUME_NAME "volume"
#define CSL_PRESSURE_NAME "pressure"
#define CSL_TEMPERATURE_NAME "temperature"
#define CSL_HEAT_CV_NAME "heat_capacity_vol"
#define CSL_HEAT_CP_NAME "heat_capacity_pres"
#define CSL_INTERNAL_ENERGY_NAME "internal_energy"
#define CSL_BETA_KR_NAME "beta_kr"
#define CSL_ENTHALPY_NAME "enthalpy"
#define CSL_STATE_PHASE_NAME "state_phase"

/** \brief перечисление типов используемых таблиц */
enum class db_table {
  /** \brief Заглушка */
  table_undefiend = UNDEFINED_TABLE,
  /** \brief Информация о ревизии уравнения состояния */
  table_model_info = MODELINFO_TABLE >> 16,
  /** \brief Информация о расчёте */
  table_calculation_info = CALCULATIONINFO_TABLE >> 16,
  /** \brief лог расчёта */
  table_calculation_state_log = CALCULATIONSTATE_TABLE >> 16
};
/** \brief Получить имя таблицы в БД по идентификатору */
std::string get_table_name(db_table dt);
/** \brief Получить код таблицы по имени */
db_table get_table_code(const std::string &str);
/** \brief Получить имя id колонки в таблице */
std::string get_id_column_name(db_table dt);

#endif  // !_DATABASE__DB_TABLES_H_
