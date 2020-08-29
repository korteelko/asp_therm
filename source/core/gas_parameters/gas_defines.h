/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__GAS_PARAMETERS__GAS_DEFINES_H_
#define _CORE__GAS_PARAMETERS__GAS_DEFINES_H_

#include "atherm_common.h"
#include "models_math.h"

#include <array>

#include <stdint.h>



// max count of components of natural gas in xml files
#define GASMIX_MAX_COUNT           32

typedef uint32_t gas_t;
#define GAS_TYPE_MIX               0xFF

/**
 * \brief Макрос на сокращение имён дефайнов
 * */
#define CH(x) (GAS_TYPE_ ## x)
/* Идентификаторы компонентов */
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
#define GAS_TYPE_ISO_BUTANE        0x0B   // C4H10
#define GAS_TYPE_N_PENTANE         0x0C   // C5H12
#define GAS_TYPE_ISO_PENTANE       0x0D   // C5H12
#define GAS_TYPE_OXYGEN            0x0E   // O2
#define GAS_TYPE_ARGON             0x0F   // Ar
#define GAS_TYPE_HEPTANE           0x11   // C7H16
#define GAS_TYPE_OCTANE            0x12   // C8H18

#define GAS_TYPE_NONANE            0x13   // C9H20
#define GAS_TYPE_DECANE            0x14   // C10H22
#define GAS_TYPE_CARBON_MONOXIDE   0x15   // CO
#define GAS_TYPE_WATER             0x16   // H2O
#ifdef ASSIGNMENT_TRACE_COMPONENTS
   // see table E.1 ISO 20765-1:2005
#  define ASSIGNMENT_COMPONENTS_MASK  0xFF00
#  define GAS_TYPE_NEO_PENTANE       (0x0100 | GAS_TYPE_N_PENTANE)     // C5H12
#  define GAS_TYPE_METHYL_PENTANE2   (0x0200 | GAS_TYPE_HEXANE)        // C6H14
#  define GAS_TYPE_METHYL_PENTANE3   (0x0300 | GAS_TYPE_HEXANE)        // C6H14
#  define GAS_TYPE_DIMETHYL_BUTANE2  (0x0400 | GAS_TYPE_HEXANE)        // C6H14
#  define GAS_TYPE_DIMETHYL_BUTANE3  (0x0500 | GAS_TYPE_HEXANE)        // C6H14
#  define GAS_TYPE_ETHYLEN           (0x0600 | GAS_TYPE_ETHANE)        // C2H4
#  define GAS_TYPE_PROPYLEN          (0x0700 | GAS_TYPE_PROPANE)       // C3H6
#  define GAS_TYPE_BUTENE1           (0x0800 | GAS_TYPE_N_BUTANE)      // C4H8
#  define GAS_TYPE_BUTENE2_CIS       (0x0900 | GAS_TYPE_N_BUTANE)      // C4H8
#  define GAS_TYPE_BUTENE2_TRANS     (0x0A00 | GAS_TYPE_N_BUTANE)      // C4H8
#  define GAS_TYPE_METHYL_PROPENE2   (0x0B00 | GAS_TYPE_N_BUTANE)      // C4H8
#  define GAS_TYPE_PENTENE1          (0x0C00 | GAS_TYPE_N_PENTANE)     // C5H10
#  define GAS_TYPE_PROPADIENTE       (0x0D00 | GAS_TYPE_PROPANE)       // C3H4
#  define GAS_TYPE_BUTADIENE2        (0x0E00 | GAS_TYPE_N_BUTANE)      // C4H6
#  define GAS_TYPE_BUTADIENE3        (0x0F00 | GAS_TYPE_N_BUTANE)      // C4H6
#  define GAS_TYPE_ACETYLENE         (0x1000 | GAS_TYPE_ETHANE)        // C2H2
#  define GAS_TYPE_CYCLOPENTENE      (0x1100 | GAS_TYPE_N_PENTANE)     // C5H10
#  define GAS_TYPE_MCYCLOPENTENE     (0x1200 | GAS_TYPE_HEXANE)        // C6H12
#  define GAS_TYPE_ECYCLOPENTENE     (0x1300 | GAS_TYPE_HEPTANE)       // C7H14
#  define GAS_TYPE_CYCLOHEXANE       (0x1400 | GAS_TYPE_HEXANE)        // C6H12
#  define GAS_TYPE_MCYCLOHEXANE      (0x1500 | GAS_TYPE_HEPTANE)       // C7H14
#  define GAS_TYPE_ECYCLOHEXANE      (0x1600 | GAS_TYPE_OCTANE)        // C8H16
   //  ароматические углеводороды
#  define GAS_TYPE_BENZENE           (0x1700 | GAS_TYPE_N_PENTANE)     // C6H6
#  define GAS_TYPE_TOLUENE           (0x1800 | GAS_TYPE_HEXANE)        // C7H8
#  define GAS_TYPE_ETHYLBENZENE      (0x1900 | GAS_TYPE_HEPTANE)       // C8H10
#  define GAS_TYPE_O_XYLENE          (0x1A00 | GAS_TYPE_HEPTANE)       // C8H10
   //  не ароматические
#  define GAS_TYPE_HYDROCARBONS_HEX  (0x1B00 | GAS_TYPE_HEXANE)        // C6
#  define GAS_TYPE_HYDROCARBONS_HEPT (0x1C00 | GAS_TYPE_HEPTANE)       // C7
#  define GAS_TYPE_HYDROCARBONS_OCT  (0x1D00 | GAS_TYPE_OCTANE)        // C8
#  define GAS_TYPE_HYDROCARBONS_NON  (0x1E00 | GAS_TYPE_NONANE)        // C9
#  define GAS_TYPE_HYDROCARBONS_DEC  (0x1F00 | GAS_TYPE_DECANE)        // C10
#  define GAS_TYPE_HYDROCARBONS_OTH  (0x2000 | GAS_TYPE_DECANE)        // C(>10)
#  define GAS_TYPE_METHANOL          (0x2100 | GAS_TYPE_ETHANE)        // CH3OH
#  define GAS_TYPE_METHANETHIOL      (0x2200 | GAS_TYPE_PROPANE)       // CH3SH
#  define GAS_TYPE_AMMONIA           (0x2300 | GAS_TYPE_METHANE)       // NH3
#  define GAS_TYPE_HYDROGEN_CYANIDE  (0x2400 | GAS_TYPE_ETHANE)        // HCN
#  define GAS_TYPE_CARBONYL_SULFIDE  (0x2500 | GAS_TYPE_N_BUTANE)      // COS
#  define GAS_TYPE_CARBON_DISULFIDE  (0x2600 | GAS_TYPE_N_PENTANE)     // CS2
#  define GAS_TYPE_SULFUR_DIOXIDE    (0x2700 | GAS_TYPE_N_BUTANE)      // SO2
#  define GAS_TYPE_NITROUS_OXIDE     (0x2800 | GAS_TYPE_CARBON_DIOXIDE)// N20
#  define GAS_TYPE_NEON              (0x2900 | GAS_TYPE_ARGON)         // Ne
#  define GAS_TYPE_KRYPTONE          (0x2A00 | GAS_TYPE_ARGON)         // Kr
#  define GAS_TYPE_XENON             (0x2B00 | GAS_TYPE_ARGON)         // Xe
#endif  // ASSIGNMENT_TRACE_COMPONENTS

/// +++ местами учитывается не определенный
///   тип газа, а сумма нескольких
#define GAS_TYPE_ALL_BUTANES       0xF1
#define GAS_TYPE_ALL_PENTANES      0xF2
#define GAS_TYPE_ALL_OTHER_ALKANES 0xF3  // OCTANE + NONANE + DECANE

/// gas constant(universal gas constant) 'R' [J/(K * mol)]
#define GAS_CONSTANT               8.314459


// state_phase enum || stateToString
/* todo: добавить состояния расслоения фаз(актуально для смесей) */
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
  GAS,
  /* специфические для смеси газов */
  LAMINATION,  // расслоение фаз в многокомпонентной смеси
  /* if binodal parameters isn't set */
  NOT_SET
};
static const std::array<std::string, 6> stateToString {
  "SCF", "LIQUID", "LIQ_STEAM", "GAS", "LAMINATION", "NOT_SET"
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

/*
 * dynamic
 */
/// setup of calculating dynamic parameters
///   init in model creation
typedef uint64_t dyn_setup;
#define DYNAMIC_SETUP_MASK         0x000000FF
#define DYNAMIC_SETUP_DEFAULT      0x00000022

#define DYNAMIC_HEAT_CAP_VOL       0x00000001
#define DYNAMIC_HEAT_CAP_PRES      0x00000002
#define DYNAMIC_INTERNAL_ENERGY    0x00000004
#define DYNAMIC_BETA_KR            0x00000008
#define DYNAMIC_HERMHOLTZ          0x00000010
#define DYNAMIC_ENTALPHY           0x00000020
#define DYNAMIC_GIBBS              0x00000040
#define DYNAMIC_LANDAUGRAND        0x00000080

#endif  // !_CORE__GAS_PARAMETERS__GAS_DEFINES_H_
