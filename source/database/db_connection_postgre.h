/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_CONNECTION_POSTGRESQL_H_
#define _DATABASE__DB_CONNECTION_POSTGRESQL_H_

#include "common.h"

#if defined(WITH_POSTGRESQL)
#include "db_connection.h"

#include <pqxx/pqxx>

#include <memory>
#include <sstream>
#include <string>


// смотри страницу:
// https://www.tutorialspoint.com/postgresql/postgresql_c_cpp.htm
/** \brief реализация DBConnection для postgresql */
class DBConnectionPostgre final: public DBConnection {
public:
  DBConnectionPostgre(const db_parameters &parameters);
 // mstatus_t ExecuteQuery(DBQuery *query) override;

  void Commit() override;
  void Rollback() override;

  mstatus_t CreateTable(const db_table_create_setup &fields) override;
  void UpdateTable(db_table t, const db_table_update_setup &vals) override;

  // void InsertModelInfo(const model_info &mi) override;
  void InsertRow(const db_table_update_setup &insert_data) override;
  void DeleteRow(const db_table_update_setup &delete_data) override;
  void SelectModelInfo(const db_table_update_setup &select_data) override;

  void InsertCalculationInfo(const calculation_info &ci) override;
  void InsertCalculationStateLog(const calculation_state_info &sl) override;

  /* checked functions */
  mstatus_t SetupConnection() override;
  void CloseConnection() override;

  mstatus_t IsTableExists(db_table t, bool *is_exists) override;

  ~DBConnectionPostgre() override;

private:
  std::string setupConnectionString();
  std::stringstream setupCreateTableString(
      const db_table_create_setup &fields);

  /** \brief собрать строку поля БД по значению db_variable */
  std::string db_variable_to_string(const db_variable &dv);
  /** \brief собрать строку ссылки на другую таблицу
    *   по значению db_reference */
  std::string db_reference_to_string(const db_reference &ref);
  /** \brief собрать строку первичного ключа */
  std::string db_primarykey_to_string(const db_complex_pk &pk);

  /** \brief Переформатировать строку общего формата даты
    *   'yyyy/mm/dd' к формату используемому БД */
  std::string dateToPostgreDate(const std::string &date);
  /** \brief Переформатировать строку общего формата времени
    *   'hh:mm:ss' к формату используемому БД */
  std::string timeToPostgreTime(const std::string &time);
  /** \brief Переформатировать строку общего формата даты БД
    *   к формату 'yyyy/mm/dd' */
  std::string postgreDateToDate(const std::string &pdate);
  /** \brief Переформатировать строку общего формата времени БД
    *   к формату 'hh:mm:ss' */
  std::string postgreTimeToTime(const std::string &ptime);

private:
  /** \brief Указатель на объект подключения */
  std::unique_ptr<pqxx::connection> pconnect_;
};

#endif  // WITH_POSTGRESQL
#endif  // !_DATABASE__DB_CONNECTION_POSTGRESQL_H_
