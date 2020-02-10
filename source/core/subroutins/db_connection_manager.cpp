#include "db_connection_manager.h"

#include <ctime>

#include <assert.h>


// db_parameters::db_parameters()
//   : supplier(db_client::NOONE) {}

Transaction::Transaction(DBConnection *connection)
  : status_(STATUS_DEFAULT), connection_(connection) {}

void Transaction::AddQuery(std::unique_ptr<DBQuery> &&query) {
  queries_.emplace_back(query);
  queries_.back()->SetDB(connection_);
}

mstatus_t Transaction::ExecuteQueries() {
  if (status_ == STATUS_DEFAULT) {
    for (std::unique_ptr<DBQuery> &query: queries_) {
      status_ = query->Execute();
      if (status_ != STATUS_OK && status_ != STATUS_DRY_RUN) {
        // have error

      }
      assert(0);
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
  if (db_connection_) {
    status_ = STATUS_DEFAULT;
    Transaction tr(db_connection_.get());
    tr.AddQuery(std::unique_ptr<DBQuery>(
        new DBQuerySetupConnection()));
    tr.AddQuery(std::unique_ptr<DBQuery>(
        new DBQueryCloseConnection()));
    try {
      status_ = tr.ExecuteQueries();
    } catch (const std::exception &e) {
      error_.SetError(ERR_DB_CONNECTION, e.what());
      error_.LogIt();
      status_ = STATUS_HAVE_ERROR;
    }
  } else {
    assert(0 && "ebobo vsyo propalo");
  }
  return status_;
}

mstatus_t DBConnectionManager::ResetConnectionParameters(
    const db_parameters &parameters) {}

merror_t DBConnectionManager::GetErrorCode() {
  return error_.GetErrorCode();
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

/* DBConnection::DBConnectionInstance */
DBConnectionIns::DBConnectionCreator() {}
