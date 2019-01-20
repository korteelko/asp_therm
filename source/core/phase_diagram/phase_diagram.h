#ifndef _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_
#define _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_

#include "common.h"
#include "gasmix_init.h"
#include "phase_diagram_models.h"
#include "target_sys.h"

#ifdef BOOST_LIB_USED
#  include <boost/noncopyable.hpp>
#endif  // BOOST_LIB_USED

#include <cassert>
#include <deque>
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
  binodalpoints();

public:
  rg_model_t mn;
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

class binodalpoints_extend {
  binodalpoints bp_;

public:
  binodalpoints_extend(binodalpoints bp);
};

// Класс вычисляющий параметры(координаты) точек бинодали
/// class calculate points on diagram liquid-steam-gas
/// for more information visit:
///    https://en.wikipedia.org/wiki/Maxwell_construction
#ifdef BOOST_LIB_USED
class PhaseDiagram: boost::noncopyable {
#else 
class PhaseDiagram {
  PhaseDiagram(const PhaseDiagram &) = delete;
  PhaseDiagram &operator=(const PhaseDiagram &) = delete;
#endif  // BOOST_LIB_USED
  typedef std::function<double(double, double, double, double)> integ_func_t;
  typedef std::function<void(
      std::vector<double>&, double, double, double)> init_func_t;
  struct uniqueMark {
    uint32_t mn;
    double   acentricfactor;
  };

  friend bool operator< (const PhaseDiagram::uniqueMark& lum,
      const PhaseDiagram::uniqueMark& rum);

  // Мьютекс здесь не нужен, ввиду отсутствия каких либо потоков(нитей),
  //   для многопоточности придётся вводить как минимум ООП исключения
  // std::mutex mtx;
  std::map<uniqueMark, std::shared_ptr<binodalpoints>> calculated_;
  std::vector<integ_func_t> line_integrate_f_ = 
      std::vector<integ_func_t> {
          lineIntegrateRK2(), lineIntegratePR()};
  std::vector<init_func_t> initialize_f_ = 
      std::vector<init_func_t> {initializeRK2(), initializePR()};
  std::vector<rg_model_t> functions_indexes_ =
     std::vector<rg_model_t> {rg_model_t::REDLICH_KWONG2,
     rg_model_t::PENG_ROBINSON};
  // DEVELOP
  //   ASSERT
  // static_assert(line_integrate_f_.size() == initialize_f_,
  //    "Phase diagram: real gas models count error. See phase_diagram.h");

private:
  PhaseDiagram();

  size_t set_functions_index(rg_model_t mn);
  void calculateBinodal(std::shared_ptr<binodalpoints> &bdp,
      rg_model_t mn, double acentric);
  void checkResult(std::shared_ptr<binodalpoints> &bdp);
  void eraseElements(std::shared_ptr<binodalpoints> &bdp, const uint32_t i);
  void searchNegative(std::shared_ptr<binodalpoints> &bdp,
      std::deque<double> &v);

public:
  // Просто метод возвращающий Синглетон-объект
  static PhaseDiagram &GetCalculated();
  // Рассчитать или получить копию, если уже было рассчитано,
  //   для этих параметров, точек на бинодали.
  binodalpoints GetBinodalPoints(double VK, double PK, double TK,
      rg_model_t mn, double acentric);
  // for gas_mix
  binodalpoints GetBinodalPoints(parameters_mix &components,
      rg_model_t mn);
  // just for lulz
  void EraseBinodalPoints(rg_model_t mn, double acentric);
};

bool operator< (const PhaseDiagram::uniqueMark &lum,
    const PhaseDiagram::uniqueMark &rum);

#endif  // !_CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_
