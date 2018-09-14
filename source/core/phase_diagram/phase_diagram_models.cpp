#include "phase_diagram_models.h"

#include <cmath>

/// magic numbers so magic
/// a priori t,vLeft,vRigth,pi > 0
/// values t, pi and both v - dimensionless
///


double lineIntegrateRK2::operator() (
    double t, double vLeft, double vRigth, double) {
  const double gam = 0.25992;
  return 3.0 * t * (std::log(std::abs(vRigth-gam)) - std::log(std::abs(vLeft-gam))) +
      14.80139 * (std::log((vRigth+gam)/vRigth) - std::log((vLeft+gam)/vLeft)) /
      std::sqrt(t);
}

void initializeRK2::operator() (std::vector<double> &tempvec,
                                                  double pi, double t, double) {
  tempvec[0] = 1.0;
  tempvec[1] = -3.0*t/pi;
  tempvec[2] = 3.84732/(pi*std::sqrt(t))-0.77976*t/pi-0.0675584;
  tempvec[3] = -1.0/(pi*std::sqrt(t));
}

double lineIntegratePR::operator() (
    double t, double vLeft, double vRigth, double ac) {
  const double J   = 3.253,
               alf = std::pow((1.0+(0.37464 + 1.54226 * ac-0.26992*ac*ac)*
                     (1.0-std::sqrt(t))), 2.0);
  return t*J*(std::log(std::abs(vRigth-0.25))-std::log(std::abs(vLeft-0.25))) +
      6.8448*alf*(std::log(std::abs((vRigth+0.60355)/(vRigth-0.10355))) -
      std::log(std::abs((vLeft+0.60355)/(vLeft-0.10355))));
}

void initializePR::operator() (std::vector<double> &tempvec,
                                              double pi, double t, double ac) {
  const double alf = std::pow((1.0+(0.37464 + 1.54226 * ac-0.26992*ac*ac)*(1.0-t)), 2.0);
  tempvec[0] = 1.0;
  tempvec[1] = 0.25307 - 3.253*t/pi;
  tempvec[2] = -0.192112 -1.646476*t/pi + 4.838465*alf/pi;
  tempvec[3] = -0.01625 + 0.20833*t/pi - 1.224472*alf/pi;
}
