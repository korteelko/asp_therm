#ifndef _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_MODELS_H_
#define _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_MODELS_H_

#include <vector>

/*
 * Вычисление параметров точек бинодали, как и другие расчёты
 *   может осуществляться по разным уравнениям состояния реального
 *   газа. В этом модуле они объявлены и реализованы, модуль
 *   phase_diagram их использует.
*/

// integrate RK2 function (P = P(v,t)) from cleft to vrigth
struct lineIntegrateRK2 {
  double operator() (double t, double vLeft, double vRigth, double = 0);
};

// calculate volume by RK2 method
struct initializeRK2 {
  void operator() (std::vector<double>& tempvec, double pi,
      double t, double = 0);
};

// same for Reng-Robinson
struct lineIntegratePR {
  double operator() (double t, double vLeft, double vRigth, double ac);
};

struct initializePR {
  void operator() (std::vector<double>& tempvec, double pi,
      double t, double ac);
};
#endif  // _CORE__PHASE_DIAGRAM__PHASE_DIAGRAM_MODELS_H_

