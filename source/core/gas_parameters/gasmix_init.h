#ifndef _CORE__GAS_PARAMETERS__GASMIX_INIT_H_
#define _CORE__GAS_PARAMETERS__GASMIX_INIT_H_

#include "gas_description_static.h"

// Не имеет слысла определять все составляющие газовой
//   смеси. 97% составляющихбудет достаточно
// И не забудем ошибку вычисления, или округления,
//   отведём ей 1%
// Итого проверяем тождество :
//   |0.99 - ${summ_of_components}| < 0.02
#define GASMIX_PERSENT_AVR  0.99
#define GASMIX_PERCENT_EPS  0.02

struct gasmix_file {
  std::string filename;
  double part;

public:
  gasmix_file(const std::string filename, const double part);
};

bool operator< (const gasmix_file &lg, const gasmix_file &rg);

class GasParameters_mix : public GasParameters {
protected:
  parameters_mix components_;

protected:
  GasParameters_mix(parameters prs, const_parameters cgp,
      dyn_parameters dgp, parameters_mix components);
  virtual ~GasParameters_mix();
};

class GasParameters_mix_dyn final : public GasParameters_mix {
  // previous pressure, volume and temperature
  parameters         prev_vpte_;
  // function for update dyn_parameters
  modelGeneral       *model_;

private:
  GasParameters_mix_dyn(parameters prs, const_parameters cgp,
      dyn_parameters dgp, parameters_mix components, modelGeneral *mg);

public:
  static GasParameters_mix_dyn *Init(gas_params_input gpi, modelGeneral *mg);
  //  неправильно, средние параметры зависят от модели
  static std::unique_ptr<const_parameters> 
      GetAverageParams(parameters_mix &components);
  const parameters_mix &GetComponents() const;
  void csetParameters(double v, double p, double t, state_phase sp) override;
};
#endif  // !_CORE__GAS_PARAMETERS__GASMIX_INIT_H_
