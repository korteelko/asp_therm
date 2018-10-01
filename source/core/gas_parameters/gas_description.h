#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_

// #include "target_sys.h"
// #include "common.h"

#include <array>
#include <string>

#include <stdint.h>

/*
 * Вводная часть проекта.
 *
 * Добро пожаловать!
 * Приветствие размещенно здесь не случайно, т.к. этот файл можно
 *   считать логическим началом проекта на который было потрачено много
 *   сил и времени, и безмерно приятно если это читает кто-либо помимо
 *   меня. Внесение посильных изменений, исправление багов, разговор по
 *   душам с разработчиком -- приветствуются. Вот его email:
 *      korteelko@gmail.com
 *
 * Итак прежде всего читателю:
 *   1) С++11  -- удобно, объектною.Сложные структуры данных пишем на нём
 *   2) Pure C -- часто более понятно разработчику.
 *     Не стоит всякую мелочь оборачивать в кучу оберток, проверок-перепроверок
 *     и прочего.
 *     Не стоит нашивать на логичные struct C в их изначальном смысле
 *     простого блока памяти С++ возможности. 'С' прекрасен сам по себе.
 *   3) Архитектура проекта. Разбито с точки зрения теплофизика, не программиста
 *   4) Напишите это сами.
 *
 *
 *   UPD:  комментарий с меткой "DEVELOP" предложение к разработчику
 *   UPD2: комментировать можно даже комментарии и это прелестно
*/

/*
 * ФАЙЛ БАЗОВЫХ СТРУКТУР ПРОЕКТА
 */

// ================================================================
// state_phase enum || stateToString
// ================================================================
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

// ================================================================
// parameters struct
// ================================================================
/// Общие параметры состояния вещества,
///   для описания его текущего состояния с размерностями
/// Common parameters of substance for describing
///   current state with dimensions
struct parameters {
  double  volume,               // m^3 / kg
          pressure,             // Pa
          temperature;          // K
};

// ================================================================
// dyn_parameters struct
// ================================================================
/// Динамические параметры вещества, зависящие от
///   других его параметров
/// Dynamic parameters of substance
/// DEVELOP
///   По структурам dyn_- и const_parameters
///     идея закрыть все уонструкторы кроме копирующих
///     Инит методы обеспечат корректное создание, а далее
///     изменение полей возлагается на методы мспользуемой модели
struct dyn_parameters {
 // adiabatic_index,  // Cp/Cv
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
  // закроем все конструкторы
  // close all constructors
  dyn_parameters(double cv, double cp, double int_eng, parameters pm);

public:
  static dyn_parameters *Init(double cv, double cp, double int_eng,
      parameters pm);
  // обновить критический параметр истечения
  void Update();
};

class modelGeneral;
/// Указатель на функцию обновления динамических параметров
///   (зависит от используемой модели реального газа)
/// Pointer to dynaparameter updating function
///   (depended by real gas model)
//typedef void(*dyn_params_update)(dyn_parameters &prev_state,
//  const parameters new_state);
// вторая версия функции с жесткой связью на модель
// typedef void (modelGeneral::*dyn_params_update)(
//    dyn_parameters &prev_state, const parameters new_state);

// ================================================================
// potentials struct
// ================================================================
/// Потенциалы сложная(м.б. и не очень) часть
///   часть ДИНАМИКИ системы
/// На данный момент в планах нет задач задействующих её
/*
struct potentials {
  double  // internalenergy,
         Hermholtz_free,
         enthalpy,
         Gibbsfree,
         LandauGrand,
         // entropy not potential but calculating in dynamic have sense
         entropy;
};
*/

// ================================================================
// const_parameters struct
// ================================================================
/// параметры газа, зависящие от его физической природы и
///   не изменяющиеся при изменении его состояния
/// gas paramaters depending on the physics characteristics
struct const_parameters {
  const double V_K,              // K point parameters (critical point)
               P_K,
               T_K,
               molecularmass,
               R,                // gas constant
               acentricfactor;

private:
  const_parameters(double vk, double pk, double tk, double mol,
      double R, double af);

public:
  static const_parameters *Init(double vk, double pk,
      double tk, double mol, double af);
//  static constgasparameters *Init(std::array<double, 5> vec);
  const_parameters(const const_parameters &cgp);
  const_parameters &operator= (const const_parameters &cgp);
};

// ================================================================
// check data functions
// ================================================================
/// Функции проверки валидности данных
bool is_valid_cgp(const const_parameters &cgp);
bool is_valid_dgp(const dyn_parameters &dgp);


//================================
// IGas_parameters interface
//================================
/// Интерфейс доступа к динамическим и статическим парамертам
/* class IGas_parameters {
public:
  virtual double cgetV_K()            const = 0;
  virtual double cgetP_K()            const = 0;
  virtual double cgetT_K()            const = 0;
  virtual double cgetMolecularMass()  const = 0;
  virtual double cgetAdiabatic()      const = 0;
  virtual double cgetCV()             const = 0;
  virtual double cgetBeta()           const = 0;
  virtual double cgetR()              const = 0;
  virtual double cgetAcentricFactor() const = 0;
  virtual double cgetVolume()         const = 0;
  virtual double cgetPressure()       const = 0;
  virtual double cgetTemperature()    const = 0;
  virtual parameters cgetParameters() const = 0;
  virtual state_phase cgetState()     const = 0;
  virtual const_parameters cgetConstparameters() const = 0;

  virtual void csetParameters(double, double, double, state_phase) = 0;
  virtual ~Igasparameters() {}
};
*/
#endif  // ! _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_H_
