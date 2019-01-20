#ifndef _CORE__COMMON__COMMON_H_
#define _CORE__COMMON__COMMON_H_

#include <stdint.h>

// separate calculation of parameters of mix 
//   просто интересно
#define GAS_MIX_VARIANT
#define BY_PSEUDO_CRITIC

#define MODEL_IDEAL_GAS       0x01
#define MODEL_REDLICH_KWONG   0x02
#define MODEL_PENG_ROBINSON   0x03
#define MODEL_NG_GOST         0x04

#define MODEL_MASK            0x0000000F
#define BINODAL_MODEL_MASK    0x000000F0
#define BINODAL_MODEL_SHIFT   4

#define MARK_MASK             0x0000FF00
#define GAS_MIX_MARK          0x00000100
// ГОСТ 30319.3-2015
#define GAS_NG_GOST_MARK      0x00000200
#define MARK_MASK_SHIFT       8

typedef uint32_t GAS_MARKS;
typedef uint32_t MODEL_MARKS;

enum class rg_model_t : uint32_t {
  IDEAL_GAS = MODEL_IDEAL_GAS,
  REDLICH_KWONG2 = MODEL_REDLICH_KWONG,
  PENG_ROBINSON = MODEL_PENG_ROBINSON,
  NG_GOST = MODEL_NG_GOST
};

#endif  // !_CORE__COMMON__COMMON_H_
