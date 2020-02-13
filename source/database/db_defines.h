#ifndef _CORE__SUBROUTINS__DB_DEFINES_H_
#define _CORE__SUBROUTINS__DB_DEFINES_H_

#include <string>

#include <stdint.h>


/*
 * Чё-та я перепутал названия 'query' и 'transaction' ( ͡° ͜ʖ ͡°)
 *   и мне нравится отображение смайла ( ͡° ͜ʖ ͡°) в qtCreator'e
*/
/** \brief клиент БД */
enum class db_client: uint32_t {
  NOONE = 0,
  /// реализация в db_connection_postgre.cpp
  POSTGRESQL = 1
};

std::string db_client_to_string(db_client client);

/** \brief перечисление типов используемых таблиц */
enum class db_table {
  /** информация о ревизии уравнения состояния */
  table_model_info = 0,
  /* TODO: добавить структуру с++, содержащую
   *   данные для этой таблицы*/
  /** информация о расчёте */
  table_calculation_info = 1, // reference to 'table_model_info '
  /** лог расчёта */
  table_calculation_state_log = 2  // foreign key to 'table_calculation_info '
};

std::string get_table_name(db_table dt);

/* если запрос как набор типизированных строк
 *   то необходимо определить строки неформата.
 *   например запрос "создать таблицу" может содержать
 *   комплексный первичный ключ, прописываемый в отд. строке */
enum class db_query_str {
  query_str_variable = 0,
  query_str_complex_pk,
  query_str_complex_fk
};

#endif  // !_CORE__SUBROUTINS__DB_DEFINES_H_