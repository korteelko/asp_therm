#ifndef GUI_MAINWINDOW_H_
#define GUI_MAINWINDOW_H_

#include <QMainWindow>
#include <QAbstractTableModel>

#include <memory>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT
  Ui::MainWindow *ui;
  // gui elements
  std::unique_ptr<QAbstractTableModel> tbl_model_components_;
  std::unique_ptr<QAbstractTableModel> tbl_model_history_;

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
// init methods
  void connect_slots();
  void init_visual_components();
  void init_components_cmb(const std::vector<std::string> &components);
  void init_tables();
  void init_table_components();
  void init_table_history();
// processing methods

private slots:
  void add_component_to_components_tbl();
  void remove_gas_from_components_cmb();
};

#endif  // !GUI_MAINWINDOW_H_
