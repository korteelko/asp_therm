/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include <calculation_setup.h>


/* CalculationSetup */
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
#ifdef _DEBUG
  params_copy_ = {0.004, 3000000, 350};
  status_ = STATUS_OK;
  return ERROR_SUCCESS_T;
#else
  return ERROR_GENERAL_T;
#endif  // _DEBUG
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
