#include "model_ng_gost.h"

#include "common.h"
#include "gas_description_dynamic.h"
#include "models_errors.h"
#include "models_math.h"

#ifdef _DEBUG
#  include <iostream>
#endif  // _DEBUG

namespace {

}  // anonymus namespace

NG_Gost::NG_Gost(const model_input &mi) 
  : modelGeneral(mi.gm, mi.bp) {}

NG_Gost *NG_Gost::Init(const model_input &mi) {
  reset_error();
  if (!check_input(mi))
    return nullptr;
  // only for gas_mix
  if (!(mi.gm & GAS_MIX_MARK))
    return nullptr;
}