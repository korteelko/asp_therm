/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * db_connection *
 *   Здесь прописан функционал инициализации подключения,
 * в том числе чтения файла конфигурации и API взаимодействия с БД.
 *   API юзерских операций прописан в файле DBConnectionManager
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_CONNECTION_H_
#define _DATABASE__DB_CONNECTION_H_

#include "Common.h"
#include "db_defines.h"
#include "db_queries_setup.h"
#include "db_query.h"
#include "db_tables.h"
#include "ErrorWrap.h"

#include <string>
#include <utility>
#include <vector>


#ifndef IS_DEBUG_MODE
#  define IS_DEBUG_MODE false
#endif  // !IS_DEBUG_MODE

namespace asp_db {
/** \brief Структура содержит параметры подключения */
struct db_parameters {
public:
  /** \brief Флаг блокирования физического подключения к базе данных, просто
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

  /** \brief Получить информацию о параметрах соединения с БД */
  std::string GetInfo() const;
};


/* develop: может ещё интерфейс сделать, а потом
 *   абстрактный класс */
/** \brief Абстрактный класс подключения к БД */
class DBConnection {
public:
  virtual ~DBConnection();

  /** \brief Добавить метку сохранения */
  virtual mstatus_t AddSavePoint(const db_save_point &sp) = 0;
  /** \brief Откатиться к метке сохранения */
  virtual void RollbackToSavePoint(const db_save_point &sp) = 0;

  /** \brief Установка соединения */
  virtual mstatus_t SetupConnection() = 0;
  /** \brief Закрытие соединения */
  virtual void CloseConnection() = 0;

  /** \brief Проверить существование таблицы */
  virtual mstatus_t IsTableExists(db_table t, bool *is_exists) = 0;
  /** \brief Вытащить формат таблицы из СУБД */
  virtual mstatus_t GetTableFormat(db_table t, db_table_create_setup *fields) = 0;
  /** \brief Проверить формат таблицы
    * \note Как метод 'UpdateTable', только не изменяет состояние таблицы */
  virtual mstatus_t CheckTableFormat(const db_table_create_setup &fields) = 0;
  /** \brief Обновить формат таблицы */
  virtual mstatus_t UpdateTable(const db_table_create_setup &fields) = 0;
  /** \brief Создать таблицу */
  virtual mstatus_t CreateTable(const db_table_create_setup &fields) = 0;
  /** \brief Удалить таблицу */
  virtual mstatus_t DropTable(const db_table_drop_setup &drop) = 0;

  /** \brief Добавить новые строки в БД, выданные id записать в
    *   результирующий вектор
    * \return STATUS_OK если строки добавлена
    * \note Пока не понятно что с вектором выходных данных */
  virtual mstatus_t InsertRows(const db_query_insert_setup &insert_data,
      id_container  *id_vec) = 0;
  /** \brief Удалить строки из БД */
  virtual mstatus_t DeleteRows(const db_query_delete_setup &delete_data) = 0;
  /** \brief Выбрать строки из БД */
  virtual mstatus_t SelectRows(const db_query_select_setup &select_data,
      db_query_select_result *result_data) = 0;
  /** \brief Обновить строки БД */
  virtual mstatus_t UpdateRows(const db_query_update_setup &update_data) = 0;

  mstatus_t GetStatus() const;
  merror_t GetErrorCode() const;
  bool IsOpen() const;
  /** \brief залогировать ошибку */
  void LogError();

protected:
  /* todo: add to parameters of constructor link to class
   *   DBConnectionCreator for closing all
   *   DBConnection inheritance classes */
  DBConnection(const IDBTables *tables, const db_parameters &parameters);

  /* функции сбора строки запроса */
  /** \brief Сбор строки запроса создания точки сохранения */
  virtual std::stringstream setupAddSavePointString(const db_save_point &sp);
  /** \brief Сбор строки запроса отката до точки сохранения */
  virtual std::stringstream setupRollbackToSavePoint(const db_save_point &sp);
  /** \brief Сбор строки запроса существования таблицы */
  virtual std::stringstream setupTableExistsString(db_table t) = 0;
  /** \brief Сбор строки на получение имён столбцов таблицы
    * \note не знаю как она в унифицированом виде может выглядеть */
  virtual std::stringstream setupGetColumnsInfoString(db_table t) = 0;
  /** \brief Сбор строки для добавления колонки */
  virtual std::stringstream setupAddColumnString(
      const std::pair<db_table, const db_variable &> &pdv);
  /** \brief Сбор строки запроса для создания таблицы */
  virtual std::stringstream setupCreateTableString(
      const db_table_create_setup &fields);
  /** \brief Сбор строки запроса для удаления таблицы
    * \note Тут ещё опции RESTRICT|CASCADE */
  virtual std::stringstream setupDropTableString(
      const db_table_drop_setup &drop);
  /** \brief Сбор строки запроса для добавления строки */
  virtual std::stringstream setupInsertString(
      const db_query_insert_setup &fields);
  /** \brief Сбор строки запроса для удаления строки */
  virtual std::stringstream setupDeleteString(
      const db_query_delete_setup &fields);
  /** \brief Сбор строки запроса для получения выборки */
  virtual std::stringstream setupSelectString(
      const db_query_select_setup &fields);
  /** \brief Сбор строки запроса для обновления строки */
  virtual std::stringstream setupUpdateString(
      const db_query_update_setup &fields);

  /** \brief собрать строку поля БД по значению db_variable
    * \note Оказалось завязано на интерпретацию переменных
    * \todo Собственно заменить прямые обращение на
    *   вызовы виртуальных функций
    */
  virtual std::string db_variable_to_string(const db_variable &dv) = 0;
  /** \brief собрать строку сложный уникальный парметр */
  virtual std::string db_unique_constrain_to_string(
      const db_table_create_setup &cs);
  /** \brief собрать строку ссылки на другую таблицу
    *   по значению db_reference */
  virtual std::string db_reference_to_string(const db_reference &ref);
  /** \brief собрать строку первичного ключа */
  virtual std::string db_primarykey_to_string(const db_complex_pk &pk);

  /** \brief Подключение к БД сымитировано */
  bool isDryRun();

protected:
  ErrorWrap error_;
  /** \brief Статус подключения */
  mstatus_t status_;
  /** \brief Параметры подключения к базе данных */
  db_parameters parameters_;
  /** \brief Указатель на имплементацию таблиц */
  const IDBTables *tables_;
  /** \brief Флаг подключения к бд */
  bool is_connected_;
};
}  // namespace asp_db

#endif  // !_DATABASE__DB_CONNECTION_H_
