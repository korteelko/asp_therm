#include "test_models_mix.h"

#include "filereader.h"
#include "xmlreader.h"
#include "model_redlich_kwong.h"

#include <vector>

#include <assert.h>

const std::string xml_path = "/../../asp_therm/data/gases/";
const std::string xml_methane = "methane.xml";
const std::string xml_ethane = "ethane.xml";


int test_models_mix() {
  assert(0 && "test_models_mix");
  std::vector<gas_mix_file> xml_files{
    {(xml_path + xml_methane).c_str(), 0.95},
    // add more (summ = 1.00)
    {(xml_path + xml_ethane).c_str(), 0.05}
  };
  char cwd[512] = {0};
  if (getcwd(cwd, (sizeof(cwd))))
    methane_path = std::string(cwd) + methane_path;
  else
    std::cerr << "cann't get current dir";
  std::unique_ptr<XmlFile> met_xml(XmlFile::Init(methane_path));
  if (met_xml == nullptr) {
    std::cerr << "Object of XmlFile wasn't created\n";
    return 1;
  }
}
