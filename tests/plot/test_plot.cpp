/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "plot_defines.h"
#include "plot_main.h"

#include <iostream>

plot_data_2d inp_2d {
  "test plot 2d",
  Color::blue,
  {{0.0, 0.0}, {0.1, 0.2}, {0.2, 0.6}, {0.3, 0.9}, {0.4, 1.5}}
};

plot_data_3d inp_3d {
  "test plot 3d",
  Color::red,
  {{0.0, 0.0, 0.0}, {0.1, 0.2, 0.0}, {0.2, 0.6, 0.0}, 
      {0.3,0.9, 0.1}, {0.4, 1.5, -0.1}
   }
};

int test_2d (){
  PlotCanvas canv2d("test canvas 2d", canvas_type::_2d, Color::black);
  auto pl1 = canv2d.AddPlot(inp_2d);
  if (pl1 == nullptr) {
    std::cerr << "pl1 null get\n" << std::flush;
    return 1;
  }
  auto pl2 = canv2d.AddPlot(inp_3d);
  if (pl2 == nullptr) {
    std::cerr << "pl2 null get\n" << std::flush;
    return 1;
  }
  canv2d.Show();
  std::cerr << "showed 2 plot\npush any key to continue\n" << std::flush;
  std::getchar();
  canv2d.RemovePlot(pl1);
  std::cerr << "removed one of this\npush any key to continue\n" << std::flush;
  std::getchar();
  canv2d.HidePlot(pl2);
  std::cerr << "hide second of this\npush any key to continue\n" << std::flush;
  std::getchar();
  canv2d.ClearCanvas();
  std::cerr << "clear canvas\npush any key to continue\n" << std::flush;
  std::getchar();
  canv2d.Hide();
}

int main() {
  if (test_2d())
    return 1;
  return 0;
}