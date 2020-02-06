#ifndef _CORE__SUBROUTINS__DB_CONNECTION_MANAGER_H_
#define _CORE__SUBROUTINS__DB_CONNECTION_MANAGER_H_

#include "db_connection.h"
#include "models_errors.h"

#include <string>
#include <vector>

#include <stdint.h>

/* TODO: add multithread and guards
 *   UPD: see gitlab issues */
/** \brief класс взаимодействия с БД */
class DBConnectionManager {
public:
  // struct TransactionBody {};
  class DBConnectionCreator;

public:
  static DBConnectionManager &Instance();
  /** \brief Попробовать законектится к БД */
  // static const std::vector<std::string> &GetJSONKeys();
  static bool ResetConnect(const db_parameters &parameters);
  static bool IsConnected();

  static merror_t GetErrorCode();
  static std::string GetErrorMessage();

private:
  DBConnectionManager();

private:
  static ErrorWrap error_;
  static db_parameters parameters_;
  static bool is_connected_;
};

class DBConnectionManager::DBConnectionCreator {
  DBConnectionCreator();
};
using DBConnectionIns = DBConnectionManager::DBConnectionCreator;

#endif  // !_CORE__SUBROUTINS__DB_CONNECTION_MANAGER_H_
