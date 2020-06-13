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

#include <algorithm>
#include <map>
#include <memory>
#include <string>


#define OWNER(x) friend class x

struct model_info;
struct calculation_info;
struct calculation_state_log;

namespace table_fields_setup {
extern const db_fields_collection model_info_fields;
extern const db_fields_collection calculation_info_fields;
extern const db_fields_collection calculation_state_log_fields;

const db_fields_collection *get_fields_collection(db_table dt);
}  // namespace table_fields_setup

/** \brief Сетап для добавления точки сохранения */
struct db_save_point {
public:
  /** \brief Формат имени точки сохранения:
    *  [a-z,A-Z,_]{1}[a-z,A-Z,1-9,_]{*} */
  db_save_point(const std::string &_name);

  /** \brief Получить собранную строку для добавления точки сохранения */
  std::string GetString() const;

public:
  /** \brief Собственное уникальное имя ноды */
  std::string name;
};

/** \brief Сетап удаления таблицы */
struct db_table_drop_setup {
  db_table table;
  db_reference_act act;
};

/** \brief Структура описывающая дерево логических отношений
  * \note В общем, во внутренних узлах хранится операция, в конечных
  *   операнды, соответственно строка выражения собирается обходом
  *   в глубь
  * Прописывал ориантируюсь на СУБД Postgre потому что
  *   более/менее похожа стандарт */
struct db_condition_node {
  /** \brief Дерево условия для where_tree */
  OWNER(db_where_tree);
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

  typedef std::function<std::string(db_type, const std::string &,
      const std::string &)> DataToStrF;

  /** \brief Конвертировать данные узла в строку */
  static std::string DataToStr(db_type, const std::string &f,
      const std::string &v);

public:
  ~db_condition_node();
  /** \brief Получить строковое представление дерева
    * \note Предварительный сетап данных для операций с СУБД */
  std::string GetString(DataToStrF dts = DataToStr) const;
  /** \brief Есть ли подузлы */
  bool IsOperator() const { return !is_leafnode; }

protected:
  // db_condition_tree();
  db_condition_node(db_operator_t db_operator);
  db_condition_node(db_type t, const std::string &fname,
      const std::string &data);

protected:
  db_condition_node *left = nullptr;
  db_condition_node *rigth = nullptr;
  /** \brief Структура данных узла */
  struct {
    /** \brief Тип данных в БД */
    db_type type;
    /** \brief Имя столбца */
    std::string field_name;
    /** \brief Строковое представление данных */
    std::string field_value;
  } data;
  db_operator_t db_operator;
  /** \brief Количество подузлов
    * \note Для insert операций */
  // size_t subnodes_count = 0;
  /** \brief КОНЕЧНАЯ */
  bool is_leafnode = false;
  /** \brief избегаем циклических ссылок для сборок строк
    * \note небольшой оверкилл наверное
    * \todo чекнуть можно ли обойтись без неё */
  mutable bool visited = false;
};

/* Data structs */
/** \brief Структура создания таблицы БД и, в перспективе,
  *   формата существующей таблицы
  * \note Эту же структуру можно заполнять по данным полученным от СУБД,
  *   результаты же можно сравнить форматы и обновить существующую таблицу
  *   не удаляя её */
struct db_table_create_setup {
  /** \brief Набор имён полей таблицы, составляющих уникальный комплекс */
  typedef std::vector<std::string> unique_constrain;
  /** \brief Контейнер уникальных комплексов */
  typedef std::vector<unique_constrain> uniques_container;
  /** \brief enum для сравнений сетапов */
  enum compare_field {
    /** \brief Код таблицы */
    cf_table = 0,
    /** \brief Поля таблицы */
    cf_fields,
    /** \brief Сложный первичный ключ */
    cf_pk_string,
    /** \brief Уникальные комплексы */
    cf_unique_constrains,
    /** \brief Внешние ссылки */
    cf_ref_strings
  };

public:
  /** \brief Получить сетап на создание таблицы */
  static const db_table_create_setup &get_table_create_setup(db_table dt);

  /** \brief Конструктор для записи данных из БД */
  db_table_create_setup(db_table table);
  /** \brief Конструктор для добавления таблицы в БД */
  db_table_create_setup(db_table table, const db_fields_collection &fields,
      const uniques_container &unique_constrains);

  /** \brief Сравнить сетапы таблиц */
  std::map<compare_field, bool> Compare(const db_table_create_setup &r);

public:
  ErrorWrap error;
  /** \brief Код таблицы */
  db_table table;
  /** \brief Наборы полей таблицы */
  db_fields_collection fields;
  /** \brief Вектор имен полей таблицы которые составляют
    *   сложный(неодинарный) первичный ключ */
  db_complex_pk pk_string;
  /** \brief Наборы уникальных комплексов */
  uniques_container unique_constrains;
  /** \brief Набор внешних ссылок */
  db_ref_collection ref_strings;

private:
  /** \brief Собрать поле 'pk_string' */
  void setupPrimaryKeyString();
  /** \brief Проверить ссылки */
  void checkReferences();
  /** \brief Шаблон сравения массивов */
  template<class ArrayT>
  static bool IsSame(const ArrayT &l, const ArrayT &r) {
    bool same = l.size() == r.size();
    if (same) {
      size_t i = 0;
      for (; i < l.size(); ++i)
        if (l[i] != r[i])
          break;
      if (i == l.size())
        same = true;
    }
    return same;
  }
};


/* queries setup */
/** \brief Базовая структура сборки запроса */
struct db_query_basesetup {
  typedef size_t field_index;
  /** \brief Набор данных */
  typedef std::map<field_index, std::string> row_values;

  static constexpr size_t field_index_end = std::string::npos;

public:
  virtual ~db_query_basesetup() = default;

  field_index IndexByFieldId(db_variable_id fid);

protected:
  db_query_basesetup(db_table table,
      const db_fields_collection &fields);

public:
  ErrorWrap error;
  db_table table;

public:
  /** \brief Ссылка на коллекцию полей(столбцов)
    *   таблицы в БД для таблицы 'table' */
  const db_fields_collection &fields;
};


/** \brief Структура для сборки INSERT запросов */
struct db_query_insert_setup: public db_query_basesetup {
public:
  static db_query_insert_setup *Init(
      const std::vector<model_info> &insert_data);
  static db_query_insert_setup *Init(
      const std::vector<calculation_info> &insert_data);
  static db_query_insert_setup *Init(
      const std::vector<calculation_state_log> &insert_data);

  size_t RowsSize() const;

  virtual ~db_query_insert_setup() = default;

protected:
  db_query_insert_setup(db_table _table, const db_fields_collection &_fields);

  template <class DataInfo, class Table>
  static db_query_insert_setup *init(Table t,
      const std::vector<DataInfo> &insert_data) {
    if (haveConflict(insert_data))
      return nullptr;
    db_query_insert_setup *ins_setup = new db_query_insert_setup(
        t, *table_fields_setup::get_fields_collection(t));
    if (ins_setup)
      for (const auto &x: insert_data)
        ins_setup->setValues(x);
    return ins_setup;
  }

  /** \brief Проверить соответствие значений полей initialized в векторе
    *   элементов данных выборки. Для всех должны быть одинаковы */
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
  /** \brief Собрать вектор 'values' значений столбцов БД,
    *   по переданным строкам model_info */
  void setValues(const model_info &select_data);
  /** \brief Собрать вектор 'values' значений столбцов БД,
    *   по переданным строкам calculation_info */
  void setValues(const calculation_info &select_data);
  /** \brief Собрать вектор 'values' значений столбцов БД,
    *   по переданным строкам calculation_state_log */
  void setValues(const calculation_state_log &select_data);

public:
  /** \brief Набор значений для операций INSERT|SELECT|DELETE */
  std::vector<row_values> values_vec;
};
/** \brief Контейнер для результатов операции INSERT, иначе говоря,
  *   для полученных от СУБД идентификаторов рядов/строк */
struct id_container {
public:
  mstatus_t status = STATUS_DEFAULT;
  /** \brief Контенер идентификаторов строк вектора в БД */
  std::vector<int> id_vec;
};


/** \brief структура для сборки SELECT(и DELETE) запросов */
struct db_query_select_setup: public db_query_basesetup {
public:
  static db_query_select_setup *Init(db_table _table);

  virtual ~db_query_select_setup() = default;

public:
  /** \brief Сетап выражения where для SELECT/UPDATE/DELETE запросов */
  std::unique_ptr<class db_where_tree> where_condition;

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

  /** \brief Записать в out_vec строки model_info из данных values_vec,
    *   полученных из БД
    * \note Обратная операция для db_query_insert_setup::setValues */
  void SetData(std::vector<model_info> *out_vec);
  /** \brief Записать в out_vec строки calculation_info из данных values_vec,
    *   полученных из БД */
  void SetData(std::vector<calculation_info> *out_vec);
  /** \brief Записать в out_vec строки calculation_state_log из данных values_vec,
    *   полученных из БД */
  void SetData(std::vector<calculation_state_log> *out_vec);

protected:
  /** \brief Проверить соответствие строки strname и имени поля
    * \note todo: Заменить строки на int идентификаторы */
  bool isFieldName(const std::string &strname, const db_variable &var);

public:
  /** \brief Набор значений для операций INSERT/UPDATE */
  std::vector<row_values> values_vec;
};
inline bool db_query_select_result::isFieldName(
    const std::string &strname, const db_variable &var) {
  return strname == var.fname;
}


/** \brief Дерево where условий
  * \note В общем и целом:
  *   1) не очень оптимизировано
  *   2) строки которая хочет видеть СУБД могут отличаться от того что
  *     представлено в коде, поэтому оставим возможность их менять */
class db_where_tree {
public:
  static db_where_tree *Init(const model_info &where);
  static db_where_tree *Init(const calculation_info &where);
  static db_where_tree *Init(const calculation_state_log &where);

  ~db_where_tree();

  db_where_tree(const db_where_tree &) = delete;
  db_where_tree(db_where_tree &&) = delete;

  db_where_tree &operator=(const db_where_tree &) = delete;
  db_where_tree &operator=(db_where_tree &&) = delete;

  // db_condition_tree *GetTree() const {return root_;}
  /** \brief Собрать строку условного выражения */
  std::string GetString(db_condition_node::DataToStrF dts =
       db_condition_node::DataToStr) const;
  /** \brief Условно(не упорядочены), первая нода коллекции дерева */
  std::vector<db_condition_node *>::iterator TreeBegin();
  /** \brief Условно(не упорядочены), конечная нода коллекции дерева */
  std::vector<db_condition_node *>::iterator TreeEnd();

protected:
  db_where_tree();

  /** \brief Шаблон функции собирающей обычное дерево where условий
    *   разнесённых операторами AND
    * \badcode Не понятно как собрать шикарное дерево с множеством
    *   разнообразных условий */
  template <class TableT>
  static db_where_tree *init(const TableT &where) {
    /* todo: это конечно мрак */
    std::unique_ptr<db_query_insert_setup> qis(
        db_query_insert_setup::Init({where}));
    if (!qis)
      return nullptr;
    if (qis->values_vec.empty())
      return nullptr;
    db_where_tree *wt = new db_where_tree();
    auto &row = qis->values_vec[0];
    const auto &fields = qis->fields;
    for (const auto &x : row) {
      auto i = x.first;
      if (i != db_query_basesetup::field_index_end && i < fields.size()) {
        auto &f = fields[i];
        wt->source_.push_back(new db_condition_node(
            f.type, f.fname, x.second));
      }
    }
    if (wt->source_.size() == 1) {
      // только одно условие выборки
      wt->root_ = wt->source_[0];
    } else if (wt->source_.size() > 1) {
      std::generate_n(std::back_insert_iterator<std::vector<db_condition_node *>>
          (wt->source_), wt->source_.size() - 1,
          []() { return new db_condition_node(
              db_condition_node::db_operator_t::op_and); });
      wt->construct();
    }
    return wt;
  }

  /** \brief Собрать дерево условий по вектору узлов условий source_ */
  void construct();

protected:
  /** \brief Контейнер-хранилище узлов условий */
  std::vector<db_condition_node *> source_;
  /** \brief Корень дерева условий */
  db_condition_node *root_ = nullptr;
  /** \brief Результирующая строка собранная из дерева условий */
  std::string data_;
};

#endif  // !_DATABASE__DB_QUERIES_SETUP_H_
