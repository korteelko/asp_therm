/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_CONNECTION_MANAGER_H_
#define _DATABASE__DB_CONNECTION_MANAGER_H_

#include "db_connection.h"
#include "ErrorWrap.h"
#include "ThreadWrap.h"

#include <string>
#include <vector>

#include <stdint.h>


/** \brief Класс инкапсулирующий конечную высокоуровневую операцию с БД
  * \note Определения 'Query' и 'Transaction' в программе условны:
  *   Query - примит обращения к БД, Transaction - связный набор примитивов */
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
  mstatus_t ResetConnectionParameters(
      const db_parameters &parameters);
  bool IsTableExist(db_table dt);
  mstatus_t CreateTable(db_table dt);
  /* todo: select, update methods */

  /* rename method to GetError */
  merror_t GetErrorCode();
  mstatus_t GetStatus();
  /* remove this(GetErrorMessage), add LogIt */
  std::string GetErrorMessage();

private:
  class DBConnectionCreator;

private:
  void initDBConnection();
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
