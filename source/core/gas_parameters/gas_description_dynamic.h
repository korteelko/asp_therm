/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
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
  double cCalculateVolume(double p, double t) override;

private:
  parameters   prev_vpte_;
  modelGeneral *model_;
};

std::ostream &operator<< (std::ostream &outstream,
    const GasParameters_dyn &gp);

#endif  // !_CORE__GAS_PARAMETERS__GAS_DESCRIPTION_DYNAMIC_H_
