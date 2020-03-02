/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "db_query.h"

#include "db_connection.h"

DBQuery::DBQuery(DBConnection *db_ptr)
  : status_(STATUS_DEFAULT), db_ptr_(db_ptr), is_performed_(false) {}

DBQuery::DBQuery()
  : DBQuery(nullptr) {}

DBQuery::~DBQuery() {}

void DBQuery::SetDB(DBConnection *db_ptr) {
  db_ptr_ = db_ptr;
}

bool DBQuery::IsPerformed() const {
  return is_performed_;
}

void DBQuery::LogError() {
  if (db_ptr_)
    db_ptr_->LogError();
}

DBQuerySetupConnection::DBQuerySetupConnection()
  : DBQuery() {}

DBQuerySetupConnection::DBQuerySetupConnection(
    DBConnection *db_ptr)
  : DBQuery(db_ptr) {}

mstatus_t DBQuerySetupConnection::Execute() {
  if (db_ptr_ && status_ == STATUS_DEFAULT)
    status_ = db_ptr_->SetupConnection();
  is_performed_ = true;
  return status_;
}

void DBQuerySetupConnection::unExecute() {
  if (db_ptr_)
    db_ptr_->CloseConnection();
}

DBQueryCloseConnection::DBQueryCloseConnection()
  : DBQuery() {}

DBQueryCloseConnection::DBQueryCloseConnection(
    DBConnection *db_ptr)
  : DBQuery(db_ptr) {}

mstatus_t DBQueryCloseConnection::Execute() {
  if (db_ptr_ && status_ == STATUS_DEFAULT)
    db_ptr_->CloseConnection();
  status_ = STATUS_OK;
  is_performed_ = true;
  return status_;
}

// может снова коннектиться
void DBQueryCloseConnection::unExecute() {}

DBQueryIsTableExist::DBQueryIsTableExist(db_table dt, bool &is_exists)
  : DBQueryIsTableExist(nullptr, dt, is_exists) {}

DBQueryIsTableExist::DBQueryIsTableExist(
    DBConnection *db_ptr, db_table dt, bool &is_exists)
  : DBQuery(db_ptr), table_(dt), is_exists_(is_exists) {}

mstatus_t DBQueryIsTableExist::Execute() {
  if (db_ptr_ && status_ == STATUS_DEFAULT)
    status_ = db_ptr_->IsTableExists(table_, &is_exists_);
  is_performed_ = true;
  return status_;
}

void DBQueryIsTableExist::unExecute() {}
