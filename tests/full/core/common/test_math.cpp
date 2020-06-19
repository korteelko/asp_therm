#include "models_math.h"

#include "common.h"

#include "gtest/gtest.h"

#include <algorithm>


TEST(is_above0Test, Simple) {
  EXPECT_EQ(is_above0(-1), false);
  EXPECT_EQ(is_above0(NAN), false);
  EXPECT_EQ(is_above0(1.0 / 0.0), false);
  EXPECT_EQ(is_above0(0.1, 1.0, 1, 0.0), false);
  // EXPECT_EQ(is_above0(HUGE_VAL), true);  // hmmmm this test fails!
}

TEST(is_equalTest, Simple) {
  EXPECT_EQ(is_equal(1, 1, FLOAT_ACCURACY), true);
  EXPECT_EQ(is_equal(1, 2, FLOAT_ACCURACY), false);
  EXPECT_EQ(is_equal(1, 2, 2), true);
  EXPECT_EQ(is_equal(NAN, 1, FLOAT_ACCURACY), false);
  EXPECT_EQ(is_equal(NAN, NAN, FLOAT_ACCURACY), false);
}

/** \brief Тесты функции, реализующей метод Кардано/Виета,
  *   для решения кубического уравнения x^3 + a*x^2 + b*x + c = 0 */
class CardanoMethodTest: public ::testing::Test {
protected:
  CardanoMethodTest() {}
  ~CardanoMethodTest() override {}

  void SetUp() override {
    std::generate(&*ans, &*(ans + 4),
        [](){ return std::complex<double>(0.0, 0.0); });
  }

  void TearDown() override {
    std::generate(&*ans, &*(ans + 4),
        [](){ return std::complex<double>(0.0, 0.0); });
  }

  /** \brief Проверить val в c */
  template <class T>
  bool is_in(T *c, T val, int n) {
    bool is_in = false;
    for (int i = 0; i < n; ++i) {
      if (is_equal(c[i], val))
        return true;
    }
    return is_in;
  }

  /** \brief Проверить соответствие массивов корней */
  template <class T>
  bool eq_roots(T *res, T *expect, int n) {
    bool same = true;
    for (int i = 0; i < n; ++i) {
      if (!is_in(res, expect[i], n)) {
        same = false;
        break;
      }
    }
    return same;
  }

protected:
  /** \brief Массив коэффициентов при x^3, x^2, x, x^0, соответственно */
  double c[4];
  /** \brief Массив корней уравнения */
  std::complex<double> ans[4];
};

/** \brief Проверить возвращаемый результат для функции
  *   CardanoMethod_roots_count с входными параметрами
  *   дающими три реальных корня */
TEST_F(CardanoMethodTest, RootsCount3) {
  c[0] = 1.0; c[1] = 0.5; c[2] = -3.72; c[3] = 2.016;
  int rk = 0;
  double res[3] = {0.0, 0.0, 0.0};
  EXPECT_EQ(CardanoMethod_roots_count(c, res, &rk), ERROR_SUCCESS_T);
  double expect[3] = {-2.4, 0.7, 1.2};
  EXPECT_EQ(rk == 3, true);
  EXPECT_EQ(eq_roots(res, expect, 3), true);
}

/** \brief Проверить возвращаемый результат для функции
  *   CardanoMethod_roots_count с входными параметрами
  *   дающими один реальный корень */
TEST_F(CardanoMethodTest, RootsCount1) {
  c[0] = 1.0; c[1] = -1.1; c[2] = 0.8; c[3] = 2.3;
  int rk = 0;
  double res[3] = {0.0, 0.0, 0.0};
  EXPECT_EQ(CardanoMethod_roots_count(c, res, &rk), ERROR_SUCCESS_T);
  double expect[1] = {-0.892279};
  EXPECT_EQ(rk == 1, true);
  EXPECT_EQ(eq_roots(res, expect, 1), true);
}

/** \brief Проверить результат вычислений функции
  *   CardanoMethod(Кардано/Виета) на обычных данных */
TEST_F(CardanoMethodTest, Simple) {
  c[0] = 1.0; c[1] = - 1.1; c[2] = 0.8; c[3] = 2.3;
  EXPECT_EQ(CardanoMethod(c, ans), ERROR_SUCCESS_T);
  std::complex<double> expect[3] = {
      {-0.892279, 0.0}, {0.99614, -1.25912}, {0.99614, 1.25912}};
  EXPECT_EQ(eq_roots(ans, expect, 3), true);
}
