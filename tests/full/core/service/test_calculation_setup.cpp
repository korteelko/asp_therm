#include "calculation_setup.h"
#include "Common.h"
#include "ErrorWrap.h"
#include "gas_defines.h"
#include "merror_codes.h"
#include "model_peng_robinson.h"
#include "program_state.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <fstream>
#include <functional>
#include <string>


namespace fs = std::filesystem;

static std::string calc_file = "test_calculation.xml";

static std::string path_gost1 = "../../../data/calculation/gost_test/gostmix1.xml";
static std::string path_gost2 = "../../../data/calculation/gost_test/gostmix2.xml";
static std::string path_gost3 = "../../../data/calculation/gost_test/gostmix3.xml";

/**
 * \brief Класс для тестинга CalculationSetup
 * */
class CalculationSetupProxy {
public:
  CalculationSetupProxy(std::shared_ptr<file_utils::FileURLRoot> &root,
      const std::string &filepath)
    : cs(CalculationSetup(root, filepath)) {}

  CalculationSetup &GetSetup() { return cs; }
  calculation_setup *GetInitData() { return cs.init_data_.get(); }

public:
  CalculationSetup cs;
};

/**
 * \brief Класс тестов сетапа расчётов
 * */
class CalculationSetupTest: public ::testing::Test {
protected:
  CalculationSetupTest() {
    data_root_p_.reset(new file_utils::FileURLRoot(
        file_utils::url_t::fs_path, "./"));
    initCalculations();
  }

  bool initCalculations() {
    bool success = false;
    fs::path p = calc_file;
    auto f = std::fstream(p, std::ios_base::out);

    if (f.is_open()) {
      f << "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
      f << "<calc_setup name=\"example\">\n";
      f << "  <models> PRb, GOST, ISO </models>\n";
      f << "  <gasmix_files>\n";
      f << "    <!-- mixtures from gost-30319 root dir is 'data' -->\n";
      f << "    <mixfile name=\"gostmix1\">" << path_gost1 << "</mixfile>\n";
      f << "    <mixfile name=\"gostmix2\">" << path_gost2 << "</mixfile>\n";
      f << "    <mixfile name=\"gostmix3\">" << path_gost3 << "</mixfile>\n";
      f << "  </gasmix_files>\n";
      f << "  <points>\n";
      f << "    <!-- [p] = Pa, [t] = K -->\n";
      f << "    <point p=\"100000\" t=\"250.0\"/>\n";
      f << "    <point p=\"5000000\" t=\"350.0\"/>\n";
      f << "    <point p=\"30000000\" t=\"300.0\"/>\n";
      f << "  </points>\n";
      f << "</calc_setup>\n";
      f.close();
      success = true;
    }
    return success;
  }

protected:
  /**
   * \brief Корневая директория дополнительных данных программы,
   *  для инициализации смесей, расчётов, т.п.
   * */
  std::shared_ptr<file_utils::FileURLRoot> data_root_p_;
};

/**
 * \brief Инициалиазция сетапов расчёта
 * */
TEST_F(CalculationSetupTest, calculation_setup_init) {
  CalculationSetupProxy csp(data_root_p_, calc_file);
  CalculationSetup &setup = csp.GetSetup();
  EXPECT_TRUE(setup.GetError() == ERROR_SUCCESS_T);
  calculation_setup *idp = csp.GetInitData();
  ASSERT_NE(idp, nullptr);

  // models
  ASSERT_EQ(idp->models.size(), 3);
  EXPECT_EQ(idp->models[0], rg_model_id(rg_model_t::PENG_ROBINSON, MODEL_PR_SUBTYPE_BINASSOC));
  EXPECT_EQ(idp->models[1], rg_model_id(rg_model_t::NG_GOST, MODEL_SUBTYPE_DEFAULT));
  EXPECT_EQ(idp->models[2], rg_model_id(rg_model_t::NG_GOST, MODEL_GOST_SUBTYPE_ISO_20765));

  // files
  ASSERT_EQ(idp->gasmix_files.size(), 3);
  auto p1 = data_root_p_->CreateFileURL(path_gost1).GetURL();
  EXPECT_TRUE(is_exist(p1));
  EXPECT_EQ(idp->gasmix_files[0], p1);
  auto p2 = data_root_p_->CreateFileURL(path_gost2).GetURL();
  EXPECT_TRUE(is_exist(p2));
  EXPECT_EQ(idp->gasmix_files[1], p2);
  auto p3 = data_root_p_->CreateFileURL(path_gost3).GetURL();
  EXPECT_TRUE(is_exist(p3));
  EXPECT_EQ(idp->gasmix_files[2], p3);

  // points
  std::function<bool(const parameters &, const parameters &)> comp_p =
      [] (const parameters &l, const parameters &r) {
      return is_equal(l.volume, r.volume) && is_equal(l.pressure, r.pressure) &&
          is_equal(l.temperature, r.temperature);
  };
  ASSERT_EQ(idp->points.size(), 3);
  EXPECT_TRUE(comp_p(idp->points[0],
      parameters {.volume = 0.0, .pressure = 100000.0, .temperature = 250.0}));
  EXPECT_TRUE(comp_p(idp->points[1],
      parameters {.volume = 0.0, .pressure = 5000000.0, .temperature = 350.0}));
  EXPECT_TRUE(comp_p(idp->points[2],
      parameters {.volume = 0.0, .pressure = 30000000.0, .temperature = 300.0}));
}
