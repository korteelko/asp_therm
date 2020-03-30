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

#include <string>
#include <vector>

#include <stdint.h>


/** \brief класс инкапсулирующий конечную
  *   высокоуровневую операцию с БД */
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
  DBConnection *connection_;
  QueryContainer queries_;
};

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


/** \brief класс взаимодействия с БД */
class DBConnectionManager {
private:
  class DBConnectionCreator;

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

  merror_t GetErrorCode();
  mstatus_t GetStatus();
  std::string GetErrorMessage();

private:
  void initDBConnection();
  /** \brief провести транзакцию tr из собраных запросов(строк) */
  [[nodiscard]]
  mstatus_t tryExecuteTransaction(Transaction &tr);

private:
  ErrorWrap error_;
  mstatus_t status_;
  db_parameters parameters_;
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
