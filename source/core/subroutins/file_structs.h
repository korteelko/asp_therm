#ifndef _CORE__SUBROUTINS__FILE_STRUCTS_H_
#define _CORE__SUBROUTINS__FILE_STRUCTS_H_

#include " common.h"
#include "db_connection.h"
#include "models_errors.h"

#include <array>
#include <string>

#include <stdint.h>

typedef uint32_t node_type;
#define CONFIG_NODE_COUNT   3
#define GAS_NODE_COUNT      5
#define GASMIX_NODE_COUNT   3
#define NODE_T_ROOT         0
#define NODE_T_UNDEFINED    0xff


/// class for initializing program configuration
class config_node {
  static std::array<std::string, CONFIG_NODE_COUNT> node_t_list;

public:
  node_type config_node_type;
  std::string name,
              value;

  config_node(node_type itype, std::string name);
  config_node(node_type itype, std::string name, std::string value);

  static std::string get_root_name();
  static node_type get_node_type(std::string type);
};

/// class for initializing gas component by file
class gas_node {
  static std::array<std::string, GAS_NODE_COUNT> node_t_list;

public:
  node_type gas_node_type;
  std::string name,
              value;

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
  std::string name,
              value;

  gasmix_node(node_type itype, std::string name);
  gasmix_node(node_type itype, std::string name, std::string value);

  static std::string get_root_name();
  static node_type get_node_type(std::string type);
};

/* PARSE TEMPLATE VALUES
 *   TODO: дописать на основе документации */
/** \brief проверить соответствие 'val' допустимым значениям
  *   шаблона для bool : "true" или "false"
  * \param val текстовый шаблон значения
  * \param ans out bool параметр значения */
merror_t set_bool(const std::string &val, bool &ans);
/** \brief проверить соответствие 'val' допустимым значениям
  *   шаблона для db_client: "none", "postgresql"
  * \param val текстовый шаблон значения
  * \param ans out db_client параметр значения */
//merror_t set_db_client(const std::string &val, db_client &ans);
/** \brief проверить соответствие 'val' допустимым значениям double
  * \param val текстовый шаблон значения
  * \param ans out double параметр значения */
//merror_t set_double(const std::string &val, double &ans);
/** \brief проверить соответствие 'val' допустимым значениям int
  * \param val текстовый шаблон значения
  * \param ans out int параметр значения */
//merror_t set_int(const std::string &val, int &ans);
/** \brief проверить соответствие 'val' допустимым значениям
  *   шаблона для io_loglvl: "debug",
  * \param val текстовый шаблон значения
  * \param ans out bool параметр значения */
//merror_t set_loglvl(const std::string &val, io_loglvl &ans);

#endif  // !_CORE__SUBROUTINS__FILE_STRUCTS_H_
