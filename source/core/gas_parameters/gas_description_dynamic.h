#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_DYNAMIC_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_DYNAMIC_H_

#include "gas_description.h"
#include "gas_description_static.h"

#include <iostream>

class GasParameters_dyn final: public GasParameters {
  GasParameters_dyn(parameters prs, const_parameters cgp,
      dyn_parameters dgp, modelGeneral *mg);

public:
  static GasParameters_dyn *Init(gas_params_input gpi, modelGeneral *mg);
  void csetParameters(double v, double p, double t, state_phase sp) override;

private:
  parameters   prev_vpte_;
  modelGeneral *model_;
};

std::ostream &operator<< (std::ostream &outstream,
    const GasParameters_dyn &gp);

#endif  // ! _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_DYNAMIC_H_
