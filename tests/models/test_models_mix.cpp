/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "test_models_mix.h"

#include "target_sys.h"

#include "gas_by_file.h"
#include "xml_reader.h"
#include "model_redlich_kwong.h"
#include "model_peng_robinson.h"
#include "models_creator.h"

#include <memory.h>
#include <vector>

#include <assert.h>

#if defined (_OS_NIX)
#  include <unistd.h>
#elif defined(_OS_WIN)
#  include <direct.h>
#  define getcwd _getcwd
#endif  // _OS

// #define RK2_TEST
#define PR_TEST
// #define GOST_TEST

#ifdef _IDE_VSCODE
const std::string xml_path = "/../asp_therm/data/gases/";
#else
const std::string xml_path = "/../../asp_therm/data/gases/";
#endif  // IDE_VSCODE
const std::string xml_methane = "methane.xml";
const std::string xml_ethane = "ethane.xml";
const std::string xml_propane = "propane.xml";

int test_models() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::string filename = std::string(cwd) + xml_path + xml_methane;
#if defined(RK2_TEST)
  Redlich_Kwong2 *calc_mod = Redlich_Kwong2::Init(modelName::REDLICH_KWONG2,
      {1.42, 100000, 275}, *cp, *dp, bp);
#elif defined(PR_TEST)
  Peng_Robinson *calc_mod = dynamic_cast<Peng_Robinson *>(
      ModelsCreator::GetCalculatingModel(rg_model_t::PENG_ROBINSON,
      std::vector<gasmix_file>{{filename, 1.0}},
      100000, 275));
#endif  // _TEST
  std::cerr << calc_mod->ParametersString() << std::flush;
  std::cerr << "Set volume(10e6, 314)\n" << std::flush;
  calc_mod->SetVolume(1000000, 314);
  std::cerr << calc_mod->ConstParametersString() << std::flush;
  std::cerr << modelGeneral::sParametersStringHead() << std::flush;
  std::cerr << calc_mod->ParametersString() << std::flush;
//  std::cerr << "\n vol  " << calc_mod->GetVolume(25000000, 365.85) <<
//               "\n pres " << calc_mod->GetPressure(0.007267, 365.85) << std::flush;
  return 0;
}

int test_models_mix() {
  // assert(0 && "test_models_mix");
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::vector<gasmix_file> xml_files{
    {(std::string(cwd) + xml_path + xml_methane).c_str(), 0.988},
    // add more (summ = 1.00)
    {(std::string(cwd) + xml_path + xml_ethane).c_str(), 0.009},
    {(std::string(cwd) + xml_path + xml_propane).c_str(), 0.003}
  };
  /*
  std::unique_ptr<XmlFile> met_xml(XmlFile::Init(methane_path));
  if (met_xml == nullptr) {
    std::cerr << "Object of XmlFile wasn't created\n";
    return 1;
  }
  */
  /*std::unique_ptr<GasMix> gm(GasMix::Init(xml_files));
  if (gm == nullptr) {
    std::cerr << "GasMix init error" << std::flush;
    return 1;
  }
  std::shared_ptr<parameters_mix> prs_mix = gm->GetParameters();
  if (!prs_mix) {
    std::cerr << "GasMix prs_mix error" << std::flush;
    return 1;
  }
  PhaseDiagram &pd = PhaseDiagram::GetCalculated();
  binodalpoints bp = pd.GetBinodalPoints(*prs_mix, rg_model_t::PENG_ROBINSON);
  */
#if defined(RK2_TEST)
  Redlich_Kwong2 *calc_mod = Redlich_Kwong2::Init(modelName::REDLICH_KWONG2,
      {1.42, 100000, 275}, *prs_mix, bp);
#elif defined(PR_TEST)
  //Peng_Robinson *calc_mod = Peng_Robinson::Init(set_input(rg_model_t::PENG_ROBINSON, bp,
  //    100000, 275, *prs_mix));
  Peng_Robinson *calc_mod = dynamic_cast<Peng_Robinson *>(
      ModelsCreator::GetCalculatingModel(rg_model_t::PENG_ROBINSON,
      xml_files, 100000, 275));
#endif  // _TEST
  std::cerr << calc_mod->ConstParametersString() << std::flush;
  std::cerr << modelGeneral::sParametersStringHead() << std::flush;
  std::cerr << calc_mod->ParametersString() << std::flush;
  calc_mod->SetVolume(1000000, 314);
  std::cerr << "Set volume(10e6, 314)\n" << std::flush;
  std::cerr << calc_mod->ParametersString() << std::flush;
  return 0;
}
