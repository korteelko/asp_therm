#ifndef _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_DYNAMIC_H_
#define _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_DYNAMIC_H_

#include "gas_description.h"
#include "gas_description_static.h"

#include <iostream>

/*
 * PARAMETERS OF GAS IN DYNAMIC.
 *
 * Параметры газа в динамике.
 *   Основная структура данных
 * По сути это совокупность других структур
 *   с элементами ООП
*/
/// DEVELOP
///   Классификатор final классная штука:
///     -- не даёт наследываться
///     -- компилятор лучше обрабатывает перегруженные функции

class GasParameters_dyn final: public GasParameters {
  GasParameters_dyn(parameters prs,
      const_parameters cgp, dyn_parameters dgp, modelGeneral *mg);

public:
  static GasParameters_dyn *Init(double v, double p, double t,
      const_parameters cgp, dyn_parameters dgp, modelGeneral *mg);

  static GasParameters_dyn *Init(parameters prs,
      const_parameters cgp, dyn_parameters dgp, modelGeneral *mg);

  void csetParameters(double v,
      double p, double t, state_phase sp) override;

private:
  // previous pressure, volume and temperature
  parameters        prev_vpte_;
  // pointer to used model(equation of state)
  // DEVELOP:
  //   После недель и месяцов раздумий, помимо отказа от 
  //     статичных параметров газа было решено ввести ещё и 
  //     жесткую связь модели с параметрами газа.
  //   P.S. 
  //     Hа самом деле здесь "model_" - weak_ptr<ModelGeneral>
  //     model is a weak pointer(in c++11 context) to model
  modelGeneral      *model_;

  // function for update dyn_parameters
  // dyn_params_update update_f_;
  // potentials      potentials_;
  
};

std::ostream &operator<< (std::ostream &outstream,
    const GasParameters_dyn &gp);

#endif  // ! _CORE__GAS_PARAMETERS__GAS_DESCRIPTION_DYNAMIC_H_
