/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _DYNAMIC_MODELING__FINITE_VOLUME_H_
#define _DYNAMIC_MODELING__FINITE_VOLUME_H_

#include "common.h"
#include "model_general.h"

#include <memory>

struct nozzle {
  double square;
  double coef;
  // and others and others and others
};

struct heating_ {
  double a[0];
};

// mass dynamic interface
class massDynamic {
  nozzle nz_;

protected:
  massDynamic();
  virtual ~massDynamic();

public:
  virtual int FlowIn() = 0;
  virtual int FlowOut() = 0;
};

// heat dynamic interface
class heatDynamic {
protected:
  heatDynamic();
  virtual ~heatDynamic();

public:
};

class tankBase : public massDynamic, public heatDynamic {
  double volume_,
         surface_area_;
  std::unique_ptr<modelGeneral> mg_;
   
protected:
  tankBase(double f, double sf);

public:
  virtual void SetGasPressure(double v, double t) = 0;
  virtual void SetGasVolume(double p, double t) = 0;
  virtual ~tankBase();
};


class RoundTank final: protected tankBase {
  double radius_;

private:
  RoundTank(double radius_);
  
public:
};

#endif  // !_DYNAMIC_MODELING__FINITE_VOLUME_H_