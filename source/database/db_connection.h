/**
 * asp_therm - implementation of real gas equations of state
 * ===================================================================
 *   Здесь прописан функционал инициализации подключения,
 * в том числе чтения файла конфигурации и API взаимодействия с БД.
 *   API юзерских операций прописан в файле DBConnectionManager
 * ===================================================================
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DATABASE__DB_CONNECTION_H_
#define _DATABASE__DB_CONNECTION_H_

#include "common.h"
#include "db_defines.h"
#include "db_queries_setup.h"
#include "db_query.h"
#include "ErrorWrap.h"

#include <string>
#include <vector>

#include <assert.h>


/** \brief структура содержит параметры коннектинга */
struct db_parameters {
public:
  /** \brief не подключаться к базе данных, просто
    * выводить получившееся запросы в stdout(или логировать) */
  bool is_dry_run;
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
  std::string GetInfo() const;
};


struct model_info;
struct calculation_info;
struct calculation_state_info;

/* develop: может ещё интерфейс сделать, а потом
 *   абстрактный класс */
 /* TODO: rename class */
/** \brief абстрактный класс подключения к БД */
class DBConnection {
public:
  /* хз... */
  // virtual mstatus_t ExecuteQuery(const std::string &query_body) = 0;

  virtual void Commit() = 0;
  virtual void Rollback() = 0;

  /* todo: вынести это всё в соответствующий класс
   *   здесь должны быть только низкоуровневые функции
   *   Update, Insert, Select */
  /* вызывается командами! */
  virtual mstatus_t CreateTable(const db_table_create_setup &fields) = 0;
  //virtual mstatus_t CheckTableFormat(const db_table_create_setup &fields) = 0;
  virtual void UpdateTable(db_table t, const db_table_update_setup &vals) = 0;

  // todo - replace argument 'const model_info &mi'
  //   with 'const db_table_select_setup &mip'
  // UPD: yes, looks wrong
  //   InsertModelInfo нужна в DBConnectionManager добавить
  // virtual void InsertModelInfo(const model_info &mi) = 0;
  virtual void InsertRow(const db_table_update_setup &insert_data) = 0;
  virtual void DeleteRow(const db_table_update_setup &delete_data) = 0;
  virtual void SelectModelInfo(const db_table_update_setup &select_data) = 0;

  virtual void InsertCalculationInfo(
      const calculation_info &ci) = 0;
  virtual void InsertCalculationStateLog(
      const calculation_state_info &sl) = 0;
  /* посюда */

  virtual mstatus_t SetupConnection() = 0;
  virtual void CloseConnection() = 0;

  virtual mstatus_t IsTableExists(db_table t, bool *is_exists) = 0;

  virtual ~DBConnection();

  mstatus_t GetStatus() const;
  merror_t GetErrorCode() const;
  bool IsOpen() const;
  /** \brief залогировать ошибку */
  void LogError();

protected:
  /* todo: add to parameters of constructor link to class
   *   DBConnectionCreator for closing all
   *   DBConnection inheritance classes */
  DBConnection(const db_parameters &parameters);

protected:
  ErrorWrap error_;
  /** \brief статус подключения */
  mstatus_t status_;
  /** \brief параметры подключения к базе данных */
  db_parameters parameters_;
  /** \brief флаг подключения к бд */
  bool is_connected_;
  /** \brief флаг работы без подключения к бд */
  bool is_dry_run_;
};

#endif  // !_DATABASE__DB_CONNECTION_H_
