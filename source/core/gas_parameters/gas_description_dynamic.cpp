#include "gas_description_dynamic.h"

#include "model_general.h"

#include <cmath>
#include <utility>

#include <assert.h>

GasParameters_dyn::GasParameters_dyn(parameters prs,
    const_parameters cgp, dyn_parameters dgp,
    modelGeneral *mg)
  : GasParameters(prs, cgp, dgp), prev_vpte_(prs), model_(mg) {}

GasParameters_dyn *GasParameters_dyn::Init(gas_params_input gpi,
    modelGeneral *mg) {
  if (mg == nullptr)
    return nullptr;
  return new GasParameters_dyn({0.0, gpi.p, gpi.t},
      *gpi.const_dyn.cdp.cgp, *gpi.const_dyn.cdp.dgp, mg);
}

void GasParameters_dyn::csetParameters(double v, double p,
    double t, state_phase sp) {
  std::swap(prev_vpte_, vpte_);
  vpte_.volume       = v;
  vpte_.pressure     = p;
  vpte_.temperature  = t;
  sph_               = sp;
  model_->update_dyn_params(dyn_params_, vpte_);
}

double GasParameters_dyn::cCalculateVolume(double p, double t) {
  state_phase bsp = sph_;
  parameters bpar = vpte_;
  dyn_parameters bdpar = dyn_params_;
  model_->SetVolume(p,t);
  double v = vpte_.volume;
  std::swap(bsp, sph_);
  std::swap(bpar, vpte_);
  std::swap(bdpar, dyn_params_);
  return v;
}

std::ostream &operator<< (std::ostream &outstream,
    const GasParameters_dyn &gp) {
  outstream << "v: " << gp.cgetVolume() << " p: " << gp.cgetPressure()
      << " t: " << gp.cgetTemperature() << "\n";
  return outstream;
}
