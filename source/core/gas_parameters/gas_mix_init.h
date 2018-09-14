#ifndef _CORE__GAS_PARAMETERS__GAS_MIX_INIT_H_
#define _CORE__GAS_PARAMETERS__GAS_MIX_INIT_H_

#include "gas_description.h"
#include "gas_description_static.h"

#include <map>
#include <utility>

/*
 * Файл для инициализации параметров газовых смесей
 *   по параметрам их составляющих(с учетом доли)
*/

// Не имеет слысла определять все составляющие газовой
//   смеси. 97% составляющихбудет достаточно
// И не забудем ошибку вычисления, или округления,
//   отведём ей 1%
// Итого проверяем тождество :
//   |0.99 - ${summ_of_components}| < 0.02
#define GAS_MIX_PERCENT_EPS  0.02

// DEVELOP
//   с typedef'ами здесь перетого переэтого,
//   надо было в структуры завернуть,
//   но так std функции испотльзовать удобно

typedef std::pair<const_parameters, dyn_parameters>
    const_dyn_parameters;
//                молярная доля, % ; параметры доли
typedef std::multimap<const double, const_dyn_parameters> parameters_mix;

/* занесено в класс GasParameters_mix_dyn 
std::pair<const_parameters *, dyn_parameters *>
    get_parameters_of_mix(parameters_mix cgp_mix);
*/


class GasParameters_mix : public GasParameters {
protected:
  parameters_mix components_;

protected:
  GasParameters_mix(parameters prs, const_parameters cgp,
      dyn_parameters dgp, parameters_mix components);

// public:
  virtual ~GasParameters_mix();
};


// =========================================================
// класс описывающий смесь газов (динамика)
// =========================================================
/*
 * Короч, при инициализации базовый класс прописываем тем,
 *   что вытащили в Init методе. Данные которые не можем просчитать
 *   обнуляем (неадитивные данные нужные при инициализации,
 *   которые при расчётах не используются)
*/
class GasParameters_mix_dyn final : public GasParameters_mix {
  // previous pressure, volume and temperature
  parameters         prev_vpte_;
  // function for update dyn_parameters
  modelGeneral      *model_;
  // function for update dyn_parameters
  // dyn_params_update  update_f_;
  // potentials      potentials_;

private:
  GasParameters_mix_dyn(parameters prs, const_parameters cgp,
      dyn_parameters dgp, parameters_mix components,
      modelGeneral *mg);

public:
  static GasParameters_mix_dyn *Init(parameters prs,
      parameters_mix components, modelGeneral *mg);

  // method for calculating binodal points
  //   (see "phase_diagram/")
  /// input : - parameters_mix &components
  ///         - parameters prs
  /// output: - const_gas_parameters
  static std::unique_ptr<const_parameters> 
      GetAverageParams(parameters_mix &components);

  void csetParameters(double v, double p, double t, state_phase sp) override;
};

#endif  // _CORE__GAS_PARAMETERS__GAS_MIX_INIT_H_
