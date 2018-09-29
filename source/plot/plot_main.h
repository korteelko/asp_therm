#ifndef _PLOT__PLOT_MAIN_H_
#define _PLOT__PLOT_MAIN_H_

#include "plot_defines.h"

#include <string>
#include <vector>

/* входные данные одного графика */
/* struct plot_data {
  std::string legend;
  Color line_color;
  virtual ~plot_data();
}; */

struct plot_data_2d {
  std::string legend;
  Color line_color;
  std::vector<point_2d> points;
};

struct plot_data_3d {
  std::string legend;
  Color line_color;
  std::vector<point_3d> points;
};

/* класс графика, зависит от используемой библиотеки построения */
class Plot;

/* класс окна графика */
class PlotCanvas {
private:
  std::string caption_;
  canvas_type canvas_type_;
  Color color_;
  std::vector<const Plot *> plots_;

public:
  PlotCanvas(std::string caption, canvas_type ct, Color cl);

  const Plot *AddPlot(const plot_data_2d &pd);
  const Plot *AddPlot(const plot_data_3d &pd);
  void RemovePlot(const Plot *pl);
  void HidePlot(const Plot *pl);
  void ClearCanvas();
  void Show();
  void Hide();
};

#endif  // !_PLOT__PLOT_MAIN_H_