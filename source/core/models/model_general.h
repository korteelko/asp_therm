/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__MODELS__MODEL_GENERAL_H_
#define _CORE__MODELS__MODEL_GENERAL_H_

#include "ErrorWrap.h"
#include "atherm_common.h"
#include "gas_description.h"
#include "gas_description_static.h"
#include "gasmix_init.h"
#include "models_configurations.h"
#include "phase_diagram.h"

#include <array>
#include <functional>
#include <memory>

/* // flow dynamic modeling
typedef std::array<double, 2> difresult_t;
typedef std::function<void (const difresult_t &x, difresult_t &dxdt,
    double t)> difEquat_f;
*/
//   Acceptor
// get the functor of the model for calculating gas dynamic equation
class DerivateFunctor {
 public:
  virtual void getFunctor(class Ideal_Gas& mg) = 0;
  virtual void getFunctor(class Redlich_Kwong2& mg) = 0;
  virtual void getFunctor(class Redlich_Kwong_Soave& mg) = 0;
  virtual void getFunctor(class Peng_Robinson& mg) = 0;
  virtual void getFunctor(class NG_Gost& mg) = 0;

  virtual ~DerivateFunctor();
};

/**
 * \brief Обёртка над данными инициализации
 */
struct model_input {
  /**
   * \brief Флаги газовой смеси
   * */
  gas_marks_t gm;
  /**
   * \brief Указатель на точки бинодали
   * \note Всё-таки для газовых смесей вещь лишняя
   * */
  binodalpoints* bp = nullptr;
  /**
   * \brief Данные параметров газовой смеси
   * */
  gas_params_input gpi;
  /**
   * \brief Краткая информация о используемом уравнении состояния
   * */
  model_str ms;
  /**
   * \brief Приоритетом модели
   * */
  model_priority mpri;

  /* todo: add model_priority */
  model_input(gas_marks_t gm,
              binodalpoints* bp,
              gas_params_input gpi,
              model_str ms);
};

/** \brief Базовый абстрактный класс имплементации
 *   уравнения состояния реаьного газа(модели) */
class modelGeneral {
  modelGeneral(const modelGeneral&) = delete;
  modelGeneral& operator=(const modelGeneral&) = delete;

 public:
  /**
   * \brief Исключения создания расчётных моделей
   * */
  class model_init_exception final : public std::exception {
   public:
    model_init_exception(const model_str* info, const std::string& _msg);

    const char* what() const noexcept override;

   private:
    std::string msg;
  };

 public:
  static model_str GetModelShortInfo(const rg_model_id model_type);

  virtual ~modelGeneral();

  /**
   * \brief Функции обновления динамических параметров
   * */
  virtual void update_dyn_params(dyn_parameters& prev_state,
                                 const parameters new_state) = 0;
  virtual void update_dyn_params(dyn_parameters& prev_state,
                                 const parameters new_state,
                                 const const_parameters& cp) = 0;

  /** \brief Получить информацию о модели:
   *   уравнение состояния, его модификация,
   *   версия его модификации в программном обеспечении */
  virtual model_str GetModelShortInfo() const = 0;

  /** \brief Проверить допустимость использования данной модели
   *   для данной конфигурации газа(смеси газов) при
   *   текущих параметрах p, t */
  virtual bool IsValid() const = 0;
  /** \brief Проверить допустимость исходных макропараметров
   *   для использования данной модели
   * \note Вызывается при выбооре наилучшей модели из
   *   возможных в наборе сетапа расчёта */
  virtual bool IsValid(parameters par) const = 0;
  virtual void DynamicflowAccept(DerivateFunctor& df) = 0;
  virtual void SetVolume(double p, double t) = 0;
  /* todo: maybe remove it??? */
  virtual void SetPressure(double v, double t) = 0;
  virtual double GetVolume(double p, double t) = 0;
  virtual double GetPressure(double v, double t) = 0;

  void SetCalculationSetup(calculation_info* calculation);

  /** \brief Получить приоритет использования ОПРЕДЕЛЁННОЙ модели
   *   в ОПРЕДЕЛЁННОЙ конфигурации.
   * \note Потрясающая по точности модель ГОСТ вообще не
   *   может применяться вне определённого состава смесей и макро
   *   параметров(давление и температура). Соответственно, где
   *   нельзя её использовать лучше взять модификацию PR или RKS
   *   для опять же специализированных параметров смеси.
   *   Модификаций PR вообще говоря великое множество, по-этому,
   *   стараемся выбрать наиболее специализированное.
   *   Итак до самых общих уравнений состояния, типо идеального газа. */
  /* todo: я пока не представляю полностью систему учёта и
   *   выбора этого всего, т.к. необходимо учитывать и состав смеси
   *   и макро параметры */
  priority_var GetPriority() const;
  /**
   * \brief Получить указатель на конфигурацию модели
   * */
  const model_str* GetModelConfig() const;

  double GetVolume() const;
  double GetPressure() const;
  double GetTemperature() const;
  double GetAcentric() const;
  double GetCP() const;
  // double GetAdiabatic() const;  // only for ideal gas
  double GetT_K() const;
  state_phase GetState() const;
  parameters GetParametersCopy() const;
  const_parameters GetConstParameters() const;
  calculation_state_log GetStateLog() const;
  merror_t GetError() const;
  calculation_info* GetCalculationInfo() const;

  // todo: maybe remove it in another class
  std::string ParametersString() const;
  /**
   * \brief Получить строковое представление неизменяемых данных
   * \param pref Префикс перед новой строкой для соблюдения отступов
   *
   * \return Строковое представление структуры
   * */
  std::string ConstParametersString(std::string pref = "") const;
  static std::string sParametersStringHead();

 protected:
  /**
   * \brief Проверить входные параметры:
   *   - флаги(ошибка в коде)
   *   - количественную составляющую смеси(ГОСТовскую или простой микс)
   * \note throw model_init_exception
   * */
  static void check_input(const model_input& mi);
  /**
   * \brief Пересчитать сумму компонентов
   * */
  static double calculate_parts_sum(const model_input& mi);

  modelGeneral(model_str model_config, gas_marks_t gm, binodalpoints* bp);

  double vapor_part(int32_t index);
  state_phase set_state_phase(double v, double p, double t);
  int32_t set_state_phasesub(double p);
  void set_parameters(double v, double p, double t);
  void set_enthalpy();
  /** \brief Инициализировать структуру параметров газа parameters_ */
  void set_gasparameters(const gas_params_input& gpi, modelGeneral* mg);
  const GasParameters* get_gasparameters() const;

 protected:
  ErrorWrap error_;
  mstatus_t status_ = STATUS_DEFAULT;
  /**
   * \brief Информация о данной модели:
   *   тип, модификация, версия имплементации в данной программе
   * */
  model_str model_config_;
  gas_marks_t gm_;
  /**
   * \brief Приоритет модели
   * */
  model_priority priority_;
  /**
   * \brief Указатель на сетап расчёта
   * */
  calculation_info* calculation_ = nullptr;

  std::unique_ptr<GasParameters> parameters_ = nullptr;
  std::unique_ptr<binodalpoints> bp_ = nullptr;
};

#endif  // !_CORE__MODELS__MODEL_GENERAL_H_
