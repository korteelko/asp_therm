/**
 * asp_therm - implementation of real gas equations of state
 *
 *
 * Copyright (c) 2020 Mishutinski Yurii
 *
 * This library is distributed under the MIT License.
 * See LICENSE file in the project root for full license information.
 */
#ifndef _GUI__GUI_ELEMENTS_H_
#define _GUI__GUI_ELEMENTS_H_

#include "gas_description.h"
#include "gas_mix_init.h"

#include <QAbstractTableModel>
#include <QString>

#define GASMIX_COMPONENTS_FIELDS_COUNT   2
#define GASMIX_HISTORY_FIELDS_COUNT      8

// gas_components table
class GasMixComponent {
  gas_mix_file mix_component_;

public:
  GasMixComponent(const gas_mix_file &gm);
  GasMixComponent(const GasMixComponent &gmc);
  GasMixComponent &operator= (const GasMixComponent &gmc);
  QString GetName() const;
  double GetPart() const;
  ~GasMixComponent();
};

class GasMixComponentModel : public QAbstractTableModel {
  QList<GasMixComponent> data_;

public:
  GasMixComponentModel(QObject *parent);
  int rowCount(const QModelIndex &) const override;
  int columnCount(const QModelIndex &) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section,
      Qt::Orientation orientation, int role) const override;
  bool removeRows(int row, int count, const QModelIndex &parent) override;
  void append(const GasMixComponent &component);
  QString getName(const QModelIndex &i);
};

// history table
class ResultHistory {
  state_log log_record_;

public:
  ResultHistory(const state_log &st_log);
  double GetPressure() const;
  double GetVolume() const;
  double GetTemperature() const;
  double GetHeatCapacityV() const;
  double GetHeatCapacityP() const;
  double GetInternalEnergy() const;
  double GetEnthalpy() const;
  QString GetState() const;
};

class ResultHistoryModel : public QAbstractTableModel {
  QList<ResultHistory> data_;

public:
  ResultHistoryModel(QObject *parent);
  int rowCount(const QModelIndex &) const override;
  int columnCount(const QModelIndex &) const override;
  QVariant data(const QModelIndex &index, int role) const override;
  QVariant headerData(int section,
      Qt::Orientation orientation, int role) const override;
  void append(const ResultHistory& st_log);
};

#endif  // !_GUI__GUI_ELEMENTS_H_
