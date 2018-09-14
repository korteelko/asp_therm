#include "test_xml.h"

// #include
#include "filereader.h"
#include "xmlreader.h"

#include <iostream>
#include <memory>
#include <string>

#include <assert.h>
#include <stdio.h>
//#ifdef _NIX
  #include <unistd.h>
//#endif  // _NIX

const std::string xml_path = "/../../asp_therm/data/gases/";
const std::string xml_methane = "methane.xml";

int run_tests_xml() {
  std::string methane_path = xml_path + xml_methane;
  char cwd[512] = {0};
  if (getcwd(cwd, (sizeof(cwd))))
    methane_path = std::string(cwd) + methane_path;
  else
    std::cerr << "cann't get current dir";
  // std::unique_ptr<XMLReader> xml_doc(XMLReader::Init(methane_path));
  std::unique_ptr<XmlFile> met_xml(XmlFile::Init(methane_path));
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
  return 0;
}
