/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * db_defines *
 *   Здесь прописаны структуры-примитивы для обеспечения
 * взаимодействия с базой данных
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_DEFINES_H_
#define _DATABASE__DB_DEFINES_H_

#include "ErrorWrap.h"

#include <exception>
#include <string>
#include <vector>

#include <stdint.h>


/** \brief Перечисление всех таблиц из файла db_tables.h */
enum class db_table;

/** \brief Уникальный идентификатор поля БД */
typedef uint32_t db_variable_id;

/** \brief клиент БД */
enum class db_client: uint32_t {
  NOONE = 0,
  /// реализация в db_connection_postgre.cpp
  POSTGRESQL = 1
};
/** \brief Получить имя клиента БД по идентификатору */
std::string db_client_to_string(db_client client);

/** \brief Структура описывающая столбец таблицы БД.
  *   собственно, описание валидно и для знчения в этом столбце,
  *   так что можно чекать формат/неформат  */
struct db_variable {
public:
  /** \brief Перечисление допустимых типов данных
    * \note Идея расширить функционал на вывод в файл.
    *   По сетап данным из db_queries_setup.h можно
    *   собирать столбцы данных, специализируя их тип и оотображение.
    *   Также можно расчитывать на реализацию обратного функционала -
    *   чтение из форматированных файлов.
    * \todo А как правильно разграничить 'text' и 'char_array'
    *   postgre вот не парится, text'ом назвал char_array */
  enum class db_var_type {
    /** пустой тип */
    type_empty = 0,
    /** автоинкрементируюмое поле id
      *   в postgresql это SERIAL */
    type_autoinc,
    /** для хранения Universal Unique Identificator(RFC 4122) */
    type_uuid,
    /** boolean */
    type_bool,
    /** 2байт int {-32,768; 32,767} */
    type_short,
    /** 4байт int {-2,147,483,648; 2,147,483,647} */
    type_int,
    /** 8байт int {-9,223,372,036,854,775,808; 9,223,372,036,854,775,807} */
    type_long,
    /** 4байт число с п.т. */
    type_real,
    /** дата */
    type_date,
    /** время */
    type_time,
    /** массив символов */
    type_char_array,
    /** просто текст(в postgresql такой есть) */
    type_text,
    /** поле сырых данных blob */
    type_blob
  };

  /** \brief Флаги полей таблицы */
  struct db_variable_flags {
  public:
    db_variable_flags() = default;

  public:
    /** \brief Значение явдяется первичным ключом */
    bool is_primary_key = false;
    /** \brief Значение является ссылкой на столбец другой таблицы */
    bool is_reference = false;
    /** \brief Значение должно быть уникальным */
    bool is_unique = false;
    /** \brief Значение может быть инициализировано */
    bool can_be_null = true;
    /** \brief Значение параметра входит в массив данных, которые
      *   учитываются(МАССИВ должен быть уникальным) при попытке
      *   включения нового элемента в таблицу
      * \note Переменная введена для конфигурации SELECT подобных запросов
      *   Example: [1,2,3] == [1,2,3]; [1,2,3] != [1,2,4]; [1,2,3] != [1,3,2]
      * \todo unique набор может быть не один, убрать эту галку,
      *   переделать создание таблицы */
    // bool in_unique_constrain = false;
    /** \brief Число может быть отрицательным
      * \note only for numeric types */
    bool can_be_negative = false;
    /** \brief Значение представляет собой массив(актуально для типа char) */
    bool is_array = false;
    bool has_default = false;
  };

public:
  /** \brief id столбца/параметра, чтоб создавать и апдейтить */
  db_variable_id fid;
  /** \brief имя столбца/параметра, чтоб создавать и апдейтить */
  const char *fname;
  /** \brief тип значения */
  db_var_type type;
  /** \brief флаги колонуи таблицы */
  db_variable_flags flags;
  /** \brief дефолтное значение, если есть */
  std::string default_str;
  /** \brief для массивов - количество элементов
    * \default 1 */
  int len;

public:
  db_variable(db_variable_id fid, const char *fname, db_var_type type,
      db_variable_flags flags, int len = 1);

  bool operator==(const db_variable &r) const;
  bool operator!=(const db_variable &r) const;

  /** \brief Проверить несовместимы ли флаги и другие параметры */
  merror_t CheckYourself() const;
};


/** \brief структура содержащая параметры первичного ключа */
struct db_complex_pk {
  std::vector<std::string> fnames;
};

/** \brief enum действий над объектами со ссылками на другие элементами:
  *   колонками другой таблицы или самими таблицами */
enum class db_reference_act {
  /** \brief Ничего не делать */
  ref_act_not = 0,
  /** \brief Установить в ноль */
  ref_act_set_null,
  /** \brief Установить значение по умолчанию
    * \note Оно посложнее в реализации, необходимо сначала будет
    *   подправить и оттестировать просто работу со значениями по умолчанию
    * \todo Собственно добавить */
  // ref_act_set_default,
  /** \brief Изменить зависимые объекты и объекты зависимые от зависимых */
  ref_act_cascade,
  /** \brief Не изменять объект при наличии связей */
  ref_act_restrict
};
// static_assert (0, "добавить нереализованные действия");


/** \brief структура содержащая параметры удалённого ключа */
struct db_reference {
public:
  /** \brief собственное имя параметра таблицы */
  std::string fname;
  /** \brief таблица на которую ссылается параметр */
  db_table foreign_table;
  /** \brief имя параметра в таблице foreign_table */
  std::string foreign_fname;

  /** \brief флаг 'внешнего ключа' */
  bool is_foreign_key = false;
  /** \brief флаг наличия on_delete метода */
  bool has_on_delete = false;
  /** \brief флаг наличия on_update метода */
  bool has_on_update = false;
  /** \brief метод удаления значения
    * \default db_on_delete::empty */
  db_reference_act delete_method;
  db_reference_act update_method;

public:
  db_reference(const std::string &fname, db_table ftable,
      const std::string &f_fname, bool is_fkey,
      db_reference_act on_del = db_reference_act::ref_act_not,
      db_reference_act on_upd = db_reference_act::ref_act_not);

  bool operator==(const db_reference &r) const;
  bool operator!=(const db_reference &r) const;

  /** \brief Проверить несовместимы ли флаги и другие параметры */
  merror_t CheckYourself() const;

  /** \brief Получить строковое представление update|delete action */
  static std::string GetReferenceActString(db_reference_act act);
};

/** \brief Класс исключений для выполнения операций с БД */
class db_exception: public std::exception {
public:
  db_exception(merror_t error, const std::string &msg);
  db_exception(const std::string &msg);

  void LogException();
  merror_t GetError() const;
  std::string GetErrorMessage() const;

private:
  /** \brief Ошибка, сюда же пишется сообщение */
  ErrorWrap error_;
};

typedef std::vector<db_variable> db_fields_collection;
typedef std::vector<db_reference> db_ref_collection;
using db_type = db_variable::db_var_type;

#endif  // !_DATABASE__DB_DEFINES_H_
