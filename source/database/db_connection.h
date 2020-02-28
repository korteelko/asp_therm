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
  /* не знаю получится ли реализовать
   *   пока это место сыровато - сного лишнего */
  enum class db_type {
    /** автоинкрементируюмое поле id
      *   в postgresql это SERIAL */
    db_type_autoinc = 0,
    /** для хранения Universal Unique Identificator(RFC 4122) */
    db_type_uuid,
    /** boolean */
    db_type_bool,
    /** 2байт int {-32,768; 32,767} */
    db_type_short,
    /** 4байт int {-2,147,483,648; 2,147,483,647} */
    db_type_int,
    /** 8байт int {-9,223,372,036,854,775,808; 9,223,372,036,854,775,807} */
    db_type_long,
    /** 4байт число с п.т. */
    db_type_real,
    /** дата */
    db_type_date,
    /** время */
    db_type_time,
    /** массив символов */
    db_type_char_array,
    /** просто текст(в postgresql такой есть) */
    db_type_text,
    /** поле сырых данных blob */
    db_type_blob
  };

public:
  struct db_variable_flags {
    // bool is_primary_key = false;
    bool is_unique = false;
    bool can_be_null = true;
    /** \only for numeric types */
    bool can_be_negative = false;
    bool is_array = false;
    bool has_default = false;
  };

public:
  /** \brief имя столбца/параметра, чтоб создавать и апдейтить */
  std::string fname;
  /** \brief тип значения */
  db_type type;
  /** \brief флаги колонуи таблицы */
  db_variable_flags flags;
  /** \brief дефолтное значение, если есть */
  std::string default_str;
  /** \brief для массивов - количество элементов
    * \default 1 */
  int len;

public:
  db_variable(std::string fname, db_type type,
      db_variable_flags flags, int len = 1);

  /** \brief проверить несовместимы ли флаги и другие параметры */
  ErrorWrap CheckYourself() const;
};

using db_type = db_variable::db_type;

/* TODO: функции составления полей(db_variable) по умолчанию */
/* набор функций для разных типов данных db параметры
 *   которых отличаются от дефолтных
 * UPD: блять, вынести всё в отдельный файл */
inline db_variable default_autoinc() {
  return db_variable("id", db_type::db_type_autoinc,
      {.is_unique = true,
       .can_be_null = false});
}

/* TODO: а pk то наверно и не нужен, в db_variable
 *   флага достаточно наверно(проверить) */
/** \brief структура содержащая параметры первичного ключа */
struct db_complex_pk {
  std::vector<db_variable> elements;
};
/** \brief структура содержащая параметры удалённого ключа */
struct db_complex_fk {
public:
  enum class db_on_delete {
    on_delete_empty = 0,
    on_delete_cascade,
    on_delete_restrict
  };

public:
  /** \brief набор элементов таблицы которые ссылаются на ключи другой */
  std::vector<db_variable> element;
  /** \brief набор элементов таблицы которые ссылаются на ключи другой */
  db_table foreign_table;
  /** \brief набор элементов таблицы которые ссылаются на ключи другой */
  std::vector<db_variable> foreign_element;

  bool has_ON_DELETE = false;
  /** \brief метод удаления значения
    * \default db_on_delete::empty */
  db_on_delete delete_method;
};

typedef std::variant<db_variable, db_complex_pk, db_complex_fk>
    create_table_variant;

/* todo: такс, выписать sql запросы для этих функций */
/** \brief функция собирающая набор полей для
  *   создания таблицы БД model_info информации о модели */
std::vector<create_table_variant> table_model_info();
/** \brief функция собирающая набор полей для
  *   создания таблицы БД calculation_info информации о расчёте */
std::vector<create_table_variant> table_calculation_info();
/** \brief функция собирающая набор полей для
  *   создания таблицы БД calculation_state_log строку расчёта */
std::vector<create_table_variant> table_calculation_state_log();


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

  /* ъ вынести это всё в соответствующий класс
   *   здесь должны быть только низкоуровневые функции
   *   Update, Insert, Select */
  /* вызывается командами! */
  virtual void CreateTable(db_table t,
      const std::vector<create_table_variant> &components) = 0;
  virtual void UpdateTable(db_table t,
      const std::vector<create_table_variant> &components) = 0;

  virtual void InsertModelInfo(const model_info &mi) = 0;
  virtual void SelectModelInfo(
      const std::vector<db_variable> &mip) = 0;

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
  DBConnection(const db_parameters &parameters);

protected:
  ErrorWrap error_;
  /** \brief статус подключения. Для этого класса кроме обычных
    *   статусных дефайнов может быть установлен 'STATUS_DRY_RUN' */
  mstatus_t status_;
  /** \brief параметры подключения к базе данных */
  db_parameters parameters_;
  /** \brief флаг подключения к бд */
  bool is_connected_;
};

#endif  // !_DATABASE__DB_CONNECTION_H_
