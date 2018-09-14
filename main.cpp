#include <iostream>
#include "gas_description.h"
#include "model_redlich_kwong.h"
#include "phase_diagram.h"
#include "models_creator.h"
// #include "core/common/models_output.h"
// #include "dynamic_modeling.h"
// #include "inputdata_by_file.h"
#include "models_errors.h"
// #include "load_config.h"

/// gas example input
///  gas   |vol,m^3/kg| p,Pa*10^7 |  T,K  |molmass,kg/mol| adiabat,1| cv, J/(kg*K)| acentric factor,1
/// methan  0.00617     4.641       190.66     16.043       ~1.3      ~1750           0.011
/// ethane  0.004926    4.871       305.33     30.07        ~1.22     ~1750           0.089
/// propane 0.004545    4.255       369.9      44.097       ~1.13     ~1750           0.153

int main() {
  /*using namespace real_gas_models;
  try {
    SetConfigure::setConf();
  }
  catch (std::exception &e) {
    std::cout << e.what() <<std::endl;
    return 1;
  }*/
 // std::shared_ptr<constgasparameters> cgp (constgasparameters::getInstance(0.00617,4641000.0,190.66,16.04,1.3,1700,0.01));

//  std::shared_ptr<Equation_of_state> PengRobinModel(new Peng_Robinson_equation());
//  std::shared_ptr<Equation_of_state> RedKwModel(new Redlich_Kwong_equation());
//  std::shared_ptr<Equation_of_state> IdealGas(new Ideal_gas_equation());
//  auto PR = std::shared_ptr<modelGeneral>(PengRobinModel->getCalculatingModel(modelName::REDLICH_KWONG2,cgp));
//  auto RK = std::shared_ptr<modelGeneral>(RedKwModel->getCalculatingModel(modelName::REDLICH_KWONG2,cgp));
//  auto Ig = std::shared_ptr<modelGeneral>(IdealGas->getCalculatingModel(modelName::PENG_ROBINSON,cgp));

//  PR->setVolume(200000,253);
//  RK->setVolume(200000,253);
//  Ig->setVolume(200000,253);

//  std::cout << PR->getVolume()<<std::endl;
//  std::cout << RK->getVolume()<<std::endl;
//  std::cout << Ig->getVolume()<<std::endl;

//  gasparameters inflow(0.0,25000000,293,cgp);

// GasDynamic gd;

//  balloonFlowDynamic* flowdynP=gd.getFlowCalculator(PR,inflow,28.872,0.000785);
//  balloonFlowDynamic* flowdynR=gd.getFlowCalculator(RK,inflow,28.872,0.000785);
//  balloonFlowDynamic* flowdynI=gd.getFlowCalculator(Ig,inflow,28.872,0.000785);

//  formatted_output SOout(std::cout);
//  flowdynP->calculateInflow(0.1,100,SOout);
//  flowdynR->calculateInflow(0.1,100,SOout);
//  flowdynI->calculateInflow(0.1,100,SOout);

/*  std::shared_ptr<InputData> idpptr;

  try {
    idpptr = std::make_shared<InputData>("inputdata.txt");
    gd.calculation(idpptr);
  } catch (modelExceptions &e) {
    std::cout << " Input data error: " << e.what()<<std::endl;
  }*/
  return 0;
}
