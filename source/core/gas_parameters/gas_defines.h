#ifndef _CORE__GAS_PARAMETERS__GAS_DEFINES_H_
#define _CORE__GAS_PARAMETERS__GAS_DEFINES_H_

#include <stdint.h>
#include <array>

#define GAS_TYPE_MIX               0xFF

#define GAS_TYPE_UNDEFINED         0x00

#define GAS_TYPE_METHANE           0x01   // CH4
#define GAS_TYPE_ETHANE            0x02   // C2H6
#define GAS_TYPE_PROPANE           0x03   // C3H8
#define GAS_TYPE_HYDROGEN_SULFIDE  0x04   // H2S
#define GAS_TYPE_HEXANE            0x05   // C6H14   // n-Hexane
#define GAS_TYPE_NITROGEN          0x06   // N2
#define GAS_TYPE_CARBON_DIOXIDE    0x07   // CO2
#define GAS_TYPE_HELIUM            0x08   // He
#define GAS_TYPE_HYDROGEN          0x09   // H2
#define GAS_TYPE_N_BUTANE          0x0A   // C4H10
#define GAS_TYPE_I_BUTANE          0x0B   // C4H10
#define GAS_TYPE_N_PENTANE         0x0C   // C5H12
#define GAS_TYPE_I_PENTANE         0x0D   // C5H12
#define GAS_TYPE_OXYGEN            0x0E   // O2
#define GAS_TYPE_ARGON             0x0F   // Ar
#define GAS_TYPE_HEPTANE           0x11   // C7H16
#define GAS_TYPE_OCTANE            0x12   // C8H18

// +++ местами учитывается не определенный
//   тип газа, а сумма нескольких
#define GAS_TYPE_ALL_PENTANES      0x13
#define GAS_TYPE_ALL_BUTANES       0x14

typedef uint32_t gas_t;

// state_phase enum || stateToString
/// Агрегатное состояние вещества (как )
/// SCF: t>T_K, p>P_K;    GAS: p_binodal < p < p_K, t>t_binodal;
/// LIQUID: p<P_K; v<vleft;
///   in perspective:  LIQ_STEAM: p<p_binodal, vleft < v < vrigth;
/// Без метастабильных состояний (между бинодалью и спинодалью)
/// There are not metastable states (between binodal and spinodal)
enum class state_phase : uint32_t {
  SCF = 0,
  LIQUID,
  LIQ_STEAM,
  GAS
};
static const std::array<std::string, 4> stateToString {
  "SCF", "LIQUID", "LIQ_STEAM", "GAS"
};

/// Общие параметры состояния вещества,
///   для описания его текущего состояния с размерностями
/// Common parameters of substance for describing
///   current state with dimensions
struct parameters {
  double  volume,               // m^3 / kg
          pressure,             // Pa
          temperature;          // K
};

#endif  // !_CORE__GAS_PARAMETERS__GAS_DEFINES_H_
