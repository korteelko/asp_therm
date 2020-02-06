#ifndef _CORE__SUBROUTINS__DB_CONNECTION_H_
#define _CORE__SUBROUTINS__DB_CONNECTION_H_

#include "common.h"
#include "db_query.h"
#include "models_errors.h"

#include <string>
#include <vector>

/** \brief клиент БД */
enum class db_client: uint32_t {
  NOONE = 0,
  POSTGRESQL = 1
};

/** \brief перечисление типов используемых таблиц */
enum class db_table {
  table_model_info = 0,
  table_state_log = 1
};

/** \brief структура содержит параметры коннектинга */
struct db_parameters {
public:
  /** \brief тип клиента */
  db_client supplier;
  /** \brief параметры подключения базы данных
    * \note сделаю как в джанго */
  std::string name,
              username,
              password,
              host;
  int port;

public:
  db_parameters();

  merror_t SetConfigurationParameter(const std::string &param_strtpl,
      const std::string &param_value);
};

struct state_log;

/** \brief абстрактный класс подключения к БД */
class DBConnection {
public:
  virtual merror_t InitConnection(const db_parameters &parameters) = 0;
  virtual void Commit() = 0;

  virtual void CreateTable(db_table t) = 0;
  virtual void InsertStateLog(const state_log &sl) = 0;
  virtual void UpdateStateLog(const state_log &sl) = 0;
  // virtual void DeleteStateLog(const state_log &sl) = 0;

  virtual ~DBConnection();

protected:
  DBConnection();

protected:
  ErrorWrap error_;
  mstatus_t status_;
  db_parameters parameters_;
  query_ptr_container queries_;
};

/** \brief реализация DBConnection для postgre */
class DBConnectionPostgre final: public DBConnection {
public:
  DBConnectionPostgre();
  merror_t InitConnection(const db_parameters &parameters) override;
  void Commit() override;

  void CreateTable(db_table t) override;
  void InsertStateLog(const state_log &sl) override;
  void UpdateStateLog(const state_log &sl) override;
  // void DeleteStateLog(const state_log &sl) override;
  // std::vector<state_log> SelectStateLog() override;

  ~DBConnectionPostgre() override;

private:
  std::string setupConnectionString();
};

#endif  // !_CORE__SUBROUTINS__DB_CONNECTION_H_
