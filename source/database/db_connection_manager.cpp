/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_connection_manager.h"

#include "db_connection_postgre.h"

#include <ctime>

#include <assert.h>


// db_parameters::db_parameters()
//   : supplier(db_client::NOONE) {}

Transaction::Transaction(DBConnection *connection)
  : status_(STATUS_DEFAULT), connection_(connection) {}

void Transaction::AddQuery(QuerySmartPtr &&query) {
  queries_.emplace_back(query);
}

mstatus_t Transaction::ExecuteQueries() {
  if (status_ == STATUS_DEFAULT) {
    for (auto it_query = queries_.begin(); it_query != queries_.end(); it_query++) {
      status_ = (*it_query)->Execute();
      // если статус после выполнения не удовлетворителен -
      //   откатим все изменения, залогируем ошибку
      if (!is_status_aval(status_)) {
        (*it_query)->LogError();
        auto ri = std::make_reverse_iterator(it_query);
        for (;ri != queries_.rend(); ++ri) {
          if ((*ri)->IsPerformed())
            // dev может и им статус поменять?
            (*ri)->unExecute();
        }
        break;
      }
    }
  }
  return status_;
}


DBConnectionManager &DBConnectionManager::Instance() {
  static DBConnectionManager db;
  return db;
}

mstatus_t DBConnectionManager::CheckConnection() {
  if (!(error_.GetErrorCode() && status_ == STATUS_HAVE_ERROR)) {
    if (!db_connection_)
      initDBConnection();
  }
  if (db_connection_ && is_status_aval(status_)) {
    Transaction tr(db_connection_.get());
    tr.AddQuery(QuerySmartPtr(
        new DBQuerySetupConnection(db_connection_.get())));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryCloseConnection(db_connection_.get())));
    status_ = tryExecuteTransaction(tr);
  } else {
    error_.SetError(ERROR_DB_CONNECTION, "Не удалось установить"
        "соединение для для БД: " + parameters_.GetInfo());
    status_ = STATUS_HAVE_ERROR;
  }
  return status_;
}

mstatus_t DBConnectionManager::ResetConnectionParameters(
    const db_parameters &parameters) {
  parameters_ = parameters;
  status_ = STATUS_DEFAULT;
  error_.Reset();
  return CheckConnection();
}

bool DBConnectionManager::IsTableExist(db_table dt) {
  bool exists = false;
  if (status_ == STATUS_DEFAULT)
    status_ = CheckConnection();
  if (db_connection_ && is_status_aval(status_)) {
    Transaction tr(db_connection_.get());
    tr.AddQuery(QuerySmartPtr(
        new DBQuerySetupConnection(db_connection_.get())));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryIsTableExist(db_connection_.get(), dt, exists)));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryCloseConnection(db_connection_.get())));
    status_ = tryExecuteTransaction(tr);
  } else {
    error_.SetError(ERROR_DB_CONNECTION, "Не удалось установить "
        "соединение для БД: " + parameters_.GetInfo());
    status_ = STATUS_HAVE_ERROR;
  }
  return exists;
}

mstatus_t DBConnectionManager::CreateTable(db_table dt) {
  if (status_ == STATUS_DEFAULT)
    status_ = CheckConnection();
  if (db_connection_ && is_status_aval(status_)) {
    Transaction tr(db_connection_.get());
    tr.AddQuery(QuerySmartPtr(
        new DBQuerySetupConnection(db_connection_.get())));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryCreateTable(db_connection_.get(),
        get_table_create_setup(dt))));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryCloseConnection(db_connection_.get())));
    status_ = tryExecuteTransaction(tr);
  } else {
    error_.SetError(ERROR_DB_CONNECTION, "Не удалось установить "
        "соединение для БД: " + parameters_.GetInfo());
    status_ = STATUS_HAVE_ERROR;
  }
  return status_;
}

merror_t DBConnectionManager::GetErrorCode() {
  return error_.GetErrorCode();
}

mstatus_t DBConnectionManager::GetStatus() {
  return status_;
}

std::string DBConnectionManager::GetErrorMessage() {
  return error_.GetMessage();
}

DBConnectionManager::DBConnectionManager() {}

void DBConnectionManager::initDBConnection() {
  db_connection_ = std::unique_ptr<DBConnection>(
      DBConnectionCreator().InitDBConnection(parameters_));
  if (!db_connection_) {
    status_ = STATUS_HAVE_ERROR;
    error_.SetError(ERROR_DB_CONNECTION,
        "Подключение к базе данных не инициализировано");
  }
}

mstatus_t DBConnectionManager::tryExecuteTransaction(Transaction &tr) {
  try {
    status_ = tr.ExecuteQueries();
  } catch (const std::exception &e) {
    error_.SetError(ERROR_DB_CONNECTION, "Во время попытки "
        "подключения к БД перехвачено исключение: " + std::string(e.what()));
    error_.LogIt();
    status_ = STATUS_HAVE_ERROR;
  }
  if (db_connection_) {
    if (db_connection_->IsOpen())
      db_connection_->CloseConnection();
    if (db_connection_->GetErrorCode())
      db_connection_->LogError();
  } else {
    if (!error_.GetErrorCode())
      error_.SetError(ERROR_DB_CONNECTION,
          "Подключение к базе данных не инициализировано");
    status_ = STATUS_HAVE_ERROR;
  }
  return status_;
}

/* DBConnection::DBConnectionInstance */
DBConnectionIns::DBConnectionCreator() {}

DBConnection *DBConnectionIns::InitDBConnection(
    const db_parameters &parameters) {
  DBConnection *connect = nullptr;
  switch (parameters.supplier) {
    case db_client::NOONE:
      break;
    #if defined(WITH_POSTGRESQL)
    case db_client::POSTGRESQL:
      connect = new DBConnectionPostgre(parameters);
      break;
    #endif  //
    // TODO: можно тут ошибку установить
    default:
      break;
  }
  return connect;
}
