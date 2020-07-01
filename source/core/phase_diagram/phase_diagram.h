/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_
#define _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_

#include "atherm_common.h"
#include "gasmix_init.h"
#include "ErrorWrap.h"
#include "phase_diagram_models.h"

#include <cassert>
#include <deque>
#include <exception>
#include <functional>
#include <map>
#include <memory>
#include <mutex>
#include <vector>

#include <stdint.h>

/*
 * Модуль расчета параметров точек на бинодали.
 *   За физическое обоснование принято правило Максвелла
 *   (см. ссылку на wiki ниже).
 *
 * DEVELOP Метод расчёта параметров очень ленивый и уродский.
 *   Возможно стоит того же Ньютона намотатьвместо этого ужаса
 *   (комментарий HORRIBLE)
*/

class PhaseDiagram;
class binodalpoints {
  friend class PhaseDiagram;
  binodalpoints(rg_model_id mn);

public:
  rg_model_id mn;
  // вектор значений безразмерной температуры по которым будут вычисляться
  //   параметры объёма и давления
  //   BASIC STRUCT
  std::deque<double> 
      t = std::deque<double> {
          0.97, 0.95, 0.92, 0.9, 0.87, 0.85,
          0.8, 0.75, 0.7, 0.6, 0.5},
      vLeft,
      vRigth,
      p;
  std::deque<double> hLeft, hRigth;
};

/** \brief Класс вычисляющий параметры(координаты) точек бинодали
  * \note Теоретическое обоснование есть только для чистых веществ
  * Расчёта линии перехода для смесей - отдельная наука
  * N.b. вблизи критической точки обычно используют
  *   модифицированные уравнения состояния. */
/* todo: можно ли рассматривать ситуацию по самому нестабильному элементу? */
class PhaseDiagram {
// Правило Максвела:
//    https://en.wikipedia.org/wiki/Maxwell_construction
  PhaseDiagram(const PhaseDiagram &) = delete;
  PhaseDiagram &operator=(const PhaseDiagram &) = delete;
  PhaseDiagram(PhaseDiagram &&) = delete;
  PhaseDiagram &operator=(PhaseDiagram &&) = delete;
public:
  class PhaseDiagramException;

private:
  /** \brief уникальный идентификатор расчёта, хранит информацию о
    *   расчётной модели и газе.
    *  Используется для индексирования ранее рассчитаных линий */
  struct uniqueMark {
    /** \brief использованная модель */
    rg_model_id mn;
    /** \brief идентификатор газа */
    gas_t gt;

    /** \brief неопределённый газ GAS_TYPE_UNDEFINED */
    uniqueMark(rg_model_id mn, gas_t gt = GAS_TYPE_UNDEFINED);
  };
  friend bool operator<(const PhaseDiagram::uniqueMark &lum,
      const PhaseDiagram::uniqueMark &rum);

  typedef std::function<double(double, double, double, double)> integ_func_t;
  typedef std::function<void(
      std::vector<double>&, double, double, double)> init_func_t;

  // Мьютекс здесь не нужен, ввиду отсутствия каких либо потоков(нитей),
  //   для многопоточности придётся вводить как минимум ООП исключения
  // std::mutex mtx;
  ErrorWrap error_;

  /* calculated points storage */
  std::map<uniqueMark, std::shared_ptr<binodalpoints>> calculated_;

  /* storage of function pointers() */
  std::vector<rg_model_id> functions_indexes_ = std::vector<rg_model_id> {
      rg_model_id(rg_model_t::REDLICH_KWONG, MODEL_SUBTYPE_DEFAULT),
      rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_SUBTYPE_DEFAULT)};
  std::vector<integ_func_t> line_integrate_f_ =
      std::vector<integ_func_t> {lineIntegrateRK2(), lineIntegratePR()};
  std::vector<init_func_t> initialize_f_ = 
      std::vector<init_func_t> {initializeRK2(), initializePR()};
  // DEVELOP
  //   ASSERT
  // static_assert(line_integrate_f_.size() == initialize_f_,
  //    "Phase diagram: real gas models count error. See phase_diagram.h");

private:
  PhaseDiagram();

  static size_t set_functions_index(rg_model_id mn);

  void calculateBinodal(std::shared_ptr<binodalpoints> &bdp,
      rg_model_id mn, double acentric);
  void checkResult(std::shared_ptr<binodalpoints> &bdp);
  void eraseElements(std::shared_ptr<binodalpoints> &bdp, const size_t i);
  void searchNegative(std::shared_ptr<binodalpoints> &bdp,
      std::deque<double> &v);

public:
  static bool IsValidModel(rg_model_id mn);
  // Просто метод возвращающий Синглетон-объект
  static PhaseDiagram &GetCalculated();
  // Рассчитать или получить копию, если уже было рассчитано,
  //   для этих параметров, точек на бинодали.
  /* note: свапнул model_str -> rg_model_id, т.к. модель вычисления
   *   не связана с вычислением бинодали */
  binodalpoints *GetBinodalPoints(const const_parameters &cp,
      const rg_model_id &id);
  // for gas_mix
  binodalpoints *GetBinodalPoints(parameters_mix &components,
      const rg_model_id &id);

  merror_t GetError() const;
};

bool operator<(const PhaseDiagram::uniqueMark &lum,
    const PhaseDiagram::uniqueMark &rum);

#endif  // !_CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_
