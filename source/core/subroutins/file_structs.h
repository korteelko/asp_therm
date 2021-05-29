/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__FILE_STRUCTS_H_
#define _CORE__SUBROUTINS__FILE_STRUCTS_H_

#include "asp_db/db_connection.h"
#include "asp_db/db_defines.h"
#include "asp_utils/ErrorWrap.h"
#include "atherm_common.h"

#include <array>
#include <map>
#include <string>

#include <stdint.h>

typedef uint32_t node_type;
#define CONFIG_NODE_COUNT 3
#define GAS_NODE_COUNT 5
#define GASMIX_NODE_COUNT 3
#define CALCUL_NODE_COUNT 4
#define NODE_T_ROOT 0
#define NODE_T_UNDEFINED 0xff

/// class for initializing program configuration
class config_node {
  static std::array<std::string, CONFIG_NODE_COUNT> node_t_list;

 public:
  node_type config_node_type;
  std::string name, value;

  config_node(node_type itype, std::string name);
  config_node(node_type itype, std::string name, std::string value);

  static std::string get_root_name();
  static node_type get_node_type(std::string type);
};

/** \brief По паре строковых аргументов записать в структуру dst
 *   новое значение параметра, представленого аргументами
 * \param param_strtpl Строковое представление параметра(ключ)
 * \param param_value Строковое значение параметра */
merror_t set_db_parameter(asp_db::db_parameters* dst,
                          const std::string& param_strtpl,
                          const std::string& param_value);

/// class for initializing gas component by file
class gas_node {
  static std::array<std::string, GAS_NODE_COUNT> node_t_list;

 public:
  node_type gas_node_type;
  std::string name, value;

  gas_node(node_type itype, std::string name);
  gas_node(node_type itype, std::string name, std::string value);

  static std::string get_root_name();
  static node_type get_node_type(std::string type);
};

/// class for initializing gasmix by file
class gasmix_node {
  static std::array<std::string, GASMIX_NODE_COUNT> node_t_list;

 public:
  node_type mix_node_type;
  std::string name, value;

  gasmix_node(node_type itype, std::string name);
  gasmix_node(node_type itype, std::string name, std::string value);

  static std::string get_root_name();
  static node_type get_node_type(std::string type);
};

/** \brief искать в переданной в аргументах мапе
 *   ключ 'v', если он там записать его по адресу
 *   переданного параметра 'a'
 * \param v ключ из xml(или другого) файла, может
 *   содержать пробелы
 * \param a out переменная по адресу расположения
 *   которой запишется результат поиска
 * \return ERROR_SUCCESS_T если ключ найден
 *         ERROR_STRTPL_VALWRONG иначе */
template <class T>
merror_t set_by_map(const std::map<const std::string, T>& m,
                    const std::string& v,
                    T& a) {
  merror_t error = ERROR_STRTPL_VALWRONG;
  auto map_it = m.find(trim_str(v));
  if (map_it != m.end()) {
    error = ERROR_SUCCESS_T;
    a = map_it->second;
  }
  return error;
}

/* PARSE TEMPLATE VALUES */
/** \brief проверить соответствие 'val' допустимым значениям
 *   шаблона для bool : "true" или "false"
 * \param val текстовый шаблон значения
 * \param ans out bool параметр значения */
[[nodiscard]] merror_t set_bool(const std::string& val, bool* ans);
/** \brief проверить соответствие 'val' допустимым значениям
 *   шаблона для db_client: "none", "postgresql"
 * \param val текстовый шаблон значения
 * \param ans out db_client параметр значения */
[[nodiscard]] merror_t set_db_client(const std::string& val,
                                     asp_db::db_client* ans);
/** \brief проверить соответствие 'val' допустимым значениям double
 * \param val текстовый шаблон значения
 * \param ans out double параметр значения */
[[nodiscard]] merror_t set_double(const std::string& val, double* ans);
/** \brief проверить соответствие 'val' допустимым значениям int
 * \param val текстовый шаблон значения
 * \param ans out int параметр значения */
[[nodiscard]] merror_t set_int(const std::string& val, int* ans);
/** \brief проверить соответствие 'val' допустимым значениям
 *   шаблона для io_loglvl: "debug", ...
 * \param val текстовый шаблон значения
 * \param ans out bool параметр значения */
[[nodiscard]] merror_t set_loglvl(const std::string& val, io_loglvl* ans);
/** \brief проверить соответствие 'val' допустимым значениям
 *   шаблона для модели расчёта
 * \param val текстовый шаблон значения
 * \param ans out bool параметр значения */
[[nodiscard]] merror_t set_model(const std::string& val, rg_model_id* ans);

#endif  // !_CORE__SUBROUTINS__FILE_STRUCTS_H_
