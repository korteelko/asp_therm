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

#include "ErrorWrap.h"
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

  mstatus_t AddSavePoint(const db_save_point &sp) override;
  void RollbackToSavePoint(const db_save_point &sp) override;

  mstatus_t SetupConnection() override;
  void CloseConnection() override;

  mstatus_t IsTableExists(db_table t, bool *is_exists) override;
  mstatus_t CheckTableFormat(const db_table_create_setup &fields) override;
  mstatus_t UpdateTable(const db_table_create_setup &fields) override;
  mstatus_t CreateTable(const db_table_create_setup &fields) override;

  mstatus_t InsertRows(const db_query_insert_setup &insert_data) override;
  mstatus_t DeleteRows(const db_query_delete_setup &delete_data) override;
  mstatus_t SelectRows(const db_query_select_setup &select_data,
      db_query_select_result *result_data) override;
  mstatus_t UpdateRows(const db_query_update_setup &update_data) override;

private:
  /** \brief Добавить бэкап точку перед операцией изменяющей
    *   состояние таблицы */
  mstatus_t addSavePoint();

  /** \brief Шаблон функции оборачивающий операции с БД:
    *  1) Собрать запрос.
    *  2) Отправить и обработать результат.
    *  3) Обработать ошибки.
    * \param data данные для сборки запроса
    * \param res указатель на хранилище результата
    * \param setup_m функция сборки текста запроса
    * \param exec_m функция отправки запроса, парсинга результата */
  template <class DataT, class OutT, class SetupF, class ExecF>
  mstatus_t exec_wrap(const DataT &data, OutT *res,
      SetupF setup_m, ExecF exec_m) {
    // setup content of query
    std::stringstream sstr = std::invoke(setup_m, *this, data);
    sstr.seekg(0, std::ios::end);
    auto sstr_len = sstr.tellg();
    if (pqxx_work.IsAvailable() && sstr_len) {
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
        // исключение пробрасывается при ошибке создания строки
        //   например если строка с таким комплексом уникальных
        //   значений уже существует
        status_ = STATUS_HAVE_ERROR;
        error_.SetError(ERROR_DB_SQL_QUERY, "Exception text: " +
            std::string(e.what()) + "\n Query: " + e.query());
      } catch (const std::exception &e) {
         status_ = STATUS_HAVE_ERROR;
         error_.SetError(ERROR_DB_CONNECTION,
             "Подключение к БД: exception. Запрос:\n" + sstr.str()
             + "\nexception what: " + e.what());
      }
    } else {
      if (is_dry_run_) {
        status_ = STATUS_OK;
        // dry_run_ programm setup
        if (sstr_len) {
          Logging::Append(io_loglvl::debug_logs, "dry_run: " + sstr.str());
        } else {
          Logging::Append(io_loglvl::debug_logs, "dry_run: 'empty query!'");
        }
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
  std::stringstream setupColumnNamesString(db_table t) override;
  std::stringstream setupInsertString(
       const db_query_insert_setup &fields) override;

  std::string db_variable_to_string(const db_variable &dv) override;

  /* функции исполнения запросов */
  /** \brief Обычный запрос к БД без возвращаемого результата */
  void execNoReturn(const std::stringstream &sstr);

  /** \brief Запрос создания метки сохранения */
  void execAddSavePoint(const std::stringstream &sstr, void *);
  /** \brief Запрос отката к метке сохранения */
  void execRollbackToSavePoint(const std::stringstream &sstr, void *);
  /** \brief Запрос существования таблицы */
  void execIsTableExists(const std::stringstream &sstr, bool *is_exists);
  /** \brief Запрос существования таблицы */
  void execColumnNamesString(const std::stringstream &sstr,
      std::vector<std::string> *column_names);
  /** \brief Запрос добавления колонки в таблицу */
  void execAddColumn(const std::stringstream &sstr, void *);
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
  struct {
  public:
    /** \brief Указатель на объект подключения */
    std::unique_ptr<pqxx::connection> pconnect_;
    /** \brief Указатель на транзакцию */
    std::unique_ptr<pqxx::nontransaction> work_;

  public:
    /** \brief Инициализировать параметры соединения, транзакции */
    bool InitConnection(const std::string &connect_str) {
      pconnect_ = std::unique_ptr<pqxx::connection>(
          new pqxx::connection(connect_str));
      if (pconnect_)
        if (pconnect_->is_open())
          work_ = std::unique_ptr<pqxx::nontransaction>(
              new pqxx::nontransaction(*pconnect_));
      return IsAvailable();
    }
    /** \brief Проверить установки текущей транзаккции */
    bool IsAvailable() const {
      return work_ != nullptr;
    }
    /** \brief Указатель на транзакцию */
    pqxx::nontransaction *GetTransaction() const {
      return work_.get();
    }
  } pqxx_work;
  // add result data for select queries for example, or IsTableExist
};

#endif  // WITH_POSTGRESQL
#endif  // !_DATABASE__DB_CONNECTION_POSTGRESQL_H_
