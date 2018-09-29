#ifndef _PLOT__PLOT_DEFINES_H_
#define _PLOT__PLOT_DEFINES_H_

#include <stdint.h>

#include <array>

/* список необходимых флагов и объявлений */

/* например */
/* пока не известно какой тип данных передаётся
     на вход ещё не разработанной библиотеки,
     т.к. мы не ограничены в выборе инструментов
   Так, можно обрабатывать или сразу данные 
     в пикселях - int,
   либо расчитанные - обычно double */
typedef double coor_t;

enum class canvas_type {
  _2d = 0,
  _3d
};

typedef std::array<coor_t, 2> point_2d;
typedef std::array<coor_t, 3> point_3d;

/* или цвета */
enum class Color : int32_t {
  black = 0,
  blue  = 1,
  red   = 2,
  green = 3,
  white = 4
};

#endif  // !_PLOT__PLOT_DEFINES_H_