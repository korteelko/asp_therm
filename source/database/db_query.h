/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_QUERY_H_
#define _DATABASE__DB_QUERY_H_

#include "db_defines.h"
#include "gas_description.h"
#include "ErrorWrap.h"

#include <memory>
#include <string>
#include <vector>


class DBConnection;
struct db_table_create_setup;

/** \brief абстрактный класс запросов */
class DBQuery {
public:
  bool IsPerformed() const {return is_performed_;}

  void LogError();

  virtual mstatus_t Execute() = 0;
  virtual void unExecute() = 0;
  virtual ~DBQuery();

protected:
  DBQuery(DBConnection *db_ptr);

protected:
  // std::string query_body_;
  mstatus_t status_;
  DBConnection *db_ptr_;
  bool is_performed_;
};
typedef std::shared_ptr<DBQuery> QuerySmartPtr;
typedef std::vector<QuerySmartPtr> QueryContainer;

/** \brief Запрос установить соединение с бд */
class DBQuerySetupConnection: public DBQuery {
public:
  DBQuerySetupConnection(DBConnection *db_ptr);
  mstatus_t Execute() override;
  /** \brief отключиться от бд */
  void unExecute() override;
};

/** \brief Запрос отключения от бд */
class DBQueryCloseConnection: public DBQuery {
public:
  DBQueryCloseConnection(DBConnection *db_ptr);
  mstatus_t Execute() override;
  /** \brief не делать ничего */
  void unExecute() override;
};

/** \brief Запрос проверки существования таблицы в бд */
class DBQueryIsTableExist: public DBQuery {
public:
  DBQueryIsTableExist(DBConnection *db_ptr, db_table dt, bool &is_exists);
  mstatus_t Execute() override;
  /** \brief не делать ничего */
  void unExecute() override;

private:
  db_table table_;
  bool &is_exists_;
};

/** \brief Запрос создания таблицы в бд */
class DBQueryCreateTable: public DBQuery {
public:
  DBQueryCreateTable(DBConnection *db_ptr, const db_table_create_setup &create_setup);
  mstatus_t Execute() override;
  /** \brief обычный rollback создания таблицы */
  void unExecute() override;

private:
  const db_table_create_setup &create_setup;
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
