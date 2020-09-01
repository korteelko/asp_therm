/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_

#include "atherm_common.h"
#include "ErrorWrap.h"
#include "gas_defines.h"

#include <map>
#include <string>
#include <utility>
#include <vector>

#include <stdint.h>


gas_t gas_by_name(const std::string &name);

inline double volume_by_compress(double p, double t, double mol, double z) {
  return z * 1000.0 * GAS_CONSTANT * t / (p * mol);
}

inline double compress_by_volume(double p, double t, double mol, double v) {
  return v * p * mol / (1000.0 * GAS_CONSTANT * t);
}

/// Динамические параметры вещества, зависящие от
///   других его параметров
struct dyn_parameters {
  mstatus_t status;
  dyn_setup setup;
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
  // in future)
  // struct therm_potentials potentials;

private:
  dyn_parameters(dyn_setup setup, double cv, double cp,
      double int_eng, parameters pm);
  void check_setup();

  static merror_t check_input(dyn_setup setup, double cv, double cp,
      double int_eng, parameters pm);
  static merror_t check_input(parameters pm,
      const std::map<dyn_setup, double> &params);

public:
  /**
   * \brief Инициализировать структуру, проверив входные параметры
   * */
  static dyn_parameters *Init(dyn_setup setup, double cv,
      double cp, double int_eng, parameters pm);
  /**
   * \brief Инициализировать пустую структуру(заглушка для газовых смесей).
   *   Инициализировать после можно методом ResetParameters
   * */
  dyn_parameters();
  /**
   * \brief Инициализировать структуру, проверив входные параметры
   * \todo Удалить
   * \deprecated Лучше через мапу
   * */
  merror_t ResetParameters(dyn_setup setup, double cv,
      double cp, double int_eng, parameters pm);
  /**
   * \brief Инициализировать структуру по словарю параметров,
   *   проверив входные параметры
   * \param pm Макропараметры точки
   * \param params Словарь параметров, ключи - целочисленный дефайн,
   *   параметры - значения параметров
   * \todo Доделать, энтальпию например
   * */
  merror_t ResetParameters(parameters pm,
      const std::map<dyn_setup, double> &params);
  void Update();
};

/* так хочется успеть сделать, но мало времени */
struct therm_potentials {
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
  static ErrorWrap init_error;

  const gas_t gas_name;
  const double V_K,              // K point parameters (critical point)
               P_K,
               T_K,
               Z_K,
               molecularmass,
               R,                // gas constant
               acentricfactor;

private:
  const_parameters(gas_t gas_name, double vk, double pk, double tk,
      double zk, double mol, double R, double af);
  const_parameters &operator=(const const_parameters &) = delete;

public:
  static const_parameters *Init(gas_t gas_name, double vk, double pk,
      double tk, double zk, double mol, double af);
  /**
   * \brief Проверить параметры компонента
   * \param vk Удельный объём в критической точке
   * \param pk Давление в критической точке
   * \param tk Температура в критической точке
   * \param zk Фактор сжимаемость в критической точке
   * \param mol молекулярная масса
   * \param af Фактор ацентричности
   * \return true Для допустимых входных данных
   * \note Просто проверяем что все данные больше нуля
   *   за исключением:
   *   -- фактора ацентричности - может быть меньше 0.0
   *   -- vk и zk - инициализирован должен быть только один из них
   * */
  static bool check_params(double vk, double pk, double tk,
      double zk, double mol, double af);

  const_parameters(const const_parameters &cgp);

  /**
   * \brief Параметры инициализированы для газовой смеси
   * */
  bool IsGasmix() const;
  /**
   * \brief Параметры инициализированы для неопределённого газа
   * */
  bool IsAbstractGas() const;
};

bool is_valid_cgp(const const_parameters &cgp);
bool is_valid_dgp(const dyn_parameters &dgp);

/*
 * mixes defines
 * Смесь газов это по сути дефолтная субстанция, с коей
 *   имеем дело, но всё же матаппарат работы с чистым
 *   вецеством скорее всего пригодится
 */
class modelGeneral;
struct model_str;
typedef std::pair<const_parameters, dyn_parameters>
    const_dyn_parameters;
typedef std::multimap<const double, const_dyn_parameters> parameters_mix;

/* mix by gost */
/// vol_part is volume part
typedef double vol_part;
/// pair <gas_t, (double) vol_part>
typedef std::pair<gas_t, vol_part> ng_gost_component;
/// vector of ng_gost_component(<gas_t, vol_part>)
typedef std::vector<ng_gost_component> ng_gost_mix;

/*
 * wrap defines
 */
/**
 * \brief implementation of input for gas( -
 *   - .compononents - parameters_mix;  -
 *   - .ng_gost_components - ng_gost_mix; -
 *   - .cdp{.cgp, .dgp} - single const and dynamic  )
 * \todo мэйби от union в 2к20 отказаться уже
 * */
union const_dyn_union {
  const parameters_mix *components;
  const ng_gost_mix *ng_gost_components;
  struct {
    const const_parameters *cgp;
    const dyn_parameters *dgp;
  } cdp;
  ~const_dyn_union();
};

/**
 * \brief Входные данные инициализации газовой смеси
 * */
struct gas_params_input {
public:
  /**
   * \brief Первые значение давления и температуры
   * */
  double p, t;
  /**
   * \brief Обёртка указателя на константные данные смеси и
   *   стартовые динамические
   * */
  const_dyn_union const_dyn;
};

/** \brief ключ для мапы бинарных коэффициентов  */
struct gas_pair {
  /** \brief значения сортированы: i >= j, для быстроко поиска */
  gas_t i, j;

  gas_pair() = delete;
  gas_pair(gas_t f, gas_t s);
  bool operator<(const gas_pair &rhs) const;
};
/** \brief мапа бинарных коэффициентов  */
typedef std::map<const gas_pair, double> binary_coef_map;

struct calculation_info;
/**
 * \brief Структура для добавления в базу данных
 * */
struct calculation_state_log {
public:
  /**
   * \brief Установить динамические параметры
   * \param dp Ссылка на динамические параметры
   * */
  calculation_state_log &SetDynPars(const dyn_parameters &dp);
  /**
   * \brief Установить указатель на структуру расчётов
   * \param ci Указатель на структуру информации о расчёте
   * */
  calculation_state_log &SetCalculationInfo(calculation_info *ci);

public:
  enum state_info_flags {
    f_empty = 0x00,
    /** \brief Уникальный id информации по расчёту */
    f_calculation_info_id = 0x01,
    /** \brief Ссылка на данные расчёта */
    // f_info_id = 0x02,
    /** \brief Объём */
    f_vol = 0x04,
    /** \brief Давление */
    f_pres = 0x08,
    /** \brief Температура */
    f_temp = 0x10,
    /** \brief Изохорная теплоёмкость */
    f_dcv = 0x20,
    /** \brief Изобарная теплоёмкость */
    f_dcp = 0x40,
    /** \brief Внутреняя энергия */
    f_din = 0x80,
    /** \brief Параметр Bk */
    f_dbk = 0x100,
    /** \brief Энтальпия */
    f_enthalpy = 0x200,
    /** \brief Фазовое состояние */
    f_state_phase = 0x400,
    /** \brief Уникальный id строки расчёта */
    f_calculation_state_log_id = 0x800,
    f_full = 0xFFF
  };

  int32_t id;
  int32_t info_id;
  const calculation_info *calculation = nullptr;

  dyn_parameters dyn_pars;    // p, v, t and cp(p,v,t), cv(p,v,t), u(p,v,t)
  double enthalpy;
  std::string state_phase;
  uint32_t initialized = f_empty;
};

/** \brief стат структура характеристик газа */
struct gas_char {
  /** \brief газ является ароматическим соединением */
  static bool IsAromatic(gas_t gas);
  /** \brief газ является углеводородом */
  static bool IsHydrocarbon(gas_t gas);
  /** \brief газ является циклопарафином */
  static bool IsCycleParafine(gas_t gas);

  static inline bool IsHydrogenSulfide(gas_t gas) {
    return (gas == GAS_TYPE_HYDROGEN_SULFIDE) ? true : false;
  }
  static inline bool IsCarbonDioxide(gas_t gas) {
    return (gas == GAS_TYPE_CARBON_DIOXIDE) ? true : false;
  }
  static inline bool IsAcetylen(gas_t gas) {
  #ifdef ASSIGNMENT_TRACE_COMPONENTS
    return (gas == GAS_TYPE_ACETYLENE) ? true : false;
  #else
    return false;
  #endif  // ASSIGNMENT_TRACE_COMPONENTS
  }
  static inline bool IsCarbonMonoxide(gas_t gas) {
    return (gas == GAS_TYPE_CARBON_MONOXIDE) ? true : false;
  }

private:
  static bool is_in(gas_t g, const std::vector<gas_t> &v);
};

#endif  // !_CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
