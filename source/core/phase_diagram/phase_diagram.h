#ifndef _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_
#define _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_

#include "phase_diagram_models.h"
#include "gas_mix_init.h"

#ifdef BOOST_LIB_ISED
#include <boost/noncopyable.hpp>
#endif  // BOOST_LIB_ISED

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

enum class modelName : uint32_t {
  REDLICH_KWONG2 = 0,
  PENG_ROBINSON
};

class PhaseDiagram;
class binodalpoints {
  friend class PhaseDiagram;
  binodalpoints();

public:
  // вектор значений безразмерной температуры по которым будут вычисляться
  //   параметры объёма и давления
  std::deque<double> t = {0.97, 0.95, 0.92, 0.9, 0.87, 0.85,
                          0.8, 0.75, 0.7, 0.6, 0.5},
                     vLeft,
                     vRigth,
                     p;
};

// Класс вычисляющий параметры(координаты) точек бинодали
/// class calculate points on diagram liquid-steam-gas
/// for more information visit:
///    https://en.wikipedia.org/wiki/Maxwell_construction
#ifdef BOOST_LIB_ISED
class PhaseDiagram: boost::noncopyable {
#else 
class PhaseDiagram {
  PhaseDiagram(const PhaseDiagram &) = delete;
  PhaseDiagram &operator=(const PhaseDiagram &) = delete;
#endif  // BOOST_LIB_ISED
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
  std::vector<std::function<double(double, double, double, double)>>
      line_integrate_f_{lineIntegrateRK2(), lineIntegratePR()};
  std::vector<std::function<void(std::vector<double>&, double, double, double)>>
      initialize_f_{initializeRK2(), initializePR()};
  // DEVELOP
  //   ASSERT
  // static_assert(line_integrate_f_.size() == initialize_f_,
  //    "Phase diagram: real gas models count error. See phase_diagram.h");

private:
  PhaseDiagram();

  void calculateBinodal(std::shared_ptr<binodalpoints> &bdp,
      modelName mn, double acentric);
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
      modelName mn, double acentric);
  // for gas_mix
  binodalpoints GetBinodalPoints(parameters_mix &components,
      modelName mn);
  // just for lulz
  void EraseBinodalPoints(modelName mn, double acentric);
};

bool operator< (const PhaseDiagram::uniqueMark &lum,
                const PhaseDiagram::uniqueMark &rum);

#endif  // _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_H_
