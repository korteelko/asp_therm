#include "mainwindow.h"

#include "gui_elements.h"
#include "program_data.h"
#include "ui_mainwindow.h"

#include <assert.h>

MainWindow::MainWindow(QWidget *parent)
  : QMainWindow(parent), ui(new Ui::MainWindow) {
  ui->setupUi(this);
  init_visual_components();
  connect_slots();
}

void MainWindow::connect_slots() {
  connect(ui->btnAddComponent, SIGNAL(clicked()), this,
      SLOT(add_component_to_components_tbl()));
  connect(ui->btnRemoveComponent, SIGNAL(clicked()), this,
      SLOT(remove_gas_from_components_cmb()));
}

void MainWindow::init_visual_components() {
  ProgramData &pd = ProgramData::Instance();
  init_components_cmb(pd.GetGasesList());
  init_tables();
  return;
}

void MainWindow::init_components_cmb(
    const std::vector<std::string> &components) {
  for (const auto &x : components)
    ui->cmbAddComponent->addItem(x.c_str());
}

void MainWindow::init_tables() {
  init_table_components();
  init_table_history();
}

void MainWindow::init_table_components() {
  tbl_model_components_ = std::unique_ptr<QAbstractTableModel>(
      new GasMixComponentModel(this));
  ui->tbl_vwComponents->setModel(tbl_model_components_.get());
}

void MainWindow::init_table_history() {
  tbl_model_history_ = std::unique_ptr<QAbstractTableModel>(
      new ResultHistoryModel(this));
  ui->tbl_vwHistory->setModel(tbl_model_history_.get());
}

void MainWindow::add_component_to_components_tbl() {
  QString gasname = ui->cmbAddComponent->currentText();
  bool convert_success = false;
  double part = ui->ln_edComponentPart->text().toDouble(&convert_success);
  if (gasname.isEmpty() || (!convert_success)) {
    ui->statusBar->showMessage("Fail add component: " + gasname +
        ui->ln_edComponentPart->text());
    return;
  }
  dynamic_cast<GasMixComponentModel *>(tbl_model_components_.get())->append(
      GasMixComponent({gasname.toStdString(), part}));
  if (ui->cmbAddComponent->count() == 0) {
    ui->cmbAddComponent->setEnabled(false);
    ui->btnAddComponent->setEnabled(false);
  }
  ui->btnRemoveComponent->setEnabled(true);
}

void MainWindow::remove_gas_from_components_cmb() {
  QModelIndex i = ui->tbl_vwComponents->currentIndex();
  tbl_model_components_->removeRows(i.row(), 1);
  if (tbl_model_components_->rowCount() == 0)
    ui->btnRemoveComponent->setEnabled(false);
  ui->cmbAddComponent->setEnabled(false);
  ui->btnAddComponent->setEnabled(false);
}

MainWindow::~MainWindow() {
  delete ui;
}
