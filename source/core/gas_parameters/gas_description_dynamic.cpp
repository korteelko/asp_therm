#include "gas_description_dynamic.h"

#include "model_general.h"

#include <cmath>
#include <utility>

#include <assert.h>

GasParameters_dyn::GasParameters_dyn(parameters prs,
    const_parameters cgp, dyn_parameters dgp,
    modelGeneral *mg)
  : GasParameters(prs, cgp, dgp), prev_vpte_(prs), model_(mg) {}
 //   update_f_(&modelGeneral::update_dyn_params) {}

GasParameters_dyn *GasParameters_dyn::Init(double v, double p, double t,
    const_parameters cgp, dyn_parameters dgp, modelGeneral *mg) {
  if (mg == nullptr)
    return nullptr;
  return new GasParameters_dyn({v,p,t}, cgp, dgp, mg);
}

GasParameters_dyn *GasParameters_dyn::Init(parameters prs,
    const_parameters cgp, dyn_parameters dgp, modelGeneral *mg) {
  if (mg == nullptr)
    return nullptr;
  return new GasParameters_dyn(prs, cgp, dgp, mg);
}

void GasParameters_dyn::csetParameters(double v, double p,
    double t, state_phase sp) {
  std::swap(prev_vpte_, vpte_);
  vpte_.volume       = v;
  vpte_.pressure     = p;
  vpte_.temperature  = t;
  sph_               = sp;
  model_->update_dyn_params(dyn_params_, vpte_);
 // assert(0);
  // update dyn_pars
  // updatePotentials();
}

std::ostream &operator<< (std::ostream &outstream,
    const GasParameters_dyn &gp) {
  outstream << "v: " << gp.cgetVolume() << " p: " << gp.cgetPressure()
            << " t: " << gp.cgetTemperature() << "\n";
  return outstream;
}
