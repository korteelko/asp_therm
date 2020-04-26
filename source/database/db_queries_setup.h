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

/** \brief Структура описывающая дерево логических отношений
  * \note В общем, во внутренних узлах хранится операция, в конечных
  *   операнды, соответственно строка выражения собирается обходом
  *   в глубь
  * Прописывал ориантируюсь на СУБД Postgre потому что
  *   более/менее похожа стандарт */
struct db_condition_tree {
  /** \brief Операторы отношений условий
    * \note чё там унарые то операторы то подвезли? */
  enum class db_operator_t {
    /** \brief пустой оператор */
    op_empty = 0,
    /** \brief IS */
    op_is,
    /** \brief IS NOT */
    op_not,
    /** \brief оператор поиска в листе */
    op_in,
    /** \brief оператор поиска в коллекции */
    op_like,
    /** \brief выборка по границам */
    op_between,
    /** \brief логическое 'И' */
    op_and,
    /** \brief логическое 'ИЛИ' */
    op_or,
    /** \brief == равно */
    op_eq,
    /** \brief != не равно */
    op_ne,
    /** \brief >= больше или равно */
    op_ge,
    /** \brief > больше */
    op_gt,
    /** \brief <= меньше или равно*/
    op_le,
    /** \brief < меньше */
    op_lt,
  };

public:
  db_condition_tree();
  db_condition_tree(db_operator_t db_operator);
  db_condition_tree(const std::string &data);
  ~db_condition_tree();
  /** \brief Получить строковое представление дерева */
  std::string GetString() const;

  /** \brief Создание поддерева для примитивной операции */
  merror_t SetSubTree(const std::string &l, const std::string &r);
  /** \brief Создание поддерева для примитивной операции */
  merror_t SetSubTree(db_operator_t operator_t, const std::string &l,
      const std::string &r);


public:
  db_condition_tree *left;
  db_condition_tree *rigth;
  /* todo: optimize here:
   *   cause we can replace 'data' and 'db_operator'
   *   as union
   */
  std::string data;
  db_operator_t db_operator;
};

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


namespace table_fields_setup {
const db_fields_collection *get_fields_collection(db_table dt);
}

/* queries setup */
/** \brief базовая структура сборки запроса */
struct db_query_basesetup {
  typedef int32_t field_index;
  /** \brief Набор данных */
  typedef std::map<field_index, std::string> row_values;

  enum db_query_t {
    SELECT,
    UPDATE,
    INSERT,
    DELETE
  };

  static constexpr int field_index_end = -1;

public:
  virtual ~db_query_basesetup() = default;

  field_index IndexByFieldName(const std::string &fname);

protected:
  db_query_basesetup(db_table table,
      const db_fields_collection &fields);

public:
  ErrorWrap error;
  db_table table;
  /** \brief Тип обновления */
  db_query_t update_t;

public:
  /** \brief Ссылка на коллекцию полей(столбцов)
    *   таблицы в БД для таблицы 'table' */
  const db_fields_collection &fields;
};


/** \brief структура для сборки INSERT запросов */
struct db_query_insert_setup: public db_query_basesetup {
public:
  static db_query_insert_setup *Init(
      const std::vector<model_info> &select_data);
  static db_query_insert_setup *Init(
      const std::vector<calculation_info> &select_data);
  static db_query_insert_setup *Init(
      const std::vector<calculation_state_info> &select_data);

  size_t RowsSize() const;

  virtual ~db_query_insert_setup() = default;

protected:
  db_query_insert_setup(db_table _table, const db_fields_collection &_fields);

  template <class DataInfo, class Table>
  static db_query_insert_setup *init(Table t,
      const std::vector<DataInfo> &select_data) {
    if (haveConflict(select_data))
      return nullptr;
    db_query_insert_setup *ins_setup = new db_query_insert_setup(t,
        *table_fields_setup::get_fields_collection(t));
    if (ins_setup)
      for (const auto &x : select_data)
        ins_setup->setValues(x);
    return ins_setup;
  }

  template <class DataInfo>
  static bool haveConflict(const std::vector<DataInfo> &select_data) {
    if (!select_data.empty()) {
      auto initialized = (*select_data.begin()).initialized;
      for (auto it = select_data.begin() + 1; it != select_data.end(); ++it )
        if (initialized != (*it).initialized)
          return true;
    }
    return false;
  }
  /** \brief Собрать вектор 'values' значеений столбцов БД,
    *   по переданным структурам */
  void setValues(const model_info &select_data);
  void setValues(const calculation_info &select_data);
  void setValues(const calculation_state_info &select_data);

public:
  /** \brief Набор значений для операций INSERT */
  // std::vector<row_values> values_vec;
  std::vector<row_values> values_vec;
};


/** \brief структура для сборки SELECT(и DELETE) запросов */
struct db_query_select_setup: public db_query_basesetup {
public:
  static db_query_select_setup *Init(db_table _table);

  virtual ~db_query_select_setup() = default;

public:
  /** \brief Сетап выражения where для SELECT/UPDATE/DELETE запросов */
  std::unique_ptr<db_condition_tree> where_condition;

protected:
  db_query_select_setup(db_table _table,
      const db_fields_collection &_fields);
};
/** \brief псевдоним DELETE запросов */
typedef db_query_select_setup db_query_delete_setup;


/** \brief структура для сборки UPDATE запросов */
struct db_query_update_setup: public db_query_select_setup {
public:
  static db_query_update_setup *Init(db_table _table);

public:
  row_values values;

protected:
  db_query_update_setup(db_table _table,
      const db_fields_collection &_fields);
};


/* select result */
/** \brief структура для сборки INSERT запросов */
struct db_query_select_result: public db_query_basesetup {
public:
  db_query_select_result() = delete;
  db_query_select_result(const db_query_select_setup &setup);

  virtual ~db_query_select_result() = default;

public:
  /** \brief Набор значений для операций INSERT/UPDATE */
  std::vector<row_values> values_vec;
};

#endif  // !_DATABASE__DB_QUERIES_SETUP_H_
