#include "model_general.h"

#include "gas_description_dynamic.h"
#include "models_errors.h"

#include <algorithm>

//==================================================================
// modelGeneral methods
//==================================================================
/// Одноэлементный газ
modelGeneral::modelGeneral(modelName mn, parameters prs,
    const_parameters cgp, dyn_parameters dgp, binodalpoints bp)
  : parameters_(nullptr),
    phasediag_model_(mn), bp_(bp) {}

/// Газовая смесь
modelGeneral::modelGeneral(modelName mn, parameters prs, parameters_mix components,
    binodalpoints bp)
  : parameters_(nullptr), phasediag_model_(mn), bp_(bp) {}

modelGeneral::~modelGeneral() {}

void modelGeneral::setParameters(double v, double p, double t) {
  parameters_->csetParameters(v, p, t, setState_phase(v, p, t));
}

int32_t modelGeneral::setState_phasesub(double p) {
  return (bp_.p.end() - std::find_if(bp_.p.begin() + 1, bp_.p.end(),
      std::bind2nd(std::less_equal<double>(), p)));
}

state_phase modelGeneral::setState_phase(
    double v, double p, double t) {
  if (t >= parameters_->cgetT_K())
    return (p >= parameters_->cgetP_K()) ? state_phase::SCF : state_phase::GAS;
  // if p on the left of binodal graph  -  liquid
  int32_t iter = setState_phasesub(p);
  if (!iter) {
    // std::cerr << " modelGeneral: gas have too low pressure\n";
    return ((v <= parameters_->cgetV_K()) ?
        state_phase::LIQ_STEAM : state_phase::GAS);
  }
  iter = bp_.p.size() - iter - 1;
  const double p_path = (p-bp_.p[iter+1]) / (bp_.p[iter]-bp_.p[iter+1]); // %
  // left branch of binodal
  if (v < parameters_->cgetV_K()) {                 
    const double vapprox = bp_.vLeft[iter] - (bp_.vLeft[iter+1] -
        bp_.vLeft[iter]) * p_path;
    return ((v < vapprox) ? state_phase::LIQUID : state_phase::LIQ_STEAM);
  }
  const double vapprox = bp_.vRigth[iter] + (bp_.vRigth[iter+1] -
      bp_.vRigth[iter])*p_path;
  return ((v > vapprox) ? state_phase::GAS : state_phase::LIQ_STEAM);
}
const GasParameters *modelGeneral::getGasParameters() const {
  // return NULL or pointer to GasParameters
  return parameters_.get();
}

double modelGeneral::GetVolume() const {
  return parameters_->cgetVolume();
}

double modelGeneral::GetPressure() const {
  return parameters_->cgetPressure();
}

double modelGeneral::GetTemperature() const {
  return parameters_->cgetTemperature();
}

double modelGeneral::GetCV() const {
  return parameters_->cgetCV();
}

double modelGeneral::GetAcentric() const {
  return parameters_->cgetAcentricFactor();
}

double modelGeneral::GetAdiabatic() const {
  return parameters_->cgetAdiabatic();
}

double modelGeneral::GetT_K() const {
  return parameters_->cgetT_K();
}

state_phase modelGeneral::GetState() const {
  return parameters_->cgetState();
}

parameters modelGeneral::GetParametersCopy() const {
  return parameters_->cgetParameters();
}

const_parameters modelGeneral::GetConstParameters() const {
  return parameters_->cgetConstparameters();
}

std::string modelGeneral::ParametersString() const {
  char str[256] = {0};
  auto prs  = parameters_->cgetParameters();
  auto dprs = parameters_->cgetDynParameters();
  sprintf(str, "%08.2f %08.4f %08.2f %08.2f %08.2f %08.2f\n", prs.pressure, prs.volume,
      prs.temperature, dprs.heat_cap_vol, dprs.heat_cap_pres, dprs.internal_energy);
  return std::string(str);
}

std::string modelGeneral::ConstParametersString() const {
  char str[256] = {0};
  const_parameters &cprs = parameters_->const_params;
  sprintf(str, "  Critical pnt: p=%8.2f; v=%8.4f; t=%8.2f\n"
      "  Others: mol_m=%6.3f R=%8.3f ac_f=%6.4f\n", cprs.P_K, cprs.V_K, cprs.T_K,
      cprs.molecularmass, cprs.R, cprs.acentricfactor);
  return std::string(str);
}

std::string modelGeneral::sParametersStringHead() {
  return "pressure volume   temperat cv       cp       u\n";
}