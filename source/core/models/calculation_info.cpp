/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "calculation_info.h"

#include "model_general.h"

#include <array>


/* calculation_configuration */
bool calculation_configuration::IsDebug() const {
  return is_debug_mode;
}
bool calculation_configuration::RK_IsEnableOriginMod() const {
  return rk_enable_origin_mod;
}
bool calculation_configuration::RK_IsEnableSoaveMod() const {
  return rk_enable_soave_mod;
}
bool calculation_configuration::PR_IsEnableByBinaryCoefs() const {
  return pr_enable_by_binary_coefs;
}
bool calculation_configuration::IsEnableISO20765() const {
  return enable_iso_20765;
}

/* calculation_info */
calculation_info::calculation_info()
  : datetime(time(0)) {}

void calculation_info::SetDateTime(std::time_t *dt) {
  datetime = *dt;
  initialized |= (f_date | f_time);
}

/* формат 'yyyy/mm/dd'
 *   +1 за regex */
mstatus_t calculation_info::SetDate(const std::string &date) {
  mstatus_t st = STATUS_DEFAULT;
  /* размер разделителя для даты это '/' */
  int delim_size = 1;
  struct tm *as_tm;
  int date_arr[3] = {0, 0, 0};
  if (!date.empty()) {
    const char *d = date.c_str();
    char *end = nullptr;
    int i = 0;
    for (i = 0; i < 3; ++i) {
      date_arr[i] = std::strtod(d, &end);
      if (d != end) {
        d = end + delim_size;
      } else {
        break;
      }
    }
    if (i == 3) {
      /* обновить только дату */
      as_tm = localtime(&datetime);
      as_tm->tm_year = date_arr[0] - 1900;
      as_tm->tm_mon = date_arr[1] - 1;
      as_tm->tm_mday = date_arr[2];
      std::time_t res_time = mktime(as_tm);
      // проверить что при записи даты не слетели на такое как:
      //   32 января -> 1 февраля
      bool NotOverride = (as_tm->tm_year == date_arr[0] - 1900) &&
          (as_tm->tm_mon == date_arr[1] - 1) && (as_tm->tm_mday == date_arr[2]);
      if (res_time != -1 && NotOverride && (as_tm->tm_year < 200)) {
        initialized |= calculation_info::f_date;
        st = STATUS_OK;
        datetime = res_time;
      }
    }
  }
  return st;
}

/* формат 'hh:mm' */
mstatus_t calculation_info::SetTime(const std::string &time) {
  mstatus_t st = STATUS_DEFAULT;
  /* размер разделителя для времени это ':' */
  int delim_size = 1;
  struct tm *as_tm;
  int time_arr[2] = {0, 0};
  if (!time.empty()) {
    const char *t = time.c_str();
    char *end = nullptr;
    int i = 0;
    for (i = 0; i < 2; ++i) {
      time_arr[i] = std::strtod(t, &end);
      if (t != end) {
        t = end + delim_size;
      } else {
        break;
      }
    }
    /* можно без секунд */
    if (i == 2) {
      /* обновить только дату */
      as_tm = localtime(&datetime);
      as_tm->tm_hour = time_arr[0];
      as_tm->tm_min = time_arr[1];
      std::time_t res_time = mktime(as_tm);
      bool NotOverride = (as_tm->tm_hour == time_arr[0]) &&
          (as_tm->tm_min == time_arr[1]);
      if ((res_time != -1) && NotOverride) {
        initialized |= calculation_info::f_time;
        st = STATUS_OK;
        datetime = res_time;
      }
    }
  }
  return st;
}

/* формат 'yyyy/mm/dd' */
std::string calculation_info::GetDate() const {
  char d[16] = {0};
  std::tm *as_tm = std::localtime(&datetime);
  sprintf(d, "%04d/%02d/%02d", as_tm->tm_year + 1900,
      as_tm->tm_mon + 1, as_tm->tm_mday);
  return std::string(d);
}
/* формат 'hh:mm:ss' */
std::string calculation_info::GetTime() const {
  char t[16] = {0};
  std::tm *as_tm = std::localtime(&datetime);
  sprintf(t, "%02d:%02d", as_tm->tm_hour, as_tm->tm_min);
  return std::string(t);
}

CalculationSetup::CalculationSetup()
  : status_(STATUS_DEFAULT) {}

CalculationSetup::CalculationSetup(const calculation_setup &cs)
  : status_(STATUS_DEFAULT), init_data_(cs) {
  merror_t error = init_setup();
  if (!error) {
    // установить модель с наибольшим приоритетом
    swap_model();
  }
}

#if !defined(DATABASE_TEST)
#  ifdef _DEBUG
merror_t CalculationSetup::AddModel(std::shared_ptr<modelGeneral> &mg) {
  merror_t error = ERROR_INIT_T;
  if (mg && is_status_ok(status_)) {
    error = mg->GetError();
    if (!error) {
      models_.emplace(mg->GetPriority(), mg);
    }
  }
  return error;
}
#  endif  // _DEBUG

mstatus_t CalculationSetup::CheckCurrentModel() {
  if (current_model_ && is_status_ok(status_)) {
    /* todo прописать этот свап */
    // если использование выбранной модели не допустимо
    //   переключиться на другую
    if (!current_model_->IsValid()) {
      params_copy_ = current_model_->GetParametersCopy();
      swap_model();
    }
  }
  return status_;
}
#endif  // !DATABASE_TEST

merror_t CalculationSetup::SetModel(int model_key) {
  merror_t error = ERROR_GENERAL_T;
  const auto it = models_.find(model_key);
  if (it != models_.end()) {
    current_model_ = it->second.get();
    error = ERROR_SUCCESS_T;
  }
  return error;
}

merror_t CalculationSetup::GetError() const {
  return error_.GetErrorCode();
}

merror_t CalculationSetup::init_setup() {
  // todo: read files, init config
  // assert(0);
  // ради дебага
  params_copy_ = {0.004, 3000000, 350};
  status_ = STATUS_OK;
  return ERROR_SUCCESS_T;
}

void CalculationSetup::swap_model() {
  status_ = STATUS_NOT;
  auto const model_it = models_.cbegin();
  while (model_it != models_.cend()) {
    // первая модель для которой допустимы макропараметры
    if (model_it->second->IsValid(params_copy_)) {
      status_ = STATUS_OK;
    }
  }
}
