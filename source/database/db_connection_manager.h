/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 * * db_connection_manager *
 *   В файле реализован API доступа к базе данных для
 * проекта asp_therm
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_CONNECTION_MANAGER_H_
#define _DATABASE__DB_CONNECTION_MANAGER_H_

#include "atherm_common.h"
#include "db_connection.h"
#include "db_query.h"
#include "db_tables.h"
#include "ErrorWrap.h"
#include "ThreadWrap.h"

#include <functional>
#include <string>
#include <vector>

#include <stdint.h>


/* todo: вынести db_connection_manager вне модуля 'database' */
/** \brief Класс инкапсулирующий конечную высокоуровневую операцию с БД
  * \note Определения 'Query' и 'Transaction' в программе условны:
  *   Query - примит обращения к БД, Transaction - связный набор примитивов
  * \todo Переместить класс в другой файл */
class Transaction {
public:
  class TransactionInfo;

public:
  Transaction(DBConnection *connection);
  // хм, прикольно
  // Transaction (std::mutex &owner_mutex);
  void AddQuery(QuerySmartPtr &&query);
  mstatus_t ExecuteQueries();
  mstatus_t CancelTransaction();

  ErrorWrap GetError() const;
  mstatus_t GetResult() const;
  TransactionInfo GetInfo() const;

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief Указатель на подключение по которому
    *   будет осуществлена транзакция */
  DBConnection *connection_;
  /** \brief Очередь простых запросов, составляющих полную транзакцию */
  QueryContainer queries_;
};

/** \brief Класс инкапсулирующий информацию о транзакции - лог, результат
  * \note Не доделан */
class Transaction::TransactionInfo {
  friend class Transaction;
// date and time +
// info
public:
  std::string GetInfo();

private:
  TransactionInfo();

private:
  std::string info_;
};


/** \brief Класс взаимодействия с БД, предоставляет API
  *   на все допустимые операции */
class DBConnectionManager {
public:
  DBConnectionManager();
  // API DB
  mstatus_t CheckConnection();
  // static const std::vector<std::string> &GetJSONKeys();
  /** \brief Попробовать законектится к БД */
  mstatus_t ResetConnectionParameters(const db_parameters &parameters);
  /** \brief Проверка существования таблицы */
  bool IsTableExist(db_table dt);
  /** \brief Создать таблицу */
  mstatus_t CreateTable(db_table dt);

  /* insert operations */
  /** \brief Сохранить в БД строку model_info */
  mstatus_t SaveModelInfo(model_info &mi);
  /** \brief Сохранить в БД строку calculation_info */
  mstatus_t SaveCalculationInfo(calculation_info &ci);
  /** \brief Сохранить в БД строку calculation_info */
  mstatus_t SaveCalculationStateLog(std::vector<calculation_state_log> &csi);

  /* select operations */
  /** \brief Вытащить из БД строки model_info по 'where' условиям */
  mstatus_t SelectModelInfo(model_info &where,
      std::vector<model_info> *res);
  /** \brief Вытащить из БД строки calculation_info по 'where' условиям */
  mstatus_t SelectCalculationInfo(calculation_info &where,
      std::vector<calculation_info> *res);
  /** \brief Вытащить из БД строки calculation_state_log по 'where' условиям */
  mstatus_t SelectCalculationStateLog(calculation_state_log &where,
      std::vector<calculation_state_log> *res);

  /* table format */
  /** \brief Проверить формат таблицы */
  bool CheckTableFormat(db_table dt);
  /** \brief Обновить формат таблицы */
  mstatus_t UpdateTableFormat(db_table dt);

  /** \brief Удалить строки */
  mstatus_t DeleteModelInfo(model_info &where);

  /* rename method to GetError */
  merror_t GetErrorCode();
  mstatus_t GetStatus();
  /* remove this(GetErrorMessage), add LogIt */
  std::string GetErrorMessage();

private:
  class DBConnectionCreator;

private:
  /** \brief Обёртка над функционалом сбора и выполнения транзакции:
    *   подключение, создание точки сохранения
    * \param DataT data входные данные
    * \param OutT res указатель на выходные данные
    * \param SetupQueryF setup_m метод на добавление
    *   специализированного запроса(Query) к транзакции
    * \param sp_ptr указатель на сетап точки сохранения
    * \todo Слишком много шаблонных параметров получается,
    *   не очень красиво смотрится */
  template <class DataT, class OutT, class SetupQueryF>
  mstatus_t exec_wrap(DataT data, OutT *res, SetupQueryF setup_m,
      db_save_point *sp_ptr) {
    if (status_ == STATUS_DEFAULT)
      status_ = CheckConnection();
    mstatus_t trans_st;
    if (db_connection_ && is_status_aval(status_)) {
      Transaction tr(db_connection_.get());
      tr.AddQuery(QuerySmartPtr(
          new DBQuerySetupConnection(db_connection_.get())));
      // добавить точку сохранения, если есть необходимость
      if (sp_ptr)
        tr.AddQuery(QuerySmartPtr(
            new DBQueryAddSavePoint(db_connection_.get(), *sp_ptr)));
      // добавить специализированные запросы
      std::invoke(setup_m, *this, &tr, data, res);
      tr.AddQuery(QuerySmartPtr(
          new DBQueryCloseConnection(db_connection_.get())));
      trans_st = tryExecuteTransaction(tr);
    } else {
      error_.SetError(ERROR_DB_CONNECTION, "Не удалось установить "
          "соединение для БД: " + parameters_.GetInfo());
      status_ = trans_st = STATUS_HAVE_ERROR;
    }
    return trans_st;
  }

  /** \brief Проинициализировать соединение с БД */
  void initDBConnection();

  /** \brief Собрать запрос на выборку данных */
  template <class DataT>
  mstatus_t selectData(db_table t, DataT &where, std::vector<DataT> *res) {
    std::unique_ptr<db_query_select_setup> dss(db_query_select_setup::Init(t));
    if (dss)
      dss->where_condition.reset(InitWhereTree(where));
    db_query_select_result result(*dss);
    auto st = exec_wrap<const db_query_select_setup &, db_query_select_result,
        void (DBConnectionManager::*)(Transaction *, const db_query_select_setup &,
        db_query_select_result *)>(*dss, &result, &DBConnectionManager::selectRows,
        nullptr);
    if (st == STATUS_OK)
      SetSelectData(&result, res);
    return st;
  }

  /* добавить в транзакцию соответствующий запрос */
  /** \brief Запрос сушествования таблицы */
  void isTableExist(Transaction *tr, db_table dt, bool *is_exists);
  /** \brief Запрос создания таблицы */
  void createTable(Transaction *tr, db_table dt, void *);
  /** \brief Запрос получения формата таблицы */
  void getTableFormat(Transaction *tr, db_table dt,
      db_table_create_setup *exist_table);
  /** \brief Запрос сохранения строки данных */
  void saveRows(Transaction *tr, const db_query_insert_setup &qi,
      id_container *id_vec);
  /** \brief Запрос выборки параметров */
  void selectRows(Transaction *tr, const db_query_select_setup &qs,
      db_query_select_result *result);
  /** \brief Запрос на удаление рядов */
  void deleteRows(Transaction *tr, const db_query_delete_setup &qd, void *);

  /** \brief провести транзакцию tr из собраных запросов(строк) */
  [[nodiscard]]
  mstatus_t tryExecuteTransaction(Transaction &tr);

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief Мьютекс на подключение к БД */
  SharedMutex connect_init_lock_;
  /** \brief Параметры текущего подключения к БД */
  db_parameters parameters_;
  /* todo: replace with connection pull */
  /** \brief Указатель иницианилизированное подключение */
  std::unique_ptr<DBConnection> db_connection_;
};


/** \brief Закрытый класс создания соединений с БД */
class DBConnectionManager::DBConnectionCreator {
  friend class DBConnectionManager;

private:
  DBConnectionCreator();

  DBConnection *InitDBConnection(const db_parameters &parameters);
};

#endif  // !_DATABASE__DB_CONNECTION_MANAGER_H_
