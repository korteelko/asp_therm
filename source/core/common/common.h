#ifndef _CORE__COMMON__COMMON_H_
#define _CORE__COMMON__COMMON_H_

#include <stdint.h>
// #define _DEBUG (_DEBUG by cmake defines)
// TEST defines
#define GAS_MIX_VARIANT

typedef unsigned int GAS_MARKS;

#define MODEL_IDEALGAS       0x01
#define MODEL_REDLICHKWONG   0x02
#define MODEL_PENGROBINSON   0x03

#define MODEL_MASK           0x0000000F
#define BINODAL_MODEL_MASK   0x000000F0
#define BINODAL_MODEL_SHIFT  4

#define MARK_MASK            0x0000FF00
#define GAS_MIX_MARK         0x00000100
#define MARK_MASK_SHIFT      8

enum class modelName : uint32_t {
  IDEAL_GAS = MODEL_IDEALGAS,
  REDLICH_KWONG2 = MODEL_REDLICHKWONG,
  PENG_ROBINSON = MODEL_PENGROBINSON
};

#endif  // _CORE__COMMON__COMMON_H_
