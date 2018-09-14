#include "examples.h"

#include <assert.h>

bool is_initalized = false;

typedef int32_t gas_t;

gas_t methane = 0;
gas_t ethane  = 1;
gas_t propane = 2;

const int32_t gas_examples_count = 3;

const_parameters *methane_cgp; 
dyn_parameters *methane_dgp;
// to get cp by T (look eCp.pdf)
double methane_cp_coef[4] = {
  19.9, 50.24, 12.69, -11.01 
};
// parameters for calculate dyn_p
//   v, p, t
parameters methane_pars = {1.0 / 0.6682, 10e5, 298};

const_parameters *ethane_cgp;
dyn_parameters *ethane_dgp;
double ethane_cp_coef[4] = {
  6.9, 172.7, -64.06, 7.28
};
parameters ethane_pars = {1.0 / 1.2601, 10e5, 298};

const_parameters *propane_cgp;
dyn_parameters *propane_dgp;
double propane_cp_coef[4] = {
  -4.0, 304.8, -157.2, 31.74
};
parameters propane_pars = {1.0 / 1.8641, 10e5, 298};

static double *cp_coefs[gas_examples_count]= {
  methane_cp_coef,
  ethane_cp_coef,
  propane_cp_coef
};

static parameters *pars[gas_examples_count] = {
  &methane_pars,
  &ethane_pars,
  &propane_pars 
};

static double cv_by_mayer(double cp, double Rm) {
  return cp - Rm;
}

/// get cp(for 1 mole) by temperature
/// T is temperature, K
///   error of this method
///     <1% for 300 < T < 1500
static double get_cp(gas_t typ, double T) {
  return cp_coefs[typ][0] + cp_coefs[typ][1]*T + 
      cp_coefs[typ][2]*T*T + cp_coefs[typ][3]*T*T*T;
}

static dyn_parameters *init_dgp(gas_t typ, double R) {
  double cp = get_cp(typ, pars[typ]->temperature);
  double cv = cv_by_mayer(cp, R);
  return dyn_parameters::Init(cv, cp, 
      cv*pars[typ]->temperature, *pars[typ]);
}

ERROR_TYPE init_examples() {
  is_initalized = true;
  ERROR_TYPE err = ERR_SUCCESS_T; 
  // methane
  methane_cgp = const_parameters::Init(0.00746, 4.7 * 10e6, 191, 16.04, 0.01);
  if (!methane_cgp) {
    set_error_code(ERR_INIT_T);
    set_error_message("examples init methane cgp");
    return ERR_INIT_T;
  }
  methane_dgp = init_dgp(methane, methane_cgp->R);
  if (!methane_dgp) {
    set_error_code(ERR_INIT_T);
    set_error_message("examples init methane dgp");
    return ERR_INIT_T;
  }
  // ethane
  ethane_cgp = const_parameters::Init(0.00552, 5.2 * 10e6, 312, 30.068, 0.1);
  if (!ethane_cgp) {
    set_error_code(ERR_INIT_T);
    set_error_message("examples init ethane cgp");
    return ERR_INIT_T;
  }
  ethane_dgp = init_dgp(ethane, ethane_cgp->R);
  if (!ethane_dgp) {
    set_error_code(ERR_INIT_T);
    set_error_message("examples init ethane dgp");
    return ERR_INIT_T;
  }
  // propane
  propane_cgp = const_parameters::Init(0.00542, 4.3 * 10e6, 373, 44.09, 0.152);
  if (!propane_cgp) {
    set_error_code(ERR_INIT_T);
    set_error_message("examples init propane cgp");
    return ERR_INIT_T;
  }
  propane_dgp = init_dgp(propane, propane_cgp->R);
  if (!propane_dgp) {
    set_error_code(ERR_INIT_T);
    set_error_message("examples init propane dgp");
    return ERR_INIT_T;
  }
  return err;
}
