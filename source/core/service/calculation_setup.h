/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SERVICE__CALCULATION_SETUP_H_
#define _CORE__SERVICE__CALCULATION_SETUP_H_

#include "atherm_common.h"
#include "calculation_info.h"
#include "model_general.h"

#include <map>


/** \brief конфигурация расчёта */
class CalculationSetup {
public:
  CalculationSetup(const calculation_setup &cs);
#if !defined(DATABASE_TEST)
#  ifdef _DEBUG
  merror_t AddModel(std::shared_ptr<modelGeneral> &mg);
#  endif  // _DEBUG
  /** \brief Проверить допустимость(валидность) используемой
    *   модели current_model_ */
  mstatus_t CheckCurrentModel();
#endif  // !DATABASE_TEST
  merror_t SetModel(int model_key);
  merror_t GetError() const;

private:
  merror_t init_setup();
  void swap_model();

private:
  ErrorWrap error_;
  mstatus_t status_;
  /** \brief копия установленных параметров */
  parameters params_copy_;
#ifdef _DEBUG
  /** \brief список используемых моделей для расчёта */
  std::multimap<priority_var, std::shared_ptr<modelGeneral>> models_;
#else
  /** \brief список используемых моделей для расчёта */
  std::multimap<priority_var, std::unique_ptr<modelGeneral>> models_;
#endif  // _DEBUG
  /** \brief ссылка на текущую используемую модель */
  modelGeneral *current_model_;
  /** \brief данные инициализации расчёта */
  calculation_setup init_data_;
  /** \brief данные расчётов динамики
    * \note в динамике отслеживается история расчётов,
    *   и вероятна последовательная алгоритмизация */
  dynamic_data dyn_data_;
};
using Calculations = std::map<int, CalculationSetup>;

#endif  // !_CORE__SERVICE__CALCULATION_SETUP_H_
