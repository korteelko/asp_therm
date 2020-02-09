#ifndef _CORE__MODELS__MODEL_GENERAL_H_
#define _CORE__MODELS__MODEL_GENERAL_H_

#include "target_sys.h"

#include "common.h"
#include "gas_description.h"
#include "gas_description_static.h"
#include "gasmix_init.h"
#include "phase_diagram.h"

#ifdef BOOST_LIB_USED
#  include <boost/noncopyable.hpp>
#endif  // BOOST_LIB_USED

#include <array>
#include <functional>
#include <memory>

/* // flow dynamic modeling
typedef std::array<double, 2> difresult_t;
typedef std::function<void (const difresult_t &x, difresult_t &dxdt,
    double t)> difEquat_f;
*/
//   Acceptor
// get the functor of the model for calculating gas dynamic equation
class DerivateFunctor {
public:
  virtual void getFunctor(class Ideal_Gas &mg) = 0;
  virtual void getFunctor(class Redlich_Kwong2 &mg) = 0;
  virtual void getFunctor(class Peng_Robinson &mg) = 0;
  virtual void getFunctor(class NG_Gost &mg) = 0;
  virtual ~DerivateFunctor();
};

struct model_input {
  gas_marks_t gm;
  binodalpoints *bp;
  gas_params_input gpi;
};

class modelGeneral {
  modelGeneral(const modelGeneral &) = delete;
  modelGeneral &operator=(const modelGeneral &) = delete;
protected:
  mutable merror_t error_;
  rg_model_t model_conf_;
  gas_marks_t gm_;
  std::unique_ptr<GasParameters> parameters_;
  std::unique_ptr<binodalpoints> bp_;

protected:
  modelGeneral(gas_marks_t gm, binodalpoints *bp);

  double vapor_part(int32_t index);
  state_phase set_state_phase(double v, double p, double t);
  int32_t set_state_phasesub(double p);
  void set_parameters(double v, double p, double t);
  void set_enthalpy();
  bool set_gasparameters(const gas_params_input &gpi,
      modelGeneral *mg);
  const GasParameters *get_gasparameters() const;

  static merror_t check_input(const model_input &mi);

public:
  /// Функции обновления динамических параметров
  virtual void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) = 0;
  virtual void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) = 0;

  // Models have application limits
  //  isValid - method for check this limits
  virtual bool IsValid() const = 0;
  virtual void DynamicflowAccept(DerivateFunctor &df) = 0;
  virtual double InitVolume(double p, double t,
      const const_parameters &cp) = 0;
  virtual void SetVolume(double p, double t) = 0;
  virtual void SetPressure(double v, double t) = 0;
#ifndef GAS_MIX_VARIANT
  virtual double GetVolume(double p, double t) const = 0;
  virtual double GetPressure(double v, double t) const = 0;
#else
  virtual double GetVolume(double p, double t) = 0;
  virtual double GetPressure(double v, double t) = 0;
#endif  // !GAS_MIX_VARIANT

  double GetVolume() const;
  double GetPressure() const;
  double GetTemperature() const;
  double GetAcentric() const;
  double GetCP() const;
  // double GetAdiabatic() const;  // only for ideal gas
  double GetT_K() const;
  state_phase GetState() const;
  parameters GetParametersCopy() const;
  const_parameters GetConstParameters() const;
  calculation_state_log GetStateLog() const;
  merror_t GetError() const;

// maybe another class
  std::string ParametersString() const;
  std::string ConstParametersString() const;
  static std::string sParametersStringHead();

  virtual ~modelGeneral();
};
#endif  // !_CORE__MODELS__MODEL_GENERAL_H_
