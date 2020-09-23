#include "gas_description.h"

#include "gtest/gtest.h"

TEST(gas_char, CheckType) {
  // aromatic - with benzene ring
  EXPECT_TRUE(gas_char::IsAromatic(CH(BENZENE)));
  EXPECT_TRUE(gas_char::IsAromatic(CH(TOLUENE)));
  //   not hydrocarbon
  EXPECT_FALSE(gas_char::IsAromatic(CH(NITROGEN)));

  // simple hydrocarbon, not aromatic
  EXPECT_TRUE(gas_char::IsHydrocarbon(CH(METHANE)));
  EXPECT_TRUE(gas_char::IsHydrocarbon(CH(ETHANE)));
  EXPECT_TRUE(gas_char::IsHydrocarbon(CH(OCTANE)));
  //   not hydrocarbon
  EXPECT_FALSE(gas_char::IsHydrocarbon(CH(NITROGEN)));
  //   not simple - aromatic
  EXPECT_FALSE(gas_char::IsHydrocarbon(CH(BENZENE)));

  // CircleParafine
  EXPECT_TRUE(gas_char::IsCycleParafine(CH(CYCLOPENTENE)));
  EXPECT_TRUE(gas_char::IsCycleParafine(CH(MCYCLOPENTENE)));
  EXPECT_TRUE(gas_char::IsCycleParafine(CH(ECYCLOPENTENE)));
  EXPECT_TRUE(gas_char::IsCycleParafine(CH(CYCLOHEXANE)));
  //   not hydrocarbon
  EXPECT_FALSE(gas_char::IsCycleParafine(CH(NITROGEN)));
  //   not hydrocarbon
  EXPECT_FALSE(gas_char::IsCycleParafine(CH(BENZENE)));

  // Noble
  EXPECT_TRUE(gas_char::IsNoble(CH(HELIUM)));
  EXPECT_TRUE(gas_char::IsNoble(CH(ARGON)));
  EXPECT_TRUE(gas_char::IsNoble(CH(NEON)));
  //   not noble
  EXPECT_FALSE(gas_char::IsNoble(CH(NITROGEN)));
  EXPECT_FALSE(gas_char::IsNoble(CH(METHANE)));

  // jff
  EXPECT_TRUE(gas_char::IsHydrogenSulfide(CH(HYDROGEN_SULFIDE)));
  EXPECT_FALSE(gas_char::IsHydrogenSulfide(CH(METHANE)));
  EXPECT_TRUE(gas_char::IsCarbonDioxide(CH(CARBON_DIOXIDE)));
  EXPECT_FALSE(gas_char::IsCarbonDioxide(CH(METHANE)));
  EXPECT_TRUE(gas_char::IsAcetylene(CH(ACETYLENE)));
  EXPECT_FALSE(gas_char::IsAcetylene(CH(METHANE)));
  EXPECT_TRUE(gas_char::IsCarbonMonoxide(CH(CARBON_MONOXIDE)));
  EXPECT_FALSE(gas_char::IsCarbonMonoxide(CH(METHANE)));
}
