/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 *   Здесь прописана логика создания абстрактных запросов к БД,
 * которые создаются классом управляющим подключением к БД
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_QUERIES_SETUP_H_
#define _DATABASE__DB_QUERIES_SETUP_H_

#include "common.h"
#include "db_defines.h"
#include "ErrorWrap.h"

#include <map>
#include <string>


struct model_info;
struct calculation_info;
struct calculation_state_info;

/* Data structs */
/* todo: можно наверное общую имплементацию
 *   db_table_create_setup и db_table_select_setup
 *   вынести в структуру db_table_setup
 * UPD: сейчас идея не кажется удачной */
struct db_table_create_setup {
public:
  /** \brief получить сетап на создание таблицы */
  static const db_table_create_setup &get_table_create_setup(db_table dt);

  db_table_create_setup(db_table table,
      const db_fields_collection &fields);

public:
  ErrorWrap init_error;
  db_table table;
  const db_fields_collection &fields;
  /** \brief вектор имен полей таблицы которые составляют
   *    сложный(неодинарный) первичный ключ */
  db_complex_pk pk_string;
  const db_ref_collection *ref_strings;

private:
  void setupPrimaryKeyString();
  void checkReferences();
};

// #define field_index_end -1
/** \brief структура для сборки SELECT/UPDATE/INSERT запросов */
struct db_table_update_setup {
  typedef int32_t field_index;
  static constexpr int field_index_end = -1;

  enum class db_update_t {
    SELECT,
    UPDATE,
    INSERT,
    DELETE
  };

public:
  static db_table_update_setup *Init(db_table table);

  void SetUpdateSetup(db_update_t up, model_info &select_data);
  void SetUpdateSetup(db_update_t up, calculation_info &select_data);
  void SetUpdateSetup(db_update_t up, calculation_state_info &select_data);

private:
  db_table_update_setup(db_table table,
      const db_fields_collection &fields);

  field_index indexByFieldName(const std::string &fname);

public:
  ErrorWrap error;
  db_table table;
  /** \brief Тип обновления */
  db_update_t update_t;
  /** \brief Ссылка на коллекцию полей(столбцов)
    *   таблицы в БД для таблицы 'table' */
  const db_fields_collection &fields;
  /** \brief Набор значений для операции(транзакции) */
  std::map<field_index, std::string> values;
};

#endif  // !_DATABASE__DB_QUERIES_SETUP_H_
