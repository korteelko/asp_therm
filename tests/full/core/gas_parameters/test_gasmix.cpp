#include "gasmix_init.h"

#include "atherm_common.h"
#include "gas_by_file.h"
#include "gas_defines.h"
#include "gas_description.h"
#include "gasmix_by_file.h"
#include "models_math.h"
#include "XMLReader.h"

#include "gtest/gtest.h"

#include <filesystem>
#include <iostream>
#include <memory>
#include <numeric>

#include <assert.h>


using namespace ns_avg;
namespace fs = std::filesystem;

namespace gas_paths {
const fs::path xml_path = "../../../data/gases";
const fs::path xml_methane = "methane.xml";
const fs::path xml_ethane  = "ethane.xml";
const fs::path xml_propane = "propane.xml";

const fs::path xml_gasmix = "../gasmix_inp_example.xml";
static std::filesystem::path cwd;
}  // namespace valid_data

// Tk setup(в xml файлах теже параметры)
static std::unique_ptr<const_parameters> methane(const_parameters::Init(
    GAS_TYPE_METHANE, 0.00617, 4226000, 190.66, 0.0, 16.043, 0.011));
static std::unique_ptr<const_parameters> ethane(const_parameters::Init(
    GAS_TYPE_ETHANE, 0.0049, 4871000, 305.33, 0.0, 30.07, 0.105));
static std::unique_ptr<const_parameters> n_butane(const_parameters::Init(
    GAS_TYPE_N_BUTANE, 0.0/*0.00438*/, 3800000, 425.66, 0.274, 58.123, 0.193));

// Vk setup
static std::unique_ptr<const_parameters> toluene(const_parameters::Init(
    GAS_TYPE_TOLUENE, 0.316 / 92.14, 4130000, 593.0, 0.284, 92.14, 0.266));
static std::unique_ptr<const_parameters> n_hexane(const_parameters::Init(
    GAS_TYPE_HEXANE, 0.37 / 86.178, 3030000, 507.85, 0.263, 86.178, 0.296));

/** \brief Тесты функций пересчёта критических значений смеси
  *   по критическим параметрам её компонентов */
class MixtureCriticalTest: public ::testing::Test {
protected:
  /** \brief Проверка инициализации структур константных параметров
    *   если вносились ломающие изменения в const_parameters
    *   слетит здесь */
  MixtureCriticalTest() {
    assert(methane);
    assert(ethane);
    assert(n_butane);
    assert(toluene);
    assert(n_hexane);
  }
  ~MixtureCriticalTest() override {}

  void SetUp() override {}

  /** \brief собрать газовую смесь для проверки пересчёта критической
    *   температуры(см. пример 5.5 в книге "Свойства газов и жидкостей") */
  void set_tk_test_case() {
    pm = parameters_mix{
        {0.193, {*methane, dyn_parameters()}},
        {0.470, {*ethane, dyn_parameters()}},
        {0.337, {*n_butane, dyn_parameters()}}};
  }
  /** \brief собрать газовую смесь для проверки пересчёта критического
    *   объёма(см. пример 5.6 в книге "Свойства газов и жидкостей") */
  void set_vk_test_case() {
    pm = parameters_mix{
        {0.495, {*toluene, dyn_parameters()}},
        {0.505, {*n_hexane, dyn_parameters()}}};
  }

protected:
  /** \brief состовляющие газовой смеси */
  parameters_mix pm;
};

TEST(rk2_avg_TkTest, DISABLED_Simple) {}

TEST(rk2_avg_PkTest, DISABLED_Simple) {}

TEST(rk2_avg_ZkTest, DISABLED_Simple) {}

TEST(rk2_avg_acentricTest, DISABLED_Simple) {
  assert(0 && "спросить Валерия Алексеевича");
}

/** \brief Проверка реализации метода Ли,
  *   расчёта критической температуры */
TEST_F(MixtureCriticalTest, lee_avg_TkTest) {
  set_tk_test_case();
  EXPECT_NEAR(lee_avg_Tk(pm), 352.0, 0.5);
}
/** \brief Проверка реализации метода Чью-Праусница,
  *   расчёта критической температуры */
TEST_F(MixtureCriticalTest, ch_pr_avg_TkTest) {
  set_tk_test_case();
  EXPECT_NEAR(ch_pr_avg_Tk(pm), 353.0, 0.5);
}
/** \brief Проверка реализации метода Чью-Праусница,
  *   расчёта критического объёма */
TEST_F(MixtureCriticalTest, ch_pr_avg_VkTest) {
  set_vk_test_case();
  double  av_mol = std::accumulate(pm.begin(), pm.end(), 0.0,
      [](double a, const std::pair<const double, const_dyn_parameters> &c){
      return a + c.first * c.second.first.mp.mass;});
  double ans = 0.328 / av_mol;
  double dans = ans * 0.015;
  EXPECT_NEAR(ch_pr_avg_Vk(pm), ans, dans);
}
/* Тест инициализации */
/** \brief тест инициализации метана */
TEST(component_InitTest, MethaneInit) {
  std::cerr << "cwd: " << gas_paths::cwd  << std::endl;
  fs::path methane_path = gas_paths::cwd /
      gas_paths::xml_path / gas_paths::xml_methane;
  std::cerr << "methane: " << methane_path << std::endl;
  ASSERT_TRUE(fs::exists(methane_path));
  std::unique_ptr<ComponentByFile<XMLReader>> met_xml(
      ComponentByFile<XMLReader>::Init(methane_path.string()));
  ASSERT_TRUE(met_xml != nullptr);
  EXPECT_TRUE(met_xml->GetConstParameters() != nullptr);
  EXPECT_TRUE(met_xml->GetDynParameters() != nullptr);
}
/** \brief тест инициализации пропана */
TEST(component_InitTest, PropaneInit) {
  fs::path propane_path = gas_paths::cwd /
      gas_paths::xml_path / gas_paths::xml_propane;
  ASSERT_TRUE(fs::exists(propane_path));
  std::unique_ptr<ComponentByFile<XMLReader>> propane_xml(
      ComponentByFile<XMLReader>::Init(propane_path.string()));
  ASSERT_TRUE(propane_xml != nullptr);
  EXPECT_TRUE(propane_xml->GetConstParameters() != nullptr);
  EXPECT_TRUE(propane_xml->GetDynParameters() != nullptr);
}
/** \brief тест инициализации смеси файлом для классической
  *   двухпараметрической модели Редлиха-Квонга */
TEST(mixture_InitTest, RK2MixtureFileInit) {
  fs::path gasmix_path = gas_paths::cwd /
      gas_paths::xml_path / gas_paths::xml_gasmix;
  ASSERT_TRUE(fs::exists(gasmix_path));
  std::unique_ptr<GasMixComponentsFile<XMLReader>> gasmix_comps(
      GasMixComponentsFile<XMLReader>::Init(
      rg_model_t::REDLICH_KWONG, nullptr, gasmix_path.string()));
  ASSERT_TRUE(gasmix_comps != nullptr);
  EXPECT_TRUE(gasmix_comps->GetMixParameters() != nullptr);
}
/** \brief тест инициализации смеси файлом для модели по ГОСТ-30319 */
TEST(mixture_InitTest, DISABLED_GOSTMixtureFileInit) {
  fs::path gasmix_path = gas_paths::cwd /
      gas_paths::xml_path / gas_paths::xml_gasmix;
  ASSERT_TRUE(fs::exists(gasmix_path));
  std::unique_ptr<GasMixComponentsFile<XMLReader>> gasmix_comps(
      GasMixComponentsFile<XMLReader>::Init(
      rg_model_t::NG_GOST, nullptr, gasmix_path.string()));
  ASSERT_TRUE(gasmix_comps != nullptr);
  EXPECT_TRUE(gasmix_comps->GetMixParameters() != nullptr);
}

int main(int argc, char **argv) {
  gas_paths::cwd = fs::path(argv[0]).parent_path();
  fs::current_path(gas_paths::cwd);
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
