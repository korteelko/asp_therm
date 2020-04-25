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
#include "program_state.h"

#include "Logging.h"

#include <pqxx/pqxx>

#include <functional>
#include <memory>
#include <sstream>
#include <string>


// смотри страницу:
// https://www.tutorialspoint.com/postgresql/postgresql_c_cpp.htm
/** \brief реализация DBConnection для postgresql */
class DBConnectionPostgre final: public DBConnection {
public:
  DBConnectionPostgre(const db_parameters &parameters);
  ~DBConnectionPostgre() override;

  /* хз... */
  void Commit() override;
  void Rollback() override;

  mstatus_t SetupConnection() override;
  void CloseConnection() override;

  mstatus_t IsTableExists(db_table t, bool *is_exists) override;
  mstatus_t CreateTable(const db_table_create_setup &fields) override;
  void UpdateTable(db_table t, const db_table_update_setup &vals) override;

  mstatus_t InsertRows(const db_table_insert_setup &insert_data) override;
  mstatus_t DeleteRows(const db_table_delete_setup &delete_data) override;
  mstatus_t SelectRows(const db_table_select_setup &select_data) override;
  mstatus_t UpdateRows(const db_table_update_setup &update_data) override;

private:
  template <class DataT, class OutT, class SetupF, class ExecF>
  mstatus_t exec_op(const DataT &data, OutT *res,
      SetupF setup_m, ExecF exec_m) {
    // setup content of query
    std::stringstream sstr = std::invoke(setup_m, *this, data);
    if (is_connected_ && pconnect_) {
      if (pconnect_->is_open() && !error_.GetErrorCode()) {
        try {
          // execute query
          std::invoke(exec_m, *this, sstr, res);
          if (IS_DEBUG_MODE)
            Logging::Append(io_loglvl::debug_logs,
                "Запрос БД на создание таблицы:" + sstr.str() + "\n\t");
        } catch (const pqxx::undefined_table &e) {
          status_ = STATUS_HAVE_ERROR;
          error_.SetError(ERROR_DB_TABLE_EXISTS, "Exception text: " +
              std::string(e.what()) + "\n Query: " + e.query());
        } catch (const pqxx::integrity_constraint_violation &e) {
          status_ = STATUS_HAVE_ERROR;
          error_.SetError(ERROR_DB_SQL_QUERY, "Exception text: " +
              std::string(e.what()) + "\n Query: " + e.query());
        } catch (const std::exception &e) {
           error_.SetError(ERROR_DB_CONNECTION,
               "Подключение к БД: exception. Запрос:\n" + sstr.str()
               + "\nexception what: " + e.what());
           status_ = STATUS_HAVE_ERROR;
        }
      } else {
        // is not connected
        if (!error_.GetErrorCode()) {
          error_.SetError(ERROR_DB_CONNECTION, "Соединение с бд не открыто");
          status_ = STATUS_NOT;
        }
      }
    } else {
      if (is_dry_run_) {
        // dry_run_ programm setup
        Logging::Append(io_loglvl::debug_logs, "dry_run: " + sstr.str());
      } else {
        // error - not connected
        error_.SetError(ERROR_DB_CONNECTION);
        status_ = STATUS_NOT;
      }
    }
    return status_;
  }

  std::string setupConnectionString();
  std::stringstream setupTableExistsString(db_table t) override;
  std::stringstream setupInsertString(
       const db_table_insert_setup &fields) override;

  std::string db_variable_to_string(const db_variable &dv) override;

  /* функции исполнения запросов */
  /** \brief Запрос существования таблицы */
  void execIsTableExists(const std::stringstream &sstr, bool *is_exists);
  /** \brief Запрос создания таблицы */
  void execCreateTable(const std::stringstream &sstr, void *);
  /** \brief Запрос на добавление строки */
  void execInsert(const std::stringstream &sstr, void *);
  /** \brief Запрос на удаление строки */
  void execDelete(const std::stringstream &sstr, void *);
  /** \brief Запрос выборки из таблицы
    * \note изменить 'void *' выход на 'pqxx::result *result' */
  void execSelect(const std::stringstream &sstr, pqxx::result *result);
  /** \brief Запрос на обновление строки */
  void execUpdate(const std::stringstream &sstr, void *);

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
  // add result data for select queries for example, or IsTableExist
};

#endif  // WITH_POSTGRESQL
#endif  // !_DATABASE__DB_CONNECTION_POSTGRESQL_H_
