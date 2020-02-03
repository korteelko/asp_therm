#ifndef _CORE__SUBROUTINS__DB_CONNECTION_H_
#define _CORE__SUBROUTINS__DB_CONNECTION_H_

#include "models_errors.h"

#include <string>
#include <vector>

#include <stdint.h>

/** \brief клиент БД */
enum class db_client: uint32_t {
  NOONE = 0,
  POSTGRESQL = 1
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

/** \brief класс взаимодействия с БД */
class DBConnection {
public:
  // struct TransactionBody {};
  class DBConnectionInstance {};

public:
  static DBConnection &Instance();
  /** \brief Попробовать законектится к БД */
  static const std::vector<std::string> &GetJSONKeys();
  static bool ResetConnect(const db_parameters &parameters);
  static bool IsConnected();

  static merror_t GetError();
  static std::string GetErrorMessage();

private:
  DBConnection();

private:
  static merror_t error_;
  static std::string error_msg_;
  static db_parameters parameters_;
  static bool is_connected_;
};

#endif  // _CORE__SUBROUTINS__DB_CONNECTION_H_
