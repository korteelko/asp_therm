#ifndef GUI_MAINWINDOW_H_
#define GUI_MAINWINDOW_H_

#include <QAbstractTableModel>
#include <QMainWindow>
#include <QTableView>

#include <list>
#include <memory>
#include <vector>
#include <string>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow {
  Q_OBJECT
  Ui::MainWindow *ui;
  // gui elements
  std::unique_ptr<QAbstractTableModel> tbl_model_components_;
  std::unique_ptr<QAbstractTableModel> tbl_model_history_;
  std::vector<std::string> gases_list_;
  std::list<std::string> gases_list_showed_;

public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

private:
// init methods
  void connect_slots();
  void set_components_cmb();
  void init_visual_components();
  void init_tables();
  void init_table_components();
  void init_table_history();

  void resize_table(QTableView *table);

private slots:
  void add_component_to_components_tbl();
  void remove_gas_from_components_cmb();
};

#endif  // !GUI_MAINWINDOW_H_
