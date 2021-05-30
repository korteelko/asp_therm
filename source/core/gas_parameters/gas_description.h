/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_

#include "asp_utils/Common.h"
#include "asp_utils/ErrorWrap.h"
#include "atherm_common.h"
#include "gas_defines.h"

#include <exception>
#include <map>
#include <string>
#include <utility>
#include <vector>

#include <stdint.h>

/* todo: remove it */
gas_t gas_by_name(const std::string& name);

inline double volume_by_compress(double p, double t, double mol, double z) {
  return z * 1000.0 * GAS_CONSTANT * t / (p * mol);
}

inline double compress_by_volume(double p, double t, double mol, double v) {
  // рассчитываются по разному,
  // например для СПГ(ГОСТ 56851), через фактор Пинцера
  return v * p * mol / (1000.0 * GAS_CONSTANT * t);
}

/**
 * \brief Класс исключений инициализации или обработки
 *   газовых параметров
 * */
class gparameters_exception : public std::exception {
 public:
  gparameters_exception(const std::string& msg);
  gparameters_exception(merror_t error, const std::string& msg);

  const char* what() const noexcept;

 private:
  /**
   * \brief Код ошибки
   * */
  merror_t error_;
  /**
   * \brief Сообщение ошибки
   * */
  std::string msg_;
};

/**
 * \brief Динамические параметры вещества, зависящие от
 *   других его параметров
 * */
struct dyn_parameters {
  mstatus_t status;
  dyn_setup setup;
  /// Удельная изохорная теплоёмкость, cv
  double heat_cap_vol,
      /// Удельная изобарная теплоёмкость, cp
      heat_cap_pres,
      /// Внутреняя энергия
      internal_energy,
      /// Энтальпия
      enthalpy,
      /// Показатель адиабаты
      adiabatic,
      /**
       * \brief Коэффициент бета, максимаьлной скорости истечения смеси
       * \note
       * beta_kr=beta_kr(adiabatic_index)
       *   = [0.0, ... ,1.0]
       *   if (pressure_1/pressure_2 < beta_kr):
       *     flow velocity = const, (maximum)
       *   else : flow velocity ~ p,
       *     adiabatic_index and etc
       *   P.S. look dynamic_modeling*.*
       * */
      beta_kr,
      /// Энтропия
      entropy;
  /**
   * \brief Соответствующие макропараметры
   * */
  parameters parm;
  // in future)
  // struct therm_potentials potentials;

 private:
  dyn_parameters(dyn_setup setup,
                 double cv,
                 double cp,
                 double int_eng,
                 parameters pm);
  void check_setup();

  static merror_t check_input(parameters pm);

 public:
  /**
   * \brief Инициализировать структуру, проверив входные параметры
   * */
  static dyn_parameters* Init(dyn_setup setup,
                              double cv,
                              double cp,
                              double int_eng,
                              parameters pm);
  /**
   * \brief Инициализировать пустую структуру(заглушка для газовых смесей).
   *   Инициализировать после можно методом ResetParameters
   * */
  dyn_parameters();
  /**
   * \brief Инициализировать структуру по словарю параметров,
   *   проверив входные параметры
   * \param pm Макропараметры точки
   * \param params Словарь параметров, ключи - целочисленный дефайн,
   *   параметры - значения параметров
   * \todo Доделать, энтальпию например
   * */
  merror_t ResetParameters(parameters pm,
                           const std::map<dyn_setup, double>& params);
  void Update();
};

/* так хочется успеть сделать, но мало времени */
/*struct therm_potentials {
  double  // internalenergy,
         Hermholtz_free,
         enthalpy,
         Gibbsfree,
         LandauGrand,
         // entropy not potential but calculating in dynamic have sense
         entropy;
}; */

/**
 * \brief Параметры газа(или смеси) зависящие от его молярной массы
 * */
struct molar_parameters {
  /**
   * \brief Молярная масса смеси
   * */
  double mass,
      /**
       * \brief Газовая постоянная смеси (Rm = R / m),
       *   где R - универсальная газовая постоянная,
       *       m - молярная масса
       * */
      Rm;
};

/// параметры газа, зависящие от его физической природы и
///   не изменяющиеся при изменении его состояния
struct const_parameters {
  const gas_t gas_name;
  /**
   * \brief Общие параметры газа в критической точке
   * */
  const parameters critical;
  /**
   * \brief Значение коэффициента сжимаемости в критической точке
   * */
  const double Z_K,
      /**
       * \brief Значение фактора ацентричности
       * */
      acentricfactor;
  /**
   * \brief Данные привязанные к значению молярной массы
   * */
  molar_parameters mp;

 private:
  const_parameters(gas_t gas_name,
                   double vk,
                   double pk,
                   double tk,
                   double zk,
                   double mol,
                   double R,
                   double af);

 public:
  const_parameters& operator=(const const_parameters&) = delete;
  /**
   * \brief конструктор для газовых смесей
   *
   * \param pseudo_critic Пседокритические параметры смеси
   * \param zk Значение фактора сжимаемости
   * \param md Молекулярная масса и газовая постоянная смеси
   * \param ??? af Значение фактора ацентричности
   *
   * \throw gparameters_exception
   *
   * \todo Не совсем ясно про  фактор ацентричности
   *   в контексте их использоапния для смесей.
   *   UPD: в ГОСТ на СПГ(56851) предлагаю фактор Пинцера для смеси
   *   считать как среднее арифмитическое.
   *
   * gas_name = GAS_TYPE_MIX
   * */
  const_parameters(parameters pc, double zk, molar_parameters md);
  /**
   * \brief Проверить параметры компонента
   *
   * \param vk Удельный объём в критической точке
   * \param pk Давление в критической точке
   * \param tk Температура в критической точке
   * \param zk Фактор сжимаемость в критической точке
   * \param mol молекулярная масса
   * \param af Фактор ацентричности
   * \return true Для допустимых входных данных
   *
   * \note Просто проверяем что все данные больше нуля
   *   за исключением:
   *   -- фактора ацентричности - может быть меньше 0.0
   *   -- vk и zk - инициализирован должен быть только один из них
   * */
  static inline bool check_params(double vk,
                                  double pk,
                                  double tk,
                                  double zk,
                                  double mol,
                                  double af) {
    return (std::isfinite(af)) ? check_params(vk, pk, tk, zk, mol) : false;
  }
  /**
   * \brief Проверить параметры компонента
   *
   * \note Копия check_params без фактора ацентричности, для смесей
   * */
  static bool check_params(double vk,
                           double pk,
                           double tk,
                           double zk,
                           double mol);

  const_parameters(const const_parameters& cgp);

  static const_parameters* Init(gas_t gas_name,
                                double vk,
                                double pk,
                                double tk,
                                double zk,
                                double mol,
                                double af);
  /**
   * \brief Параметры инициализированы для газовой смеси
   * */
  bool IsGasmix() const;
  /**
   * \brief Параметры инициализированы для неопределённого газа
   * */
  bool IsAbstractGas() const;
  /**
   * \brief Получить строковое представление данных
   * \param pref Префикс перед новой строкой, для соблюдения отступов
   *
   * \return Строковое представление структуры
   * */
  std::string GetString(std::string pref = "") const;
};

bool is_valid_cgp(const const_parameters& cgp);
bool is_valid_dgp(const dyn_parameters& dgp);

/*
 * mixes defines
 * Смесь газов это по сути дефолтная субстанция, с коей
 *   имеем дело, но всё же матаппарат работы с чистым
 *   вецеством скорее всего пригодится
 */
class modelGeneral;
struct model_str;
typedef std::pair<const_parameters, dyn_parameters> const_dyn_parameters;
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
 * \todo
 * 1. От union ибвиться, заменить на variant<...>
 * 2. Сами данные обернуть в структуру с get операциями
 * */
union const_dyn_union {
  const parameters_mix* components;
  const ng_gost_mix* ng_gost_components;
  struct {
    const const_parameters* cgp;
    const dyn_parameters* dgp;
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
  bool operator<(const gas_pair& rhs) const;
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
   *
   * \todo Как насчёт сюда тоже мапу прокидывать
   * */
  calculation_state_log& SetDynPars(const dyn_parameters& dp);
  /**
   * \brief Установить указатель на структуру расчётов
   * \param ci Указатель на структуру информации о расчёте
   * */
  calculation_state_log& SetCalculationInfo(calculation_info* ci);

 public:
  enum state_info_flags {
    f_empty = 0x00,
    /** \brief Уникальный id информации по расчёту */
    f_calculation_info_id = 0x01,
    /** \brief Объём */
    f_vol = 0x02,
    /** \brief Давление */
    f_pres = 0x04,
    /** \brief Температура */
    f_temp = 0x08,
    /** \brief Изохорная теплоёмкость */
    f_dcv = 0x10,
    /** \brief Изобарная теплоёмкость */
    f_dcp = 0x20,
    /** \brief Внутреняя энергия */
    f_din = 0x40,
    /** \brief Энтальпия */
    f_denthalpy = 0x80,
    /** \brief Показатель адиабатты */
    f_dadiabatic = 0x100,
    /** \brief Параметр Bk */
    f_dbk = 0x200,
    /** \brief Энтропия */
    f_dentropy = 0x400,
    /** \brief Фазовое состояние */
    f_state_phase = 0x800,
    /** \brief Уникальный id строки расчёта */
    f_calculation_state_log_id = 0x1000,
    f_full = 0x1FFF
  };

  int32_t id;
  int32_t info_id;
  const calculation_info* calculation = nullptr;

  dyn_parameters dyn_pars;
  std::string state_phase;
  uint32_t initialized = f_empty;
};

/**
 * \brief Стат структура характеристик газа
 * */
struct gas_char {
  /**
   * \brief Газ является ароматическим соединением
   * */
  static bool IsAromatic(gas_t gas);
  /**
   * \brief Газ является углеводородом
   * */
  static bool IsHydrocarbon(gas_t gas);
  /**
   * \brief Газ является циклопарафином
   * */
  static bool IsCycleParafine(gas_t gas);
  /**
   * \brief Благородный газ
   * */
  static bool IsNoble(gas_t gas);

  static inline bool IsHydrogenSulfide(gas_t gas) {
    return (gas == GAS_TYPE_HYDROGEN_SULFIDE) ? true : false;
  }
  static inline bool IsCarbonDioxide(gas_t gas) {
    return (gas == GAS_TYPE_CARBON_DIOXIDE) ? true : false;
  }
  static inline bool IsAcetylene(gas_t gas) {
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
  static bool is_in(gas_t g, const std::vector<gas_t>& v);
};

#endif  // !_CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
