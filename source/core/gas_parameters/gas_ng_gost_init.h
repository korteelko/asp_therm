#ifndef _CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_
#define _CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_

#include "gas_description_static.h"
#include "models_errors.h"

#include <vector>

// размерности, константы, параметры при НФУ см.
//   в ГОСТ 30319.1-2015 (!!! коэффициенты в третьем(30319.3) а константы в первом)

struct ng_gost_params {
  double A0,
         A1,
         A2,
         A3;
  double z,     // фактор сжимаемости
         k,     // показатель адиабаты
         u,     // скорорсть звука
         cp0r,  // изобарная теплоёмкость (в идеальном состоянии)
         mu;    // динамическая вязкость
};

// const_dyn_parameters init_natural_gas(const gost_ng_components &comps);
class GasParameters_NG_Gost_dyn : public GasParameters {
  merror_t error_status_;
  ng_gost_mix components_;
  parameters pseudocrit_vpte_;
  ng_gost_params ng_gost_params_;

  double ng_molar_mass_;
  double coef_kx_;
  double coef_V_,
         coef_Q_,
         coef_F_,
         coef_G_,
         coef_p0m_;
  std::vector<double> Bn_;
  std::vector<double> Cn_;

private:
  GasParameters_NG_Gost_dyn(parameters prs, const_parameters cgp,
      dyn_parameters dgp, ng_gost_mix components);
  // init methods
  //   call 1 time in ctor
  void set_V();
  void set_Q();
  void set_F();
  void set_G();
  void set_Bn();
  void set_Cn();
  merror_t set_molar_mass();
  void set_p0m();
  merror_t init_kx();
  merror_t init_pseudocrit_vpte();
  // init methods end
  double get_Dn(size_t n) const;
  double get_Un(size_t n) const;
  // calculating sigma(it is volume)
  merror_t set_volume();
  merror_t set_cp0r();
  double sigma_start() const;
  double calculate_d_sigm(double sigm) const;
  double calculate_A0(double sigm) const;
  double calculate_A1(double sigm) const;
  double calculate_A2(double sigm) const;
  double calculate_A3(double sigm) const;

public:
  static GasParameters_NG_Gost_dyn *Init(gas_params_input gpi);
  void csetParameters(double v, double p, double t, state_phase);
}; 

#endif  // !_CORE__GAS_PARAMETERS__GAS_NG_GOST_INIT_H_
