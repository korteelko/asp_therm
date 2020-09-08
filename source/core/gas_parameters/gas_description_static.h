/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
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
/* todo: make abstract
 * Убрать init_error! */
class GasParameters {
protected:
  ErrorWrap error_;
  mstatus_t status_;
  state_phase sph_;
  parameters vpte_;
  dyn_parameters dyn_params_;

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
  /* todo: remove it
   *   Использование критических параметров для смесей
   *   не рекомендуется */
  double cgetV_K() const;
  double cgetP_K() const;
  double cgetT_K() const;
  double cgetAcentricFactor() const;

  double cgetMolecularMass() const;
  double cgetR() const;
  double cgetVolume() const;
  double cgetPressure() const;
  double cgetTemperature() const;
  dyn_setup cgetDynSetup() const;
  double cgetIntEnergy() const;
  state_phase cgetState() const;
  parameters cgetParameters() const;
  dyn_parameters cgetDynParameters() const;
  const_parameters cgetConstparameters() const;

  double cgetCP() const;
  double cgetBeta() const;

  mstatus_t cGetStatus() const;
  merror_t cGetError() const;

  /** reset parameters of gas **/
  virtual void csetParameters(double v,
      double p, double t, state_phase sp);
  /** get volume of gas by pressure and temperature **/
  virtual double cCalculateVolume(double p, double t);
  virtual ~GasParameters();
};

using gp = GasParameters;
inline merror_t gp::cGetError() const { return error_.GetErrorCode(); }
inline mstatus_t gp::cGetStatus() const { return status_; }

/* todo: check this */
std::ostream& operator<< (std::ostream &outstream, const GasParameters &gp);

#endif  // !_CORE__GAS_PARAMETERS__GAS_DESCRIPTION_STATIC_H_
