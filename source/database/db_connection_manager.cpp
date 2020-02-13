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
  queries_.back()->SetDB(connection_);
}

mstatus_t Transaction::ExecuteQueries() {
  if (status_ == STATUS_DEFAULT) {
    for (auto &query: queries_) {
      status_ = query->Execute();
      // если статус после выполнения не удовлетворителен -
      //   откатим все изменения, залоггируем ошибку
      if (status_ != STATUS_OK && status_ != STATUS_DRY_RUN) {
        query->LogError();
        for (QueryContainer::reverse_iterator ri = queries_.rbegin(); ri != queries_.rend(); ++ri )
          if ((*ri)->IsPerformed())
            // dev может и им статус поменять?
            (*ri)->unExecute();
        return status_;
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
  if (db_connection_ && status_ != STATUS_HAVE_ERROR) {
    status_ = STATUS_DEFAULT;
    Transaction tr(db_connection_.get());
    tr.AddQuery(QuerySmartPtr(
        new DBQuerySetupConnection()));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryCloseConnection()));
    tryExecuteTransaction(tr);
  } else {
    error_.SetError(ERR_DB_CONNECTION, "Не удалось установить"
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
  if (db_connection_ && status_ != STATUS_HAVE_ERROR) {
    status_ = STATUS_DEFAULT;
    Transaction tr(db_connection_.get());
    tr.AddQuery(QuerySmartPtr(
        new DBQuerySetupConnection()));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryIsTableExist(dt, exists)));
    tr.AddQuery(QuerySmartPtr(
        new DBQueryCloseConnection()));
    tryExecuteTransaction(tr);
  } else {
    error_.SetError(ERR_DB_CONNECTION, "Не удалось установить"
        "соединение для для БД: " + parameters_.GetInfo());
    status_ = STATUS_HAVE_ERROR;
  }
  return exists;
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
    assert(0 && "ebobo vsyo propalo");
  }
}

void DBConnectionManager::tryExecuteTransaction(Transaction &tr) {
  try {
    status_ = tr.ExecuteQueries();
  } catch (const std::exception &e) {
    error_.SetError(ERR_DB_CONNECTION, "Во время попытки "
        "подключения к БД перехвачено исключение: " + std::string(e.what()));
    error_.LogIt();
    status_ = STATUS_HAVE_ERROR;
  }
  if (db_connection_)
    if (db_connection_->IsOpen())
      db_connection_->CloseConnection();
}

/* DBConnection::DBConnectionInstance */
DBConnectionIns::DBConnectionCreator() {}

DBConnection *DBConnectionIns::InitDBConnection(
    const db_parameters &parameters) {
  DBConnection *connect = nullptr;
  switch (parameters.supplier) {
    case db_client::NOONE:
      break;
    case db_client::POSTGRESQL:
      connect = new DBConnectionPostgre(parameters);
      break;
    // TODO: можно тут ошибку установить
  }
  return connect;
}
