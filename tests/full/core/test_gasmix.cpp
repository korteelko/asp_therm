#include "gasmix_init.h"

#include "common.h"
#include "gas_defines.h"
#include "gas_description.h"
#include "models_math.h"

#include "gtest/gtest.h"

#include <iostream>
#include <numeric>

#include <assert.h>

using namespace ns_avg;

// Tk setup(в xml файлах теже параметры)
static const_parameters *methane = const_parameters::Init(
    GAS_TYPE_METHANE, 0.00617, 4226000, 190.66, 0.0, 16.043, 0.011);
static const_parameters *ethane = const_parameters::Init(
    GAS_TYPE_ETHANE, 0.0049, 4871000, 305.33, 0.0, 30.07, 0.105);
static const_parameters *n_butane = const_parameters::Init(
    GAS_TYPE_N_BUTANE, 0.0/*0.00438*/, 3800000, 425.66, 0.274, 58.123, 0.193);

// Vk setup
static const_parameters *toluene = const_parameters::Init(
    GAS_TYPE_TOLUENE, 0.316 / 92.14, 4130000, 593.0, 0.284, 92.14, 0.266);
static const_parameters *n_hexane = const_parameters::Init(
    GAS_TYPE_HEXANE, 0.37 / 86.178, 3030000, 507.85, 0.263, 86.178, 0.296);
/** \brief Тесты функций пересчёта критических значений смеси
  *   по критическим параметрам её компонентов */
class MixtureCriticalTest: public ::testing::Test {
protected:
  MixtureCriticalTest() {}
  ~MixtureCriticalTest() override {}

  /** \brief Проверка инициализации структур константных параметров
    *   если вносились ломающие изменения в const_parameters
    *   слетит здесь */
  void SetUp() override {
    assert(methane);
    assert(ethane);
    assert(n_butane);
    assert(toluene);
    assert(n_hexane);
  }

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
      return a + c.first * c.second.first.molecularmass;});
  double ans = 0.328 / av_mol;
  double dans = ans * 0.015;
  EXPECT_NEAR(ch_pr_avg_Vk(pm), ans, dans);
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
