#ifndef _CORE__SUBROUTINS__DB_CONNECTION_H_
#define _CORE__SUBROUTINS__DB_CONNECTION_H_

#include "common.h"
#include "db_query.h"
#include "models_errors.h"

#include <string>
#include <variant>
#include <vector>

#include <assert.h>

/** \brief клиент БД */
enum class db_client: uint32_t {
  NOONE = 0,
  /// реализация в db_connection_postgre.cpp
  POSTGRESQL = 1
};

/** \brief перечисление типов используемых таблиц */
enum class db_table {
  /** информация о ревизии уравнения состояния */
  table_model_info = 0,
  /* TODO: добавить структуру с++, содержащую
   *   данные для этой таблицы*/
  /** информация о расчёте */
  table_calculation_info = 1, // reference to 'table_model_info '
  /** лог расчёта */
  table_calculation_state_log = 2  // foreign key to 'table_calculation_info '
};

std::string get_table_name(db_table dt);

/* если запрос как набор типизированных строк
 *   то необходимо определить строки неформата.
 *   например запрос "создать таблицу" может содержать
 *   комплексный первичный ключ, прописываемый в отд. строке */
enum class db_query_str {
  query_str_variable = 0,
  query_str_complex_pk,
  query_str_complex_fk
};

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
  enum class db_on_delete {
    empty = 0,
    cascade,
    restrict
  };

public:
  struct db_variable_flags {
    bool is_primary_key = false;
    bool is_unique = false;
    bool can_be_null = true;
    /** \only for numeric types */
    bool can_be_negative = false;
    bool is_array = false;
    bool is_foreign_key = false;
    /** \note это есть в postgresql */
    bool has_reference = false;
    bool has_default = false;
    bool not_empty_on_delete = false;
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
  /** \brief метод удаления значения
    * \default db_on_delete::empty */
  db_on_delete delete_method;
  /** \brief для массивов - количество элементов
    * \default 1 */
  int len;

public:
  db_variable(std::string fname, db_type type,
      db_variable_flags flags, int len = 1);
};

using db_type = db_variable::db_type;

/* TODO: функции составления полей(db_variable) по умолчанию */
/* набор функций для разных типов данных db параметры
 *   которых отличаются от дефолтных
 * UPD: блять, вынести всё в отдельный файл */
inline db_variable defaul_autoinc() {
  return db_variable("id", db_type::db_type_autoinc,
      {.is_primary_key = true,
       .is_unique = true,
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
  /** \brief набор элементов таблицы которые ссылаются на ключи другой */
  std::vector<db_variable> element;
  /** \brief набор элементов таблицы которые ссылаются на ключи другой */
  db_table foreign_table;
  /** \brief набор элементов таблицы которые ссылаются на ключи другой */
  std::vector<db_variable> foreign_element;
};

typedef std::variant<db_variable, db_complex_pk, db_complex_fk>
    create_table_variant;

std::vector<create_table_variant> table_model_info();
std::vector<create_table_variant> table_calculation_info();
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
};

struct model_info;
struct calculation_info;
struct calculation_state_log;

/* develop: может ещё интерфейс сделать, а потом
 *   абстрактный класс */
/** \brief абстрактный класс подключения к БД */
class DBConnection {
public:
  /* хз... */
  virtual mstatus_t ExecuteQuery(const std::string &query_body) = 0;

  virtual mstatus_t CheckConnection(const db_parameters &parameters) = 0;

  /** \brief */
  virtual void Commit() = 0;
  virtual void Rollback() = 0;

  virtual bool IsTableExist(db_table t) = 0;
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

  virtual ~DBConnection();

  mstatus_t GetStatus() const;
  merror_t GetErrorCode() const;

protected:
  DBConnection();

protected:
  ErrorWrap error_;
  mstatus_t status_;
  db_parameters parameters_;
  /** \brief контайнер запросов для сложных запросов */
  QueryContainer queries_;
};

#endif  // !_CORE__SUBROUTINS__DB_CONNECTION_H_
