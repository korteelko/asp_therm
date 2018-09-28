#include "test_models_mix.h"

#include "target_sys.h"

#include "filereader.h"
#include "xmlreader.h"
#include "model_redlich_kwong.h"

#include <vector>

#include <assert.h>

#if defined (_OS_NIX)
#  include <unistd.h>
#elif defined(_OS_WIN)
#  include <direct.h>
#  define getcwd _getcwd
#endif  // _OS

const std::string xml_path = "/../../asp_therm/data/gases/";
const std::string xml_methane = "methane.xml";
const std::string xml_ethane = "ethane.xml";
const std::string xml_propane = "propane.xml";

/*
static int getcwd_cross(char *src, int src_len) {
  assert(0);
#ifdef _OS_NIX
  return getcwd(src, src_len);
#elif _OS_WIN
  
#endif 
}
*/

int test_models() {
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::string filename = std::string(cwd) + xml_path + xml_methane;
  std::unique_ptr<XmlFile> met_xml(XmlFile::Init(filename));
  if (met_xml == nullptr) {
    std::cerr << "Object of XmlFile wasn't created\n";
    return 1;
  }
  auto cp = met_xml->GetConstParameters();
  auto dp = met_xml->GetDynParameters();
  if (cp == nullptr) {
    std::cerr << "const_parameters by xml wasn't created\n";
    return 2;
  }
  if (dp == nullptr) {
    std::cerr << "dyn_parameters by xml wasn't created\n";
    return 2;
  }
  PhaseDiagram &pd = PhaseDiagram::GetCalculated();
  binodalpoints bp = pd.GetBinodalPoints(cp->V_K, cp->P_K, cp-> T_K,
      modelName::REDLICH_KWONG2, cp->acentricfactor);
  Redlich_Kwong2 *rk2 = Redlich_Kwong2::Init(modelName::REDLICH_KWONG2,
      {1.42, 100000, 275}, *cp, *dp, bp);
  std::cerr << rk2->ParametersString() << std::flush;
  std::cerr << "Set volume(10e5, 314)\n" << std::flush;
  rk2->SetVolume(100000, 314);
  std::cerr << rk2->ConstParametersString() << std::flush;
  std::cerr << modelGeneral::sParametersStringHead() << std::flush;
  std::cerr << rk2->ParametersString() << std::flush;
//  std::cerr << "\n vol  " << rk2->GetVolume(25000000, 365.85) <<
//               "\n pres " << rk2->GetPressure(0.007267, 365.85) << std::flush;
  return 0;
}

int test_models_mix() {
  // assert(0 && "test_models_mix");
  char cwd[512] = {0};
  if (!getcwd(cwd, (sizeof(cwd)))) {
    std::cerr << "cann't get current dir";
    return 1;
  }
  std::vector<gas_mix_file> xml_files{
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
  GasMix *gm = GasMix::Init(xml_files);
  if (!gm) {
    std::cerr << "GasMix init error" << std::flush;
    return 1;
  }
  std::shared_ptr<parameters_mix> prs_mix = gm->GetParameters();
  if (!prs_mix) {
    std::cerr << "GasMix prs_mix error" << std::flush;
    return 1;
  }
  PhaseDiagram &pd = PhaseDiagram::GetCalculated();
  binodalpoints bp = pd.GetBinodalPoints(*prs_mix, modelName::REDLICH_KWONG2);
  Redlich_Kwong2 *rk2 = Redlich_Kwong2::Init(modelName::REDLICH_KWONG2,
      {1.42, 100000, 275}, *prs_mix, bp);
  std::cerr << rk2->ConstParametersString() << std::flush;
  std::cerr << modelGeneral::sParametersStringHead() << std::flush;
  std::cerr << rk2->ParametersString() << std::flush;
  rk2->SetVolume(100000, 314);
  std::cerr << "Set volume(10e5, 314)\n" << std::flush;
  std::cerr << rk2->ParametersString() << std::flush;
  return 0;
}
