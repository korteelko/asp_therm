#ifndef _CORE__SUBROUTINS__DB_QUERY_H_
#define _CORE__SUBROUTINS__DB_QUERY_H_

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
  inline void SetDB(DBConnection *db_ptr) {
    db_ptr_ = db_ptr;
  }
  inline std::string GetQueryBody() const {
    return query_body_;
  }

  virtual mstatus_t Execute() = 0;
  virtual void unExecute() = 0;
  virtual ~DBQuery();

protected:
  DBQuery(DBConnection *db_ptr, const std::string &query);
  DBQuery(const std::string &query);
  DBQuery();

protected:
  std::string query_body_;
  mstatus_t status_;
  DBConnection *db_ptr_;
};
typedef std::vector<std::unique_ptr<DBQuery>> QueryContainer;

class DBQueryCheckConnection: public DBQuery {
public:
  DBQueryCheckConnection(const std::string &query);
  DBQueryCheckConnection(
      DBConnection *db_ptr, const std::string &query);
  mstatus_t Execute() override;
  void unExecute() override;
};

class DBQueryIsTableExist: public DBQuery {
public:
  DBQueryIsTableExist(const std::string &query);
  DBQueryIsTableExist(
      DBConnection *db_ptr, const std::string &query);
  mstatus_t Execute() override;
  void unExecute() override;
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

#endif  // !_CORE__SUBROUTINS__DB_QUERY_H_
