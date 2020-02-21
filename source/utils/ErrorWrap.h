#ifndef _UTILS__ERRORWRAP_H_
#define _UTILS__ERRORWRAP_H_

#include "common.h"
#if defined(INCLUDE_ERRORCODES)
#  include "merror_codes.h"
#endif  // INCLUDE_ERRORCODES

#include <string>

#include <stdint.h>


#define ERROR_SUCCESS_T     0x00000000
#define ERROR_GENERAL_T     0x00000001

#define XML_LAST_STRING     0x1000


typedef size_t merror_t;

/// ГЛОБАЛЬНАЯ ОШИБКА, ЧТОБЫ ПОЛОЖИТЬ ПРОГРАММУ
// ERR_SUCCESS as default
merror_t set_error_code(merror_t err_code);
merror_t get_error_code();
// set err_code to SUCCESS and
//   forget setted error message
void reset_error();

/** replace custom errormsg with passed string
      return passed to function err_code
 */
merror_t set_error_message(merror_t err_code,
    const char *format, ...);
const char *get_error_message();
// my be 'char *get_custom_err_msg'


/* TODO: rename to ModelError */
/** \brief класс, в котором инкапсулирована ошибка(код, сообщение,
  *   логирована ли, выведелена ли и т.п.) */
class ErrorWrap {
public:
  ErrorWrap();
  ErrorWrap(merror_t error);
  ErrorWrap(merror_t error, const std::string &msg);
  /** \brief установить(хранить) код ошибки 'error'
    * \param error код ошибки
    * \param(optional) msg сопроводительное сообщение */
   merror_t SetError(merror_t error);
   merror_t SetError(merror_t error, const std::string &msg);
  /** \brief заменить сообщение об ошибке 'msg_' на 'new_msg'
    * \param new_msg новое сообщение(инфо) об ошибке */
  void ChangeMessage(const std::string &new_msg);
  /** \brief залогировать текущее состояние(если есть ошибка)
    *   установить 'is_logged_' в true
    * \param lvl(optional) особый логлевел для данного сообщения
    * \note здесь и везде Log* методы не константные,
    *   т.к. в них отслеживается состояние */
  void LogIt();
  void LogIt(io_loglvl lvl);

  merror_t GetErrorCode() const;
  std::string GetMessage() const;
  bool IsLogged() const;

  /** \brief скинуть все параметры объекта ErrorWrap в значения по умолчанию:
    *   error_ to ERR_SUCCESS_T
    *   msg_ to ""
    *   is_logged_ to false */
  void Reset();

  /* DEVELOP: такая перегрузка может стать причиной ментального бо-бо */
  ErrorWrap &operator= (merror_t) = delete;

private:
  /** \brief код ошибки(см дефайны выше) */
  merror_t error_;
  /** \brief сообщение к ошибке */
  std::string msg_;
  /** \brief переменная отслеживающая выводилось ли уже
    *   информация об этой ошибке */
  bool is_logged_;
};

#endif  // !_UTILS__ERRORWRAP_H_
