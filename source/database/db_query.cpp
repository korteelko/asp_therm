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
#include "models_configurations.h"
#include "program_state.h"
#include "Logging.h"

DBQuery::DBQuery(DBConnection *db_ptr)
  : status_(STATUS_DEFAULT), db_ptr_(db_ptr), is_performed_(false) {}

DBQuery::~DBQuery() {}

void DBQuery::LogError() {
  if (db_ptr_)
    db_ptr_->LogError();
}

mstatus_t DBQuery::Execute() {
  if (db_ptr_) {
    if (status_ == STATUS_DEFAULT) {
      status_ = exec();
      is_performed_ = true;
    } else {
      if (IS_DEBUG_MODE)
        Logging::Append("not default status of query '" + q_info() + "'");
    }
  } else {
    status_ = STATUS_HAVE_ERROR;
    Logging::Append(ERROR_DB_QUERY_NULLP, "Execute setup connection query");
  }
  return status_;
}

/* DBQuerySetupConnection */
DBQuerySetupConnection::DBQuerySetupConnection(
    DBConnection *db_ptr)
  : DBQuery(db_ptr) {}

void DBQuerySetupConnection::unExecute() {
  if (db_ptr_)
    db_ptr_->CloseConnection();
}

mstatus_t DBQuerySetupConnection::exec() {
  return db_ptr_->SetupConnection();
}

std::string DBQuerySetupConnection::q_info() {
  return "SetupConnection";
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

mstatus_t DBQueryCloseConnection::exec() {
  // метод Execute перегружен
  return STATUS_NOT;
}

std::string DBQueryCloseConnection::q_info() {
  return "CloseConnection";
}

/* DBQueryIsTableExist */
DBQueryIsTableExists::DBQueryIsTableExists(
    DBConnection *db_ptr, db_table dt, bool &is_exists)
  : DBQuery(db_ptr), table_(dt), is_exists_(is_exists) {}

void DBQueryIsTableExists::unExecute() {}

mstatus_t DBQueryIsTableExists::exec() {
  return db_ptr_->IsTableExists(table_, &is_exists_);
}

std::string DBQueryIsTableExists::q_info() {
  return "IsTableExists";
}

/* DBQueryCreateTable */
DBQueryCreateTable::DBQueryCreateTable(DBConnection *db_ptr,
    const db_table_create_setup &setup)
 : DBQuery(db_ptr), create_setup(setup) {}

mstatus_t DBQueryCreateTable::exec() {
  return db_ptr_->CreateTable(create_setup);
}

std::string DBQueryCreateTable::q_info() {
  return "CreateTable";
}

/** \brief обычный rollback создания таблицы */
void DBQueryCreateTable::unExecute() {
  if (db_ptr_)
    db_ptr_->Rollback();
}

/* DBQueryUpdateTable */

/* DBQueryInsert */
DBQueryInsertRows::DBQueryInsertRows(DBConnection *db_ptr,
    const db_query_insert_setup &insert_setup)
  : DBQuery(db_ptr), insert_setup(insert_setup) {}

void DBQueryInsertRows::unExecute() {}

mstatus_t DBQueryInsertRows::exec() {
  return db_ptr_->InsertRows(insert_setup);
}

std::string DBQueryInsertRows::q_info() {
  return "InsertRows";
}
