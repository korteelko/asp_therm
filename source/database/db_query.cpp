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
#include "Logging.h"
#include "models_configurations.h"

DBQuery::DBQuery(DBConnection *db_ptr)
  : status_(STATUS_DEFAULT), db_ptr_(db_ptr), is_performed_(false) {}

DBQuery::~DBQuery() {}

void DBQuery::LogError() {
  if (db_ptr_)
    db_ptr_->LogError();
}

/* DBQuerySetupConnection */
DBQuerySetupConnection::DBQuerySetupConnection(
    DBConnection *db_ptr)
  : DBQuery(db_ptr) {}

mstatus_t DBQuerySetupConnection::Execute() {
  if (db_ptr_) {
    if (status_ == STATUS_DEFAULT) {
      status_ = db_ptr_->SetupConnection();
      is_performed_ = true;
    } else {
      if (ProgramState::Instance().IsDebugMode())
        Logging::Append("not default status of tableExist query");
    }
  } else {
    status_ = STATUS_HAVE_ERROR;
    Logging::Append(ERROR_DB_QUERY_NULLP, "Execute setup connection query");
  }
  return status_;
}

void DBQuerySetupConnection::unExecute() {
  if (db_ptr_)
    db_ptr_->CloseConnection();
}

/* DBQueryCloseConnection */
DBQueryCloseConnection::DBQueryCloseConnection(
    DBConnection *db_ptr)
  : DBQuery(db_ptr) {}

mstatus_t DBQueryCloseConnection::Execute() {
  if (db_ptr_) {
    db_ptr_->CloseConnection();
    status_ = STATUS_OK;
  } else {
    status_ = STATUS_HAVE_ERROR;
    Logging::Append(ERROR_DB_QUERY_NULLP, "Execute close connection query");
  }
  return status_;
}

void DBQueryCloseConnection::unExecute() {}

/* DBQueryIsTableExist */
DBQueryIsTableExist::DBQueryIsTableExist(
    DBConnection *db_ptr, db_table dt, bool &is_exists)
  : DBQuery(db_ptr), table_(dt), is_exists_(is_exists) {}

mstatus_t DBQueryIsTableExist::Execute() {
  if (db_ptr_) {
    if (status_ == STATUS_DEFAULT) {
      status_ = db_ptr_->IsTableExists(table_, &is_exists_);
      is_performed_ = true;
    } else {
      if (ProgramState::Instance().IsDebugMode())
        Logging::Append("not default status of tableExist query");
    }
  } else {
    status_ = STATUS_HAVE_ERROR;
    Logging::Append(ERROR_DB_QUERY_NULLP, "Execute table exist query");
  }
  return status_;
}

void DBQueryIsTableExist::unExecute() {}


/* DBQueryCreateTable */
DBQueryCreateTable::DBQueryCreateTable(DBConnection *db_ptr,
    const db_table_create_setup &setup)
 : DBQuery(db_ptr), create_setup(setup) {}

mstatus_t DBQueryCreateTable::Execute() {
  if (db_ptr_) {
    if (status_ == STATUS_DEFAULT) {
      status_ = db_ptr_->CreateTable(create_setup);
      is_performed_ = true;
    } else {
      if (ProgramState::Instance().IsDebugMode())
        Logging::Append("not default status of tableExist query");
    }
  } else {
    status_ = STATUS_HAVE_ERROR;
    Logging::Append(ERROR_DB_QUERY_NULLP, "Execute setup connection query");
  }
  return status_;
}

/** \brief обычный rollback создания таблицы */
void DBQueryCreateTable::unExecute() {
  if (db_ptr_)
    db_ptr_->Rollback();
}
