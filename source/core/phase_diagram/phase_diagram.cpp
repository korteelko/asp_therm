/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "phase_diagram.h"

#include "asp_utils/Logging.h"
#include "gas_description.h"
#include "models_configurations.h"
#include "models_math.h"

#include <algorithm>
#include <tuple>
#include <utility>
#ifdef _DEBUG
#include <iostream>
#endif

#define FUNCTIONS_INDEX_OUT 0xFF

PhaseDiagram ::uniqueMark::uniqueMark(rg_model_id mn, gas_t gt)
    : mn(mn), gt(gt) {}

size_t PhaseDiagram::set_functions_index(rg_model_id mn) {
  switch (mn.type) {
    case rg_model_t::REDLICH_KWONG:
      return (mn.subtype == MODEL_SUBTYPE_DEFAULT) ? 0 : FUNCTIONS_INDEX_OUT;
    case rg_model_t::PENG_ROBINSON:
      return (mn.subtype == MODEL_SUBTYPE_DEFAULT) ? 1 : FUNCTIONS_INDEX_OUT;
    default:
      return FUNCTIONS_INDEX_OUT;
  }
}

void PhaseDiagram::calculateBinodal(std::shared_ptr<binodalpoints>& bdp,
                                    rg_model_id mn,
                                    double acentric) {
  const uint32_t nPoints = bdp->t.size();
  // Суть правила Максвелла: Расчитанные значения va и vb лежат на
  //   бинодали если
  //   площадь под кривой изотермы состояния(p=p(v,t)) от va до vb
  //   равна площади прямоугольника(p=const) от va до vb
  // См. картинку Vanderwaals2.jpg в этой папке
  double pi,
      dpi,          // current pressure and pressure correction
      spline_area,  // area under function t(p,v) from v0 to v2;
      rectan_area;  // area under pi=const, from v0 to v2;
                    // Maxwell construction requirement splineAR==rectanAR;
  size_t functions_index = set_functions_index(mn);
  if (functions_index == FUNCTIONS_INDEX_OUT) {
    error_.SetError(ERROR_INIT_T, "error in programmer DNA");
    return;
  }
  auto integrateFun = line_integrate_f_.at(functions_index);
  auto inializeFun = initialize_f_.at(functions_index);
  merror_t error = ERROR_SUCCESS_T;
  // get pressure by volume and temperature
  for (size_t t_iter = 0; t_iter < nPoints; ++t_iter) {
    std::vector<double> tempvec = {1.0,
                                   -9.0 / (4.0 * bdp->t[t_iter]),
                                   6.0 / (4.0 * bdp->t[t_iter]),
                                   -1.0 / (4.0 * bdp->t[t_iter]),
                                   0.0,
                                   0.0,
                                   0.0};
    // calculate derivate(dpressure/dtemperature)=0
    int roots_count = 0;
    error = CardanoMethod_roots_count(&tempvec[0], &tempvec[4], &roots_count);
    bool has_uniq_root = (roots_count == 1) ? true : false;
    if (error) {
      Logging::Append(io_loglvl::debug_logs,
                      "For temperature index: " + std::to_string(t_iter)
                          + "\nCardano method error(phase_diagram)");
      bdp->t[t_iter] = -1.0;
      continue;
    }
    if (has_uniq_root) {
      bdp->t[t_iter] = -1.0;
      continue;
    }
    std::sort(tempvec.begin() + 4, tempvec.end());
    assert((tempvec[4] >= 0.0) && (tempvec[5] >= 0.0) && (tempvec[6] >= 0.0));
    pi = bdp->t[t_iter] * bdp->t[t_iter] * bdp->t[t_iter];
    if (pi <= 0.0)
      pi = 0.01;
    dpi = -pi * 0.002;
    uint32_t trycount = 0;
    // DEVELOP
    //   HORRIBLE
    // Вот здесь метод не очень. Суть его плохости:
    //   В не гибком выборе приращения dpi
    // Method of calculating is horrible:
    //   choosing of dpi is not flexible
    // Shame on me
    while (true) {
      // Чтобы цикл не сваливался в бесконечный
      //   если решение не сходится
      if (trycount > 3000) {
        bdp->t[t_iter] = -1.0;
        break;
      }
      ++trycount;
      // Особенность вычислений: вблизи критической точки
      //   сходимость хуже
      // descrease dpi for field located nearby to critical point
      if (t_iter < 4)
        dpi *= 0.1;
      pi += dpi;
      // calculate volume
      inializeFun(tempvec, pi, bdp->t[t_iter], acentric);
      int roots_count = 0;
      error = CardanoMethod_roots_count(&tempvec[0], &tempvec[4], &roots_count);
      bool has_uniq_root = (roots_count == 1) ? true : false;
      if (error) {
        bdp->t[t_iter] = -1.0;
        break;
      }
      if (has_uniq_root) {
        dpi = 0.002 * pi;
        pi = (tempvec[4] <= 1.0) ? (pi - 2.0 * dpi) : (pi + 2.0 * dpi);
        continue;
      }
      assert((tempvec[4] >= 0.0) && (tempvec[5] >= 0.0) && (tempvec[6] >= 0.0));
      std::sort(tempvec.begin() + 4, tempvec.end());
      if (is_equal(tempvec[4], tempvec[6], 0.0001)) {
        bdp->t[t_iter] = -1.0;
        break;
      }
      bdp->vLeft[t_iter] = tempvec[4];
      bdp->vRigth[t_iter] = tempvec[6];
      // Вычислить площадь и безразмерную разницу
      rectan_area = (tempvec[6] - tempvec[4]) * pi;
      spline_area = integrateFun(bdp->t[t_iter], bdp->vLeft[t_iter],
                                 bdp->vRigth[t_iter], acentric);
      double ARdiffer = (rectan_area - spline_area) / rectan_area;
      // Если разница между площадями меньше 0.5%
      //   считаем ответ верным
      if (std::abs(ARdiffer) < 0.005) {
        bdp->p[t_iter] = pi;
        break;
        // Если разница больше 0.5% пересчитаем давление
      } else if (ARdiffer > 0.0) {
        // DEVELOP
        //   Может реализовать вычисление производной и через неё пересчитать
        pi -= 2.0 * dpi;
      } else {
        pi += 3.0 * dpi;
      }
      dpi = 0.002 * pi;
    }
  }
}

// erase not calculated points
void PhaseDiagram::checkResult(std::shared_ptr<binodalpoints>& bdp) {
  size_t sizebdp = bdp->t.size();
  for (size_t i = 0; i < sizebdp; ++i) {
    if (bdp->t[i] < -0.5) {
      eraseElements(bdp, i);
      --i;
      --sizebdp;
    }
  }
  // testing on negative values in vectors
  searchNegative(bdp, bdp->p);
  searchNegative(bdp, bdp->vLeft);
  searchNegative(bdp, bdp->vRigth);
}

void PhaseDiagram::eraseElements(std::shared_ptr<binodalpoints>& bdp,
                                 const size_t i) {
  bdp->t.erase(bdp->t.begin() + i);
  bdp->p.erase(bdp->p.begin() + i);
  bdp->vLeft.erase(bdp->vLeft.begin() + i);
  bdp->vRigth.erase(bdp->vRigth.begin() + i);
}

void PhaseDiagram::searchNegative(std::shared_ptr<binodalpoints>& bdp,
                                  std::deque<double>& v) {
  auto hasnegative = [](std::deque<double>& vec) {
    return vec.end() - std::find_if(vec.begin(), vec.end(), [](const auto v) {
             return v < DOUBLE_ACCURACY;
           });
  };
  int hasneg = 1;
  int vecsize = v.size();
  while (true) {
    if ((hasneg = hasnegative(v))) {
      eraseElements(bdp, vecsize - hasneg);
      --vecsize;
      continue;
    }
    break;
  }
}

PhaseDiagram::PhaseDiagram() {}

bool PhaseDiagram::IsValidModel(rg_model_id mn) {
  return set_functions_index(mn) != FUNCTIONS_INDEX_OUT;
}

PhaseDiagram& PhaseDiagram::GetCalculated() {
  static PhaseDiagram phd;
  return phd;
}

binodalpoints* PhaseDiagram::GetBinodalPoints(const const_parameters& cp,
                                              const rg_model_id& id) {
  binodalpoints* bp = nullptr;
  if (!cp.IsGasmix() && PhaseDiagram::IsValidModel(id)) {
    auto f = [](std::deque<double>& vec, double K) {
      std::transform(vec.begin(), vec.end(), vec.begin(),
                     [K](const auto e) { return K * e; });
    };
    uniqueMark um(id, cp.gas_name);
    // если для таких параметров(модель и фактор ацентричности)
    //   бинодаль ещё не рассчитана -- рассчитать и сохранить
    std::shared_ptr<binodalpoints> bdp(new binodalpoints(id));
    calculateBinodal(bdp, id, cp.acentricfactor);
    checkResult(bdp);
    bdp->p.push_front(1.0);
    bdp->t.push_front(1.0);
    bdp->vLeft.push_front(1.0);
    bdp->vRigth.push_front(1.0);
    if (!cp.IsAbstractGas())
      if (calculated_.find(um) == calculated_.end())
        calculated_.emplace(um, bdp);
    bp = new binodalpoints(*bdp);
    f(bp->vLeft, cp.critical.volume);
    f(bp->vRigth, cp.critical.volume);
    f(bp->p, cp.critical.pressure);
    f(bp->t, cp.critical.temperature);
    bp->mn = um.mn;
  }
  return bp;
}

binodalpoints* PhaseDiagram::GetBinodalPoints(parameters_mix& components,
                                              const rg_model_id& id) {
  // 25.01.2019
  // Здесь нужно прописать как считать линию перехода для газовых смесей
  // UPD: 17.03.2020
  // todo: здесь неправильно - переделать(V_k для смеси - не пойми что)
  const auto max_el = components.upper_bound(0.95);
  if (max_el == components.end() || max_el->first < 0.95)
    // todo: перевести условие в более удобоваримую форму
    return nullptr;
  return PhaseDiagram::GetBinodalPoints(max_el->second.first, id);
}

merror_t PhaseDiagram::GetError() const {
  return error_.GetErrorCode();
}

bool operator<(const PhaseDiagram::uniqueMark& lum,
               const PhaseDiagram::uniqueMark& rum) {
  return std::tie(lum.mn.type, lum.mn.subtype, lum.gt)
         < std::tie(rum.mn.type, rum.mn.subtype, rum.gt);
}

binodalpoints::binodalpoints(rg_model_id mn)
    : mn(mn), vLeft(t.size(), 0.0), vRigth(t.size(), 0.0), p(t.size(), 0.0) {}
