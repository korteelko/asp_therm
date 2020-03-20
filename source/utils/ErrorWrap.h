/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef UTILS__ERRORWRAP_H
#define UTILS__ERRORWRAP_H

#include "common.h"
#if defined(INCLUDE_ERRORCODES)
#  include "merror_codes.h"
#endif  // INCLUDE_ERRORCODES

#include <string>

#include <stdint.h>

#define ERROR_SUCCESS_T    0x00000000
#define ERROR_GENERAL_T    0x00000001

/* just for lulz */
#if defined(_DEBUG)
#  define STRING_DEBUG_INFO ("file: " + std::string(__FILE__) + \
       "\n\tfunction: " + std::string(__FUNCTION__) \
       + " line: " + std::to_string(__LINE__) + "\n")
#  define APPEND_STREAM_DEBUG_INFO(sstm) (sstm << "file: " << __FILE__ \
       << "\n\tfunction: " << __FUNCTION__ << " line: " << __LINE__ << "\n")
#  define APPEND_STRING_DEBUG_INFO(str) (str += STRING_DEBUG_INFO)
#else
#  define STRING_DEBUG_INFO ""
#  define APPEND_STREAM_DEBUG_INFO(sstm) (sstream << "")
#  define APPEND_STRING_DEBUG_INFO(str) (str += "")
#endif  // _DEBUG


typedef size_t merror_t;

/** \brief класс, в котором инкапсулирована ошибка(код, сообщение,
  *   логирована ли, выведелена ли и т.п.) */
class ErrorWrap {
public:
  ErrorWrap();
  explicit ErrorWrap(merror_t error);
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
    *   error_ to ERROR_SUCCESS_T
    *   msg_ to ""
    *   is_logged_ to false */
  void Reset();

  /* DEVELOP: такая перегрузка может стать причиной ментального бо-бо */
  ErrorWrap &operator= (merror_t) = delete;

private:
  /** \brief код ошибки(см merror_codes.h) */
  merror_t error_;
  /** \brief сообщение к ошибке */
  std::string msg_;
  /** \brief переменная отслеживающая выводилось ли уже
    *   информация об этой ошибке */
  bool is_logged_;
};

#endif  // !UTILS__ERRORWRAP_H
