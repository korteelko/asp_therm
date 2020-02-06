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
  DBQuery();
  void SetDB(DBConnection *db_ptr);

  virtual void Execute() = 0;
  virtual void unExecute() = 0;
  virtual ~DBQuery();

protected:
  std::string query_body_;
  DBConnection *db_ptr_;
};
typedef std::vector<std::unique_ptr<DBQuery>> query_ptr_container;

class DBQueryInitConnection: public DBQuery {
public:
  DBQueryInitConnection(const std::string &query);
  void Execute();
  void unExecute();
};

class DBQueryCreateTable: public DBQuery {
public:
  DBQueryCreateTable(const std::string &query);
  void Execute();
  void unExecute();
};

class DBQueryInsertStateLog: public DBQuery {
public:
  DBQueryInsertStateLog(const std::string &query);
  void Execute();
  void unExecute();
};

#endif  // !_CORE__SUBROUTINS__DB_QUERY_H_
