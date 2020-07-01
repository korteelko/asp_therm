#include "calculation_info.h"
#include "atherm_common.h"

#include "gtest/gtest.h"

#include <ctime>


TEST(calculation_info, DateTimeByString) {
  const char *ds = "1937/03/15",
             *dt = "13:33";
  calculation_info ci;
  EXPECT_EQ(ci.initialized, calculation_info::f_empty);
  /* try to set valid date */
  mstatus_t st = ci.SetDate(ds);
  EXPECT_EQ(st, STATUS_OK);
  EXPECT_EQ(ci.initialized, calculation_info::f_date);
  /* try to set valid time */
  st = ci.SetTime(dt);
  EXPECT_EQ(st, STATUS_OK);
  EXPECT_EQ(ci.initialized, calculation_info::f_date | calculation_info::f_time);

  /* check result for valid data */
  EXPECT_TRUE(ci.GetDate() == ds);
  EXPECT_TRUE(ci.GetTime() == dt);

  /* corrupted data */
  /*   date */
  ci.initialized = calculation_info::f_empty;
  st = ci.SetDate("sd1999/02/03");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetDate("19999/12/12");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetDate("1999/13/12");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetDate("1999/11/32");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetDate("1999/02/29");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetDate("1999/02");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetDate("1999/02/");
  EXPECT_NE(st, STATUS_OK);
  EXPECT_EQ(ci.initialized, calculation_info::f_empty);
  /*   time */
  st = ci.SetTime("sd08:12");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetTime("24:54");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetTime("22:64:01");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetTime("24");
  EXPECT_NE(st, STATUS_OK);
  st = ci.SetTime("24:");
  EXPECT_NE(st, STATUS_OK);
  EXPECT_EQ(ci.initialized, calculation_info::f_empty);
}

TEST(calculation_info, DateTimeCTime) {
  calculation_info ci;
  std::time_t dati;
  struct tm *as_tm;
  as_tm = localtime(&dati);
  as_tm->tm_year = 37;
  as_tm->tm_mon = 2;
  as_tm->tm_mday = 15;
  as_tm->tm_hour = 13;
  as_tm->tm_min = 33;
  as_tm->tm_sec = 23;
  dati = mktime(as_tm);
  ci.SetDateTime(&dati);

  /* check result */
  EXPECT_EQ(ci.initialized, calculation_info::f_date | calculation_info::f_time);
  EXPECT_EQ(ci.GetDate(), "1937/03/15");
  EXPECT_EQ(ci.GetTime(), "13:33");
}

int main(int argc, char **argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}
