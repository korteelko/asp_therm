#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_STATIC_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_STATIC_H_

#include "gas_description.h"

#include <iostream>
#include <memory>

/*
 * PARAMETERS OF GAS IN STATIC.
 *   Base class for dynamic gas parameters
 * Параметры газа в статике.
 *   Базовый класс для динамических параметров
 * По сути это совокупность других структур
 *   с элементами ООП
*/
class GasParameters {
public:
  static ErrorWrap init_error;

protected:
  state_phase sph_;
  parameters  vpte_;
  dyn_parameters  dyn_params_;

public:
  const_parameters const_params;

protected:
  GasParameters(double v, double p, double t,
      const const_parameters cgp, dyn_parameters dgp);
  GasParameters(parameters prs,
      const const_parameters cgp, dyn_parameters dgp);
  static GasParameters *Init(parameters prs,
      const const_parameters cgp, dyn_parameters dgp);

public:
  double cgetV_K()            const;
  double cgetP_K()            const;
  double cgetT_K()            const;
  double cgetMolecularMass()  const;
  double cgetR()              const;
  double cgetAcentricFactor() const;
  double cgetVolume()         const;
  double cgetPressure()       const;
  double cgetTemperature()    const;
  dyn_setup cgetDynSetup()    const;
  double cgetIntEnergy()      const;
  state_phase cgetState()     const;
  parameters cgetParameters() const;
  dyn_parameters cgetDynParameters() const;
  const_parameters cgetConstparameters() const;

  double cgetCP()         const;
  double cgetBeta()       const;

  /** reset parameters of gas **/
  virtual void csetParameters(double v,
      double p, double t, state_phase sp);
  /** get volume of gas by pressure and temperature **/
  virtual double cCalculateVolume(double p, double t);
  virtual ~GasParameters();
};

std::ostream& operator<< (std::ostream &outstream, const GasParameters &gp);

#endif  // !_CORE__GAS_PARAMETERS__GAS_DESCRIPTION_STATIC_H_
