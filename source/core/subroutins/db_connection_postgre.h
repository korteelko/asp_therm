#ifndef _CORE__SUBROUTINS__DB_CONNECTION_POSTGRESQL_H_
#define _CORE__SUBROUTINS__DB_CONNECTION_POSTGRESQL_H_

#include "common.h"
#include "db_connection.h"

#include <memory>
#include <string>

#include <pqxx/pqxx>


// смотри страницу:
// https://www.tutorialspoint.com/postgresql/postgresql_c_cpp.htm
/** \brief реализация DBConnection для postgresql */
class DBConnectionPostgre final: public DBConnection {
public:
  DBConnectionPostgre(const db_parameters &parameters);
 // mstatus_t ExecuteQuery(DBQuery *query) override;

  void Commit() override;
  void Rollback() override;

  /* not checked */
  void CreateTable(db_table t,
      const std::vector<create_table_variant> &components) override;
  void UpdateTable(db_table t,
      const std::vector<create_table_variant> &components) override;

  void InsertModelInfo(const model_info &mi) override;
  void SelectModelInfo(const std::vector<db_variable> &mip) override;

  void InsertCalculationInfo(const calculation_info &ci) override;
  void InsertCalculationStateLog(const calculation_state_log &sl) override;


  /* checked functions */
  mstatus_t SetupConnection() override;
  void CloseConnection() override;

  mstatus_t IsTableExists(db_table t, bool *is_exists) override;

  ~DBConnectionPostgre() override;



private:
  std::string setupConnectionString();
  std::string db_variable_to_string(const db_variable &dv);

private:
  std::unique_ptr<pqxx::connection> pconnect_;
};

#endif  // !_CORE__SUBROUTINS__DB_CONNECTION_POSTGRESQL_H_
