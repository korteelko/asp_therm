/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _CORE__SUBROUTINS__GAS_BY_FILE_H_
#define _CORE__SUBROUTINS__GAS_BY_FILE_H_

#include "atherm_common.h"
#include "file_structs.h"
#include "gas_description.h"
#include "gasmix_init.h"
#include "models_math.h"

#include <exception>
#include <memory>
#include <string>
#include <vector>

#define XML_PATHLEN_SUBGROUP 3
#define PNT_PARAMS_COUNT 3
#define CRIT_PNT_PARAMS_COUNT 4
#define DYN_PARAMS_COUNT 3

#ifdef HIDDEN_CODE
/**
 * \brief Исключения парсинга файлов
 * */
class file_parse_exception : public std::exception {
 public:
  file_parse_exception(std::string* source_str_p, const std::string& _msg);

  const char* what() const noexcept override;

 private:
  /**
   * \brief Строка которую не смогли распарсить
   * */
  std::string source_str;
  /**
   * \brief Доп. информация
   * */
  std::string msg;
};
#endif  // HIDDEN_CODE

/**
 * \brief Шаблоный класс инициализации компонента
 *   газовой смеси, описанного в xml или json файле
 * \note Интересно, можно ли бы было спроектировать его ещё хуже.
 *   Ошибка была в предположении, что данные файла сводимы к
 *   строгой 3х-ступенчатой структуре. Но в итоге, такой подход
 *   рушит логику построения некоторых компонентов, а следовательно и
 *   логику их обработки/инициализации.
 * \todo В общем, переделать под класс ReaderSample<>
 * */
template <template <class gas_node> class ConfigReader>
class ComponentByFile {
  ComponentByFile(const ComponentByFile&) = delete;
  ComponentByFile& operator=(const ComponentByFile&) = delete;

 private:
  // standard paths
  static std::vector<std::string> critical_point_path;
  static std::vector<std::string> const_parameters_path;
  static std::vector<std::string> dynamic_point_path;
  static std::vector<std::string> dynamic_capacities_path;
  static std::vector<std::string> dynamic_potentials_path;

  static const std::string point_names[CRIT_PNT_PARAMS_COUNT];

 public:
  static ComponentByFile* Init(const std::string& filename) {
    ConfigReader<gas_node>* config_doc = ConfigReader<gas_node>::Init(filename);
    if (config_doc == nullptr)
      return nullptr;
    return new ComponentByFile(config_doc);
  }

  std::shared_ptr<const_parameters> GetConstParameters() {
    std::shared_ptr<const_parameters> result = nullptr;
    if (config_doc_) {
      // critical point parameters
      double cp[CRIT_PNT_PARAMS_COUNT];
      double mol, af;
      std::vector<std::string> tmp_vec(critical_point_path);
      tmp_vec.push_back("");
      if (initCriticalPoint(tmp_vec, cp)) {
        tmp_vec[XML_PATHLEN_SUBGROUP - 1] =
            const_parameters_path[XML_PATHLEN_SUBGROUP - 1];

        if (!set_double(get_val(tmp_vec, "molec_mass"), &mol)
            && !set_double(get_val(tmp_vec, "acentric"), &af)) {
          set_gas_name();
          result.reset(const_parameters::Init(gas_name_, cp[0], cp[1], cp[2],
                                              cp[3], mol, af));
        }
      }
    }
    return result;
  }

  std::shared_ptr<dyn_parameters> GetDynParameters() {
    std::shared_ptr<dyn_parameters> result = nullptr;
    if (config_doc_) {
      // dynamic point parameters
      double pnt[PNT_PARAMS_COUNT] = {0.0};
      std::vector<std::string> tmp_vec(dynamic_point_path);
      tmp_vec.push_back("");
      if (initPoint(tmp_vec, pnt)) {
        /* set calculating dynamic parameters */
        double dyn[DYN_PARAMS_COUNT] = {0.0};
        dyn_setup ds = initDynSetup(tmp_vec, dyn);
        result.reset(dyn_parameters::Init(ds, dyn[0], dyn[1], dyn[2],
                                          parameters{pnt[0], pnt[1], pnt[2]}));
      }
    }
    return result;
  }

 private:
  ComponentByFile(ConfigReader<gas_node>* config_doc)
      : status_(STATUS_DEFAULT),
        config_doc_(config_doc),
        gas_name_(GAS_TYPE_UNDEFINED) {}
  /**
   * \brief Получить строковое представление параметра
   * \param vec Вектор пути к группе параметров
   * \param valname Имя параметра в группе
   * */
  std::string get_val(std::vector<std::string>& vec,
                      const std::string& valname) {
    std::string tmp_str = "";
    vec[XML_PATHLEN_SUBGROUP] = valname;
    config_doc_->GetValueByPath(vec, &tmp_str);
    return tmp_str;
  }
  /**
   * \brief Инициализировать критические параметры точки
   * \param cp Указатель на массив выходных значений
   * */
  bool initCriticalPoint(std::vector<std::string>& vec, double* cp) {
    for (size_t i = 0; i < CRIT_PNT_PARAMS_COUNT; ++i)
      if (set_double(get_val(vec, point_names[i]), &cp[i]))
        cp[i] = 0.0;
    // check pressure and temperature
    bool is_valid = is_above0(cp[1], cp[2]);
    // check volume or compress_factor
    if (!is_above0(cp[0]) && !is_above0(cp[2]))
      is_valid = false;
    return is_valid;
  }
  /**
   * \brief Инициализировать параметры точки
   * \param point Указатель на массив значений параметров
   * */
  bool initPoint(std::vector<std::string>& vec, double* point) {
    for (size_t i = 0; i < PNT_PARAMS_COUNT; ++i)
      if (set_double(get_val(vec, point_names[i]), &point[i]))
        point[i] = 0.0;
    // check pressure and temperature, volume can be equal 0.0
    bool is_valid = is_above0(point[1], point[2]);
    return is_valid;
  }
  /**
   * \brief Инициализировать динамические параметры
   * \param dyn Указатель на массив значений параметров
   * */
  dyn_setup initDynSetup(std::vector<std::string>& vec, double* dyn) {
    dyn_setup ds = 0;
    vec[XML_PATHLEN_SUBGROUP - 1] =
        dynamic_capacities_path[XML_PATHLEN_SUBGROUP - 1];
    if (!set_double(get_val(vec, "heat_cap_vol"), &dyn[0]))
      if (is_above0(dyn[0]))
        ds |= DYNAMIC_HEAT_CAP_VOL;
    if (set_double(get_val(vec, "heat_cap_pres"), &dyn[1]))
      if (is_above0(dyn[1]))
        ds |= DYNAMIC_HEAT_CAP_PRES;
    vec[XML_PATHLEN_SUBGROUP - 1] =
        dynamic_potentials_path[XML_PATHLEN_SUBGROUP - 1];
    if (set_double(get_val(vec, "internal_energy"), &dyn[2]))
      if (is_above0(dyn[2]))
        ds |= DYNAMIC_INTERNAL_ENERGY;
    return ds;
  }

  void set_gas_name() { gas_name_ = gas_by_name(config_doc_->GetRootName()); }

 private:
  mstatus_t status_;
  std::unique_ptr<ConfigReader<gas_node>> config_doc_;
  gas_t gas_name_;
};

/**
 * \brief Путь в файле конфигурации до макропараметров(pvt)
 *   критической точки
 * */
template <template <class gas_node> class ConfigReader>
std::vector<std::string> ComponentByFile<ConfigReader>::critical_point_path{
    "parameters", "constants", "critical_point"};
/** \brief путь в файле конфигурации до неизменяемых параметров компонента */
template <template <class gas_node> class ConfigReader>
std::vector<std::string> ComponentByFile<ConfigReader>::const_parameters_path{
    "parameters", "constants", "const_parameters"};
/** \brief путь в файле конфигурации до макропараметров pvt компонента
 *   при которых расчитаны его динамические значения(параметры ниже) */
template <template <class gas_node> class ConfigReader>
std::vector<std::string> ComponentByFile<ConfigReader>::dynamic_point_path{
    "parameters", "dynamics", "point"};
/** \brief путь в файле конфигурации до теплоёмкости */
template <template <class gas_node> class ConfigReader>
std::vector<std::string> ComponentByFile<ConfigReader>::dynamic_capacities_path{
    "parameters", "dynamics", "capacities"};
/** \brief путь в файле конфигурации до потенциалов */
template <template <class gas_node> class ConfigReader>
std::vector<std::string> ComponentByFile<ConfigReader>::dynamic_potentials_path{
    "parameters", "dynamics", "potentials"};
/**
 * \brief Параметры критической точки
 * */
template <template <class gas_node> class ConfigReader>
const std::string
    ComponentByFile<ConfigReader>::point_names[CRIT_PNT_PARAMS_COUNT] = {
        "volume", "pressure", "temperature", "compress_factor"};

#endif  // !_CORE__SUBROUTINS__GAS_BY_FILE_H_
