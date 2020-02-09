#include "db_query.h"

#include "db_connection.h"

DBQuery::DBQuery(DBConnection *db_ptr, const std::string &query)
  : query_body_(query), status_(STATUS_DEFAULT), db_ptr_(db_ptr) {}

DBQuery::DBQuery(const std::string &query)
  : DBQuery(nullptr, query) {}

DBQuery::DBQuery()
  : DBQuery(nullptr, "") {}

DBQuery::~DBQuery() {}

DBQueryCheckConnection::DBQueryCheckConnection(
    const std::string &query)
  : DBQuery(query) {}

DBQueryCheckConnection::DBQueryCheckConnection(
    DBConnection *db_ptr, const std::string &query)
  : DBQuery(db_ptr, query) {}

mstatus_t DBQueryCheckConnection::Execute() {
  if (db_ptr_ && status_ == STATUS_DEFAULT)
    status_ = db_ptr_->ExecuteQuery(query_body_);
  return status_;
}

void DBQueryCheckConnection::unExecute() {

}
