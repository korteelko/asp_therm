#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_

#include "gas_description_static.h"

// блин, там вообще нету подобной хрени, всё пересчитывается каждый раз заново
// typedef std::pair<gas_t, double> gost_ng_components;

// const_dyn_parameters init_natural_gas(const gost_ng_components &comps);
class GasParameters_NG_Gost_dyn {
  ng_gost_mix components_;
  parameters prev_vpte_;
  // NG_Gost *model_;
  double coef_kx;

private:
  GasParameters_NG_Gost_dyn(parameters prs, ng_gost_mix components);
  void initKx();
  double getA0(double dens, double temp);

public:
  static GasParameters_NG_Gost_dyn *Init(gas_params_input &gpi);
  void csetParameters(double v, double p, double t, state_phase);
}; 

#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_