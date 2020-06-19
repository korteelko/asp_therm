#include "merror_codes.h"
#include "program_state.h"

#include "gtest/gtest.h"


const unsigned int error_types[] = {
  ERROR_SUCCESS_T,
  ERROR_GENERAL_T,
  ERROR_FILEIO_T,
  ERROR_CALCULATE_T,
  ERROR_STRING_T,
  ERROR_INIT_T,
  ERROR_STRTPL_T,
  ERROR_DATABASE_T
};
#define error_types_size sizeof(error_types)/sizeof(error_types[0])


/** \brief Проверим работу дефолтных сообщений групп */
TEST(defaultErrorMessages, MessagesExists) {
  EXPECT_EQ(GetCustomErrorMsg(0x1111), nullptr);
  // Сообщения групп ошибок
  for (size_t i = 0; i < error_types_size; ++i)
    EXPECT_TRUE(GetCustomErrorMsg(error_types[i]) != nullptr);
  EXPECT_FALSE(GetCustomErrorMsg(ERROR_MASK_TYPE) != nullptr);
  // Существование нетривиальных сообщений ошибок
  for (size_t i = 2; i < error_types_size; ++i) {
    // Для примера первое сообщение
    EXPECT_TRUE(GetCustomErrorMsg(error_types[i] | 0x0100) != nullptr);
    // Ошибка
    EXPECT_TRUE(GetCustomErrorMsg(error_types[i] | 0xff00) == nullptr);
  }
}

/** \brief Проверим работу других дефолтных сообщений */
TEST(testProgramState, Init) {
  // EXPECT_TRUE(GetCustomErrorMsg() != nullptr);
}
