#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_

#include "gas_description_static.h"
#include "models_errors.h"

// размерности, константы, параметры при НФУ см.
//   в ГОСТ 30319.1-2015 (!!! коэффициенты в третьем(30319.3) а константы в первом)

// const_dyn_parameters init_natural_gas(const gost_ng_components &comps);
class GasParameters_NG_Gost_dyn {
  ng_gost_mix components_;
  parameters vpte_;
  // NG_Gost *model_;
  double coef_kx;
  double coef_G,
         coef_Q,
         coef_F,
         coef_V;

private:
  GasParameters_NG_Gost_dyn(parameters prs, ng_gost_mix components);
  ERROR_TYPE initKx();
  double sigma_start() const;
  double getA0_sub();
  void setV();
  void setQ();
  void setF();
  void setG();
  double getA0(double dens, double temp);

public:
  static GasParameters_NG_Gost_dyn *Init(gas_params_input &gpi);
  void csetParameters(double v, double p, double t, state_phase);
}; 

#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_