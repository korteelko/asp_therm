#ifndef _CORE__COMMON__MODELS_ERRORS_H_
#define _CORE__COMMON__MODELS_ERRORS_H_

#include "common.h"

#include <string>

#include <stdint.h>

/** swap uint32 to uint64 for simple padding
  *   merror_t ussualy first field of struct/class
  */
typedef uint64_t merror_t;

#define ERR_MSG_MAX_LEN     200

// error type
#define ERR_MASK_TYPE       0x000f

#define ERR_SUCCESS_T       0x0000
/** ошибка файлового ввода/вывода */
#define ERR_FILEIO_T        0x0001
/** ошибка при проведении расчётов */
#define ERR_CALCULATE_T     0x0002
/** ошибка при работе со строками */
#define ERR_STRING_T        0x0003
/** ошибка инициализации */
#define ERR_INIT_T          0x0004
/** ошибка работы со стоковыми шаблонами.
  *   строковые шаблоны - текстовые значения в xml и json файлах */
#define ERR_STRTPL_T        0x0005

// type gas / gas_mix
#define ERR_MASK_GAS_MIX    0x00f0
// посути просто маркер
//   смеси газов
// #define ERR_GAS_PURE      0x0000
// assert UDOLI
#define ERR_GAS_MIX         0x0010

// error subtype
#define ERR_MASK_SUBTYPE    0x0f00
// fileio errors
#define ERR_FILE_IN_ST      0x0100
#define ERR_FILE_OUT_ST     0x0200
#define ERR_FILE_LOGGING    0x0300

// calculate errors
#define ERR_CALC_GAS_P_ST   0x0100
#define ERR_CALC_PHASE_ST   0x0200
#define ERR_CALC_MODEL_ST   0x0300
#define ERR_CALC_MIX_ST     0x0400

// string errors
#define ERR_STR_MAX_LEN_ST  0x0100
#define ERR_STR_PARSE_ST    0x0200
#define ERR_STR_NULL_ST     0x0300
#define ERR_STR_TOINT_ST    0x0400

// init errors
#define ERR_INIT_ZERO_ST    (0x0100 | ERR_INIT_T)
#define ERR_INIT_NULLP_ST   (0x0200 | ERR_INIT_T)

// string templates error
/** пустой текстовый шаблон */
#define ERR_STRTPL_TPLNULL  (0x0100 | ERR_STRTPL_T)
/** неизвестный текстовый шаблон */
#define ERR_STRTPL_TPLUNDEF (0x0200 | ERR_STRTPL_T)
/** недопустимое значение текстового шаблона */
#define ERR_STRTPL_VALWRONG (0x0300 | ERR_STRTPL_T)

// info type mask
// #define ERR_MASK_TYPE      0xf000
// not error, but here
#define XML_LAST_STRING    0x1000


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
    *   устанавить 'is_logged_' в true
    * \param lvl(optional) особый лог левел для данного сообщения */
  void LogIt();
  void LogIt(io_loglvl lvl);

  merror_t GetErrorCode() const;
  std::string GetMessage() const;
  bool IsLogged() const;

  /* DEVELOP: такая перегрузка может стать причиной ментального бо-бо */
  ErrorWrap &operator= (merror_t) = delete;

private:
  merror_t error_;
  std::string msg_;
  bool is_logged_;
};

#endif  // !_CORE__COMMON__MODELS_ERRORS_H_
