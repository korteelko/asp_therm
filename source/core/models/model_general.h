#ifndef _CORE__MODELS__MODEL_GENERAL_H_
#define _CORE__MODELS__MODEL_GENERAL_H_

#include "target_sys.h"

#include "common.h"
#include "gas_description.h"
#include "gas_description_static.h"
#include "gas_mix_init.h"
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
// класс для восстановления типа модели
// DEVELOP
//   Acceptor
// get the functor of the model for calculating gas dynamic equation
class DerivateFunctor {
public:
  virtual void getFunctor(class Ideal_Gas &mg) = 0;
  virtual void getFunctor(class Redlich_Kwong2 &mg) = 0;
  virtual void getFunctor(class Peng_Robinson &mg) = 0;
  virtual ~DerivateFunctor() {}
};

struct model_input {
  GAS_MARKS gm;
  const binodalpoints &bp;
  gas_params_input gpi;
};

class modelGeneral {
  modelGeneral(const modelGeneral &) = delete;
  modelGeneral &operator=(const modelGeneral &) = delete;
protected:
  // Храним указатель на GasParameters
  //   чтобы создать возможность использовать ООП для расчёта динамики газа
  // parameters_ must be a pointer for enable
  //   swap static-dynamic parameters
  std::unique_ptr<GasParameters> parameters_;
  // имя модели для расчёта точек бинодали
  // model for calculating phase diagram
  GAS_MARKS gm_;
  // binodal points
  binodalpoints bp_;

 // dyn_params_update params_update_f_;

protected:
  modelGeneral(GAS_MARKS gm, binodalpoints bp);

  double vapor_part(int32_t index);
  state_phase setState_phase(double v, double p, double t);
  int32_t  setState_phasesub(double p);
  void setParameters(double v, double p, double t);
  // virtual void set_enthalpy() = 0;
  void set_enthalpy();
//  void setDynamicParameters();
//  void setStaticParameters();
  bool set_gasparameters(const gas_params_input &gpi,
      modelGeneral *mg);
  const GasParameters *getGasParameters() const;
  static bool check_input(const model_input &mi);

public:
//  virtual double internal_energy_integral(
//      const parameters state) = 0;
  /// Функция обновления динамических параметров
  /// Update dyn_parameters function
  virtual void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state) = 0;
  /// Update dyn_parameters function for 
  virtual void update_dyn_params(dyn_parameters &prev_state,
      const parameters new_state, const const_parameters &cp) = 0;

  // Модели имеют определенные границы применения
  //   метод isValid проверяет их
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
  double GetCV() const;
  double GetAdiabatic() const;
  double GetT_K() const;
  state_phase GetState() const;
  parameters GetParametersCopy() const;
  const_parameters GetConstParameters() const;

// maybe another class
  std::string ParametersString() const;
  std::string ConstParametersString() const;
  static std::string sParametersStringHead();

  virtual ~modelGeneral();
};
#endif  // ! _CORE__MODELS__MODEL_GENERAL_H_
