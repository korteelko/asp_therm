#ifndef _DATABASE__DB_QUERY_H_
#define _DATABASE__DB_QUERY_H_

#include "db_defines.h"
#include "gas_description.h"
#include "models_errors.h"

#include <memory>
#include <string>
#include <vector>

/* !NOTE try pattern Command for it */
/* а не усложню ли я этим архитектуру,
 *   в принципе, не творю ли я фигню.
 * хотя здесь всё выглядит логично */

class DBConnection;
/** \brief класс запросов, сделаю пока так может
  *  потом понятнее будет как лучше */
class DBQuery {
public:
  void SetDB(DBConnection *db_ptr);
  bool IsPerformed() const;

  void LogError();

  virtual mstatus_t Execute() = 0;
  virtual void unExecute() = 0;
  virtual ~DBQuery();

protected:
  DBQuery(DBConnection *db_ptr);
  DBQuery();

protected:
  // std::string query_body_;
  mstatus_t status_;
  DBConnection *db_ptr_;
  bool is_performed_;
};
typedef std::shared_ptr<DBQuery> QuerySmartPtr;
typedef std::vector<QuerySmartPtr> QueryContainer;

class DBQuerySetupConnection: public DBQuery {
public:
  DBQuerySetupConnection();
  DBQuerySetupConnection(DBConnection *db_ptr);
  mstatus_t Execute() override;
  void unExecute() override;
};

class DBQueryCloseConnection: public DBQuery {
public:
  DBQueryCloseConnection();
  DBQueryCloseConnection(DBConnection *db_ptr);
  mstatus_t Execute() override;
  void unExecute() override;
};

class DBQueryIsTableExist: public DBQuery {
public:
  DBQueryIsTableExist(db_table dt, bool &is_exists);
  DBQueryIsTableExist(DBConnection *db_ptr,
      db_table dt, bool &is_exists);
  mstatus_t Execute() override;
  void unExecute() override;

private:
  db_table table_;
  bool &is_exists_;
};



class DBQueryCreateTable: public DBQuery {
public:
  DBQueryCreateTable(const std::string &query);
  DBQueryCreateTable(
      DBConnection *db_ptr, const std::string &query);
  mstatus_t Execute() override;
  void unExecute() override;
};

class DBQueryUpdateTable: public DBQuery {
public:
  DBQueryUpdateTable(const std::string &query);
  DBQueryUpdateTable(
      DBConnection *db_ptr, const std::string &query);
  mstatus_t Execute() override;
  void unExecute() override;
};

class DBQueryInsertModelInfo: public DBQuery {
public:
  DBQueryInsertModelInfo(const std::string &query);
  DBQueryInsertModelInfo(
      DBConnection *db_ptr, const std::string &query);
  mstatus_t Execute() override;
  void unExecute() override;
};

class DBQueryInsertCalculationInfo: public DBQuery {
public:
  DBQueryInsertCalculationInfo(const std::string &query);
  DBQueryInsertCalculationInfo(
      DBConnection *db_ptr, const std::string &query);
  mstatus_t Execute() override;
  void unExecute() override;
};

class DBQueryInsertCalculationStateLog: public DBQuery {
public:
  DBQueryInsertCalculationStateLog(const std::string &query);
  DBQueryInsertCalculationStateLog(
      DBConnection *db_ptr, const std::string &query);
  mstatus_t Execute() override;
  void unExecute() override;
};

#endif  // !_DATABASE__DB_QUERY_H_
