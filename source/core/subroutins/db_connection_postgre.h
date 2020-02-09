#ifndef _CORE__SUBROUTINS__DB_CONNECTION_POSTGRESQL_H_
#define _CORE__SUBROUTINS__DB_CONNECTION_POSTGRESQL_H_

#include "common.h"
#include "db_connection.h"

#include <memory>
#include <string>

#include <pqxx/pqxx>


/** \brief реализация DBConnection для postgresql */
class DBConnectionPostgre final: public DBConnection {
public:
  DBConnectionPostgre();

  mstatus_t ExecuteQuery(const std::string &query_body) override;

  mstatus_t CheckConnection(const db_parameters &parameters) override;

  void Commit() override;
  void Rollback() override;

  bool IsTableExist(db_table t) override;
  void CreateTable(db_table t,
      const std::vector<create_table_variant> &components) override;
  void UpdateTable(db_table t,
      const std::vector<create_table_variant> &components) override;

  void InsertModelInfo(const model_info &mi) override;
  void SelectModelInfo(const std::vector<db_variable> &mip) override;

  void InsertCalculationInfo(const calculation_info &ci) override;
  void InsertCalculationStateLog(const calculation_state_log &sl) override;

  ~DBConnectionPostgre() override;

private:
  void setupConnection();
  void closeConnection();
  std::string setupConnectionString();

private:
  std::unique_ptr<pqxx::connection> pconnect_;
};

#endif  // !_CORE__SUBROUTINS__DB_CONNECTION_POSTGRESQL_H_
