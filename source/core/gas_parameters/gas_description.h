#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_

#include <array>
#include <map>
#include <string>
#include <utility>

#include <stdint.h>

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
#define GAS_TYPE_N_BUTAN           0x0A   // C4H10
#define GAS_TYPE_I_BUTAN           0x0B   // C4H10
#define GAS_TYPE_N_PENTANE         0x0C   // C5H12
#define GAS_TYPE_I_PENTANE         0x0D   // C5H12
#define GAS_TYPE_MIX               0xFF

typedef uint32_t gas_t;
bool is_valid_gas(gas_t gas_name);

// state_phase enum || stateToString
/// Агрегатное состояние вещества (как )
/// SCF: t>T_K, p>P_K;    GAS: p_binodal < p < p_K, t>t_binodal;
/// LIQUID: p<P_K; v<vleft;
///   in perspective:  LIQ_STEAM: p<p_binodal, vleft < v < vrigth;
/// Без метастабильных состояний (между бинодалью и спинодалью)
/// There are not metastable states (between binodal and spinodal)
enum class state_phase : int32_t {
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

/// Динамические параметры вещества, зависящие от
///   других его параметров
struct dyn_parameters {
  double heat_cap_vol,     // heat capacity for volume = const // Cv
         heat_cap_pres,    // heat capacity for pressure = const // Cp
         internal_energy,  //
         beta_kr;          // beta_kr=beta_kr(adiabatic_index)
                           //   = [0.0, ... ,1.0]
                           //   if (pressure_1/pressure_2 < beta_kr):
                           //     flow velocity = const, (maximum)
                           //   else : flow velocity ~ p,
                           //     adiabatic_index and etc
                           //   P.S. look dynamic_modeling*.*

  parameters parm;         // current parameters

private:
  dyn_parameters(double cv, double cp, double int_eng, parameters pm);

public:
  static dyn_parameters *Init(double cv, double cp, double int_eng,
      parameters pm);
  void Update();
};

class modelGeneral;
struct potentials {
  double  // internalenergy,
         Hermholtz_free,
         enthalpy,
         Gibbsfree,
         LandauGrand,
         // entropy not potential but calculating in dynamic have sense
         entropy;
};

/// параметры газа, зависящие от его физической природы и
///   не изменяющиеся при изменении его состояния
struct const_parameters {
  const gas_t gas_name;
  const double V_K,              // K point parameters (critical point)
               P_K,
               T_K,
               molecularmass,
               R,                // gas constant
               acentricfactor;

private:
  const_parameters(gas_t gas_name, double vk, double pk, double tk, double mol,
      double R, double af);
  const_parameters &operator= (const const_parameters &) = delete;

public:
  static const_parameters *Init(gas_t gas_name, double vk, double pk,
      double tk, double mol, double af);
  const_parameters(const const_parameters &cgp);
};

bool is_valid_cgp(const const_parameters &cgp);
bool is_valid_dgp(const dyn_parameters &dgp);

typedef std::pair<const_parameters, dyn_parameters>
    const_dyn_parameters;
typedef std::multimap<const double, const_dyn_parameters> parameters_mix;

struct gas_params_input {
  double p, t;
  union cd {
    const parameters_mix *components;
    struct cd_pair {
      const const_parameters *cgp;
      const dyn_parameters *dgp;
    } cdp;
  } const_dyn;
};

struct state_log {
  dyn_parameters dyn_pars;    // p, v, t and cp(p,v,t), cv(p,v,t), u(p,v,t)
  double enthalpy;
  std::string state_phase;
};
#endif  // !_CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
