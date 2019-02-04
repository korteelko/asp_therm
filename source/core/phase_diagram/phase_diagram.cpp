#include "phase_diagram.h"

#include "models_errors.h"
#include "models_math.h"

#include <algorithm>
#include <tuple>
#include <utility>
#ifdef _DEBUG
#  include <iostream>
#endif

size_t PhaseDiagram::set_functions_index(rg_model_t mn) {
  size_t functions_index = 0xFF;
  for (size_t i = 0; i < functions_indexes_.size(); ++i)
    if (functions_indexes_[i] == mn) {
      functions_index = i;
      break;
    }
  return functions_index;
}

void PhaseDiagram::calculateBinodal(
    std::shared_ptr<binodalpoints>& bdp, rg_model_t mn, double acentric) {
  const uint32_t nPoints = bdp->t.size();
  // Суть правила Максвелла: Расчитанные значения va и vb лежат на
  //   бинодали если
  //   площадь под кривой изотермы состояния(p=p(v,t)) от va до vb
  //   равна площади прямоугольника(p=const) от va до vb
  // См. картинку Vanderwaals2.jpg в этой папке
  double  pi,
          dpi,            // current pressure and pressure correction
          spline_area,       // area under function t(p,v) from v0 to v2;
          rectan_area;       // area under pi=const, from v0 to v2;
                        // Maxwell construction requirement splineAR==rectanAR;
  size_t functions_index = set_functions_index(mn);
  if (functions_index == 0xFF) {
    set_error_message(ERR_INIT_T, "error in programmer DNA");
    return;
  }
  auto integrateFun = line_integrate_f_.at(functions_index);
  auto inializeFun  = initialize_f_.at(functions_index);
  // get pressure by volume and temperature
  for (size_t t_iter = 0; t_iter < nPoints; ++t_iter) {
    std::vector<double> tempvec = {
        1.0,
        -9.0/(4.0*bdp->t[t_iter]),
        6.0/(4.0*bdp->t[t_iter]),
        -1.0/(4.0*bdp->t[t_iter]),
        0.0, 0.0, 0.0
    };
    // calculate derivate(dpressure/dtemperature)=0
    reset_error();
    bool has_uniq_root = CardanoMethod_HASUNIQROOT(&tempvec[0], &tempvec[4]);
    if (get_error_code() != ERR_SUCCESS_T) {
  #ifdef _DEBUG
      std::cerr << " for temperature index: "
          << std::to_string(t_iter) << " Cardano method error \n";
  #endif  // _DEBUG
      bdp->t[t_iter] = -1.0;
      continue;
    }
    if (has_uniq_root) {
      bdp->t[t_iter] = -1.0;
      continue;
    }
    std::sort(tempvec.begin() + 4, tempvec.end());
    assert((tempvec[4] >= 0.0) && (tempvec[5] >= 0.0) && (tempvec[6] >= 0.0));
    pi = bdp->t[t_iter]*bdp->t[t_iter]*bdp->t[t_iter];
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
      pi+=dpi;
      // calculate volume
      inializeFun(tempvec, pi, bdp->t[t_iter], acentric);
      reset_error();
      bool has_uniq_root = CardanoMethod_HASUNIQROOT(&tempvec[0], &tempvec[4]);
      if (get_error_code() != ERR_SUCCESS_T) {
        bdp->t[t_iter] = -1.0;
        break;
      }
      if (has_uniq_root) {
        dpi = 0.002*pi;
        pi  = (tempvec[4] <= 1.0) ? (pi - 2.0*dpi) : (pi + 2.0*dpi);
        continue;
      }
      assert((tempvec[4] >= 0.0) && (tempvec[5] >= 0.0) && (tempvec[6] >= 0.0));
      std::sort(tempvec.begin() + 4, tempvec.end());
      if (tempvec[4] == tempvec[6]) {
        bdp->t[t_iter] = -1.0;
        break;
      }
      bdp->vLeft[t_iter]  = tempvec[4];
      bdp->vRigth[t_iter] = tempvec[6];
      // Вычислить площадь и безразмерную разницу
      rectan_area = (tempvec[6]-tempvec[4])*pi;
      spline_area = integrateFun(bdp->t[t_iter], bdp->vLeft[t_iter],
          bdp->vRigth[t_iter], acentric);
      double ARdiffer = (rectan_area -spline_area) / rectan_area;
      // Если разница между площадями меньше 0.5%
      //   считаем ответ верным
      if (std::abs(ARdiffer) < 0.005) {
        bdp->p[t_iter] = pi;
        break;
      // Если разница больше 0.5% пересчитаем давление
      } else if (ARdiffer > 0.0) {
        // DEVELOP
        //   Может реализовать вычисление производной и через неё пересчитать
        pi -= 2.0*dpi;
      } else {
        pi += 3.0*dpi;
      }
      dpi = 0.002*pi;
    }
  }
}

// erase not calculated points
void PhaseDiagram::checkResult(
    std::shared_ptr<binodalpoints> &bdp) {
  uint32_t sizebdp = bdp->t.size();
  for (uint32_t i = 0; i < sizebdp; ++i) {
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

void PhaseDiagram::eraseElements(
    std::shared_ptr<binodalpoints> &bdp, const uint32_t i) {
  bdp->t.erase(bdp->t.begin() + i);
  bdp->p.erase(bdp->p.begin() + i);
  bdp-> vLeft.erase(bdp->vLeft.begin() + i);
  bdp->vRigth.erase(bdp->vRigth.begin() + i);
}

void PhaseDiagram::searchNegative(
    std::shared_ptr<binodalpoints> &bdp, std::deque<double> &v) {
  auto hasnegative = [](std::deque<double>& vec) {
      return vec.end() - std::find_if(vec.begin(), vec.end(),
          std::bind2nd(std::less_equal<double>(), 0.0));};
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

PhaseDiagram &PhaseDiagram::GetCalculated() {
  static PhaseDiagram phd;
  return phd;
}

binodalpoints PhaseDiagram::GetBinodalPoints(double VK, double PK,
    double TK, rg_model_t mn, double acentric) {
  reset_error();
  bool isValid = is_above0(VK, PK, TK, acentric);
  if (!isValid) {
    set_error_code(ERR_CALCULATE_T);
    set_error_message("PhaseDiagram::getBinodalPoints get incorrect data:\n"
        " V_K, P_K, T_K or acentric_factor <= 0.0 or is NaN");
    return binodalpoints();
  }
  uniqueMark um {(uint32_t)mn, acentric};
  // если для таких параметров(модель и фактор ацентричности)
  //   бинодаль ещё не рассчитана -- рассчитать и сохранить
  if (calculated_.find(um) == calculated_.end()) {
   // std::lock_guard<std::mutex> lg(mtx);
    std::shared_ptr<binodalpoints> bdp(new binodalpoints());
    calculateBinodal(bdp, mn, acentric);
    checkResult(bdp);
    bdp->p.push_front(1.0);
    bdp->t.push_front(1.0);
    bdp->vLeft.push_front(1.0);
    bdp->vRigth.push_front(1.0);
    calculated_.emplace(um, bdp);
  }
  // найти в среди сохраненных бинодалей нужную,
  //   восстановить размерность и вернуть
  binodalpoints bp((*calculated_.find(um)).second.operator *());
  auto f = [] (std::deque<double> &vec, double K) {
      std::transform(vec.begin(), vec.end(), vec.begin(),
          std::bind1st(std::multiplies<double>(), K));};
  f(bp.vLeft, VK);
  f(bp.vRigth, VK);
  f(bp.p, PK);
  f(bp.t, TK);
  bp.mn = mn;
  return bp;
}

binodalpoints PhaseDiagram::GetBinodalPoints(parameters_mix &components,
    rg_model_t mn) {
// 25.01.2019
// Здесь нужно прописать как считать линию перехода для газовых смесей
  std::unique_ptr<const_parameters> cgp = 
      GasParameters_mix_dyn::GetAverageParams(components);
  return PhaseDiagram::GetBinodalPoints(cgp->V_K, cgp->P_K, cgp->T_K, 
      mn, cgp->acentricfactor);
}
// DEVELOP
//   NOT TESTED
void PhaseDiagram::EraseBinodalPoints(rg_model_t mn,
    double acentric) {
// std::lock_guard<std::mutex> lg(mtx);
  auto iter = calculated_.find({(size_t)mn, acentric});
  if (iter != calculated_.end())
    calculated_.erase(iter);
}

bool operator< (const PhaseDiagram::uniqueMark &lum,
    const PhaseDiagram::uniqueMark &rum) {
  return std::tie(lum.mn, lum.acentricfactor) <
      std::tuple<size_t, double>(rum.mn, rum.acentricfactor + 0.0001) &&
         std::tie(rum.mn, rum.acentricfactor) >
      std::tuple<size_t, double>(lum.mn, lum.acentricfactor + 0.0001);
}

binodalpoints::binodalpoints()
  : vLeft(t.size(), 0.0), vRigth(t.size(), 0.0), p(t.size(), 0.0) {}
