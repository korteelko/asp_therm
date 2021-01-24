/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020-2021 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#include "mainwindow.h"

#include "gui_elements.h"
#include "program_data.h"
#include "ui_mainwindow.h"

#include <assert.h>

const size_t table_margin = 4;

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
  gases_list_ = pd.GetGasesList();
  set_components_cmb();
  init_tables();
  return;
}

void MainWindow::set_components_cmb() {
  for (const auto &x : gases_list_)
    ui->cmbAddComponent->addItem(x.c_str());
}

void MainWindow::init_tables() {
  init_table_components();
  init_table_history();
}

void MainWindow::init_table_components() {
  tbl_model_components_ = std::unique_ptr<QAbstractTableModel>(
      new GasMixComponentModel(this));
  //QAbstractTableModel *d = dynamic_cast<QAbstractTableModel *>(tbl_model_components_.get());
  ui->tbl_vwComponents->setModel(tbl_model_components_.get());
  QAbstractItemDelegate *del = ui->tbl_vwComponents->itemDelegateForColumn(1);
  del->
  resize_table(ui->tbl_vwComponents);
}

void MainWindow::init_table_history() {
  tbl_model_history_ = std::unique_ptr<QAbstractTableModel>(
      new ResultHistoryModel(this));
  ui->tbl_vwHistory->setModel(tbl_model_history_.get());
  resize_table(ui->tbl_vwHistory);
}

void MainWindow::resize_table(QTableView *table) {
  for (int i = 0; i < table->horizontalHeader()->count(); ++i)
    table->horizontalHeader()->setSectionResizeMode(i, QHeaderView::Stretch);
  table->resizeColumnsToContents();
}

// slots
void MainWindow::add_component_to_components_tbl() {
  QString gasname = ui->cmbAddComponent->currentText();
  bool convert_success = false;
  double part = ui->ln_edComponentPart->text().toDouble(&convert_success);
  if (gasname.isEmpty() || (!convert_success)) {
    ui->statusBar->showMessage("Fail add component: " + gasname +
        ui->ln_edComponentPart->text());
    return;
  }
  gases_list_showed_.push_back(gasname.toStdString());
  ui->cmbAddComponent->removeItem(ui->cmbAddComponent->currentIndex());
  dynamic_cast<GasMixComponentModel *>(tbl_model_components_.get())->append(
      GasMixComponent(gas_mix_file(gasname.toStdString(), part)));
  if (ui->cmbAddComponent->count() == 0) {
    ui->cmbAddComponent->setEnabled(false);
    ui->btnAddComponent->setEnabled(false);
  }
  ui->btnRemoveComponent->setEnabled(true);
}

void MainWindow::remove_gas_from_components_cmb() {
  QModelIndex i = ui->tbl_vwComponents->currentIndex();
  // add gas to combobox
  QString gas_name = dynamic_cast<GasMixComponentModel *>(
      tbl_model_components_.get())->getName(i);
  ui->cmbAddComponent->addItem(gas_name);
  // remove from table
  tbl_model_components_->removeRows(i.row(), 1);
  if (tbl_model_components_->rowCount() == 0)
    ui->btnRemoveComponent->setEnabled(false);
  ui->cmbAddComponent->setEnabled(false);
  ui->btnAddComponent->setEnabled(false);
}

MainWindow::~MainWindow() {
  delete ui;
}
