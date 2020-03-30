/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_CONNECTION_H_
#define _DATABASE__DB_CONNECTION_H_

#include "common.h"
#include "db_defines.h"
#include "db_query.h"
#include "ErrorWrap.h"

#include <string>
#include <variant>
#include <vector>

#include <assert.h>


/** \brief структура описывающая столбец таблицы БД.
  *   собственно, описание валидно и для знчения в этом столбце,
  *   так что можно чекать формат/неформат  */
struct db_variable {
public:
  enum class db_var_type {
    /** автоинкрементируюмое поле id
      *   в postgresql это SERIAL */
    type_autoinc = 0,
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

  struct db_variable_flags {
    bool is_primary_key = false;
    bool is_reference = false;
    bool is_unique = false;
    bool can_be_null = true;
    /** \note only for numeric types */
    bool can_be_negative = false;
    bool is_array = false;
    bool has_default = false;
  };

public:
  /** \brief имя столбца/параметра, чтоб создавать и апдейтить */
  std::string fname;
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
  db_variable(std::string fname, db_var_type type,
      db_variable_flags flags, int len = 1);

  /** \brief проверить несовместимы ли флаги и другие параметры */
  merror_t CheckYourself() const;
};


/** \brief структура содержащая параметры первичного ключа */
struct db_complex_pk {
  std::vector<std::string> fnames;
};

/** \brief структура содержащая параметры удалённого ключа */
struct db_reference {
public:
  enum class db_reference_act {
    ref_act_empty = 0,
    // ref_act_set_null,
    // ref_act_set_default,
    ref_act_cascade,
    ref_act_restrict
  };

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
      db_reference_act on_del = db_reference_act::ref_act_empty,
      db_reference_act on_upd = db_reference_act::ref_act_empty);

  /** \brief проверить несовместимы ли флаги и другие параметры */
  merror_t CheckYourself() const;

  static std::string GetReferenceActString(db_reference_act act);
};

typedef std::vector<db_variable> db_fields_collection;
typedef std::vector<db_reference> db_ref_collection;
using db_type = db_variable::db_var_type;


/* todo: можно наверное общую имплементацию
 *   db_table_create_setup и db_table_select_setup
 *   вынести в структуру db_table_setup */
struct db_table_create_setup {
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

public:
  db_table_create_setup(db_table table, const db_fields_collection &fields);
};

/** \brief структура для сборки SELECT/UPDATE запросов */
struct db_table_select_setup {
  typedef uint32_t field_index;

public:
  ErrorWrap error;
  db_table table;
  const db_fields_collection &fields;
  std::map<field_index, std::string> values;

public:
  db_table_select_setup(db_table table, const db_fields_collection &fields);
};

/** \brief получить сетап на создание таблицы */
const db_table_create_setup &get_table_create_setup(db_table dt);

/** \brief структура содержит параметры коннектинга */
struct db_parameters {
public:
  /** \brief не подключаться к базе данных, просто
    * выводить получившееся запросы в stdout(или логировать) */
  bool is_dry_run;
  /** \brief тип клиента */
  db_client supplier;
  /** \brief параметры подключения базы данных
    * \note сделаю как в джанго */
  std::string name,
              username,
              password,
              host;
  int port;

public:
  db_parameters();

  merror_t SetConfigurationParameter(const std::string &param_strtpl,
      const std::string &param_value);

  std::string GetInfo() const;
};


struct model_info;
struct calculation_info;
struct calculation_state_log;

/* develop: может ещё интерфейс сделать, а потом
 *   абстрактный класс */
 /* TODO: rename class */
/** \brief абстрактный класс подключения к БД */
class DBConnection {
public:
  /* хз... */
  // virtual mstatus_t ExecuteQuery(const std::string &query_body) = 0;

  virtual void Commit() = 0;
  virtual void Rollback() = 0;

  /* todo: вынести это всё в соответствующий класс
   *   здесь должны быть только низкоуровневые функции
   *   Update, Insert, Select */
  /* вызывается командами! */
  virtual mstatus_t CreateTable(const db_table_create_setup &fields) = 0;
  virtual void UpdateTable(db_table t, const db_table_select_setup &vals) = 0;

  // todo - replace argument 'const model_info &mi'
  //   with 'const db_table_select_setup &mip'
  virtual void InsertModelInfo(const model_info &mi) = 0;
  virtual void SelectModelInfo(const db_table_select_setup &mip) = 0;

  virtual void InsertCalculationInfo(
      const calculation_info &ci) = 0;
  virtual void InsertCalculationStateLog(
      const calculation_state_log &sl) = 0;
  /* посюда */

  virtual mstatus_t SetupConnection() = 0;
  virtual void CloseConnection() = 0;

  virtual mstatus_t IsTableExists(db_table t, bool *is_exists) = 0;

  virtual ~DBConnection();

  mstatus_t GetStatus() const;
  merror_t GetErrorCode() const;
  bool IsOpen() const;
  /** \brief залогировать ошибку */
  void LogError();

protected:
  /* todo: add to parameters of constructor link to class
   *   DBConnectionCreator for closing all
   *   DBConnection inheritance classes */
  DBConnection(const db_parameters &parameters);

protected:
  ErrorWrap error_;
  /** \brief статус подключения */
  mstatus_t status_;
  /** \brief параметры подключения к базе данных */
  db_parameters parameters_;
  /** \brief флаг подключения к бд */
  bool is_connected_;
  /** \brief флаг работы без подключения к бд */
  bool is_dry_run_;
};

#endif  // !_DATABASE__DB_CONNECTION_H_
