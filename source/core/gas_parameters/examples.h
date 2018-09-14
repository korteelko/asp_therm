#ifndef _CORE__GAS_PARAMETERS__EXAMPLES_H_
#define _CORE__GAS_PARAMETERS__EXAMPLES_H_

// #include "target_sys.h"
#include "models_errors.h"
#include "gas_description.h"
// #include "gas_description_dynamic.h"

extern bool is_initalized;

extern const_parameters *methane_cgp;
extern dyn_parameters *methane_dgp;

extern const_parameters *ethane_cgp;
extern dyn_parameters *ethane_dgp;

extern const_parameters *propane_cgp;
extern dyn_parameters *propane_dgp;

ERROR_TYPE init_examples();
#endif  // ! _CORE__GAS_PARAMETERS__EXAMPLES_H_
