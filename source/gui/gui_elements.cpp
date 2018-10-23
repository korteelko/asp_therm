#include "gui_elements.h"

namespace {
  QString table_components_header[GASMIX_COMPONENTS_FIELDS_COUNT] = {
    "gas name",
    "part"
  };
  enum components_header_id {
    COMPONENTS_GASNAME = 0,
    COMPONENTS_PART = 1
  };
  QString table_history_header[GASMIX_HISTORY_FIELDS_COUNT] = {
    "p",
    "v",
    "t",
    "Cp",
    "Cv",
    "u",
    "h",
    "state"
  };
  enum history_header_id {
    HISTORY_P   = 0,
    HISTORY_V   = 1,
    HISTORY_T   = 2,
    HISTORY_CV  = 3,
    HISTORY_CP  = 4,
    HISTORY_U   = 5,
    HISTORY_H   = 6,
    HISTORY_ST  = 7
  };
}  // anonymus namespace

GasMixComponent::GasMixComponent(const gas_mix_file &gm)
  : mix_component_(gm) {}

GasMixComponent::GasMixComponent(const GasMixComponent &gmc)
  : mix_component_(gmc.mix_component_) {}

GasMixComponent &GasMixComponent::operator= (const GasMixComponent &gmc) {
  return (*this);
}

QString GasMixComponent::GetName() const {
  return QString(mix_component_.filename.c_str());
}

double GasMixComponent::GetPart() const {
  return mix_component_.part;
}

GasMixComponent::~GasMixComponent() {}

GasMixComponentModel::GasMixComponentModel(QObject *parent = NULL)
  : QAbstractTableModel(parent) {}

int GasMixComponentModel::rowCount(const QModelIndex &) const {
  return data_.count();
}

int GasMixComponentModel::columnCount(const QModelIndex &) const {
  return GASMIX_COMPONENTS_FIELDS_COUNT;
}

QVariant GasMixComponentModel::data(const QModelIndex &index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();
  const auto &component = data_[index.row()];
  switch (index.column()) {
    case COMPONENTS_GASNAME: return component.GetName();
    case COMPONENTS_PART: return component.GetPart();
    default: return QVariant();
  }
}

QVariant GasMixComponentModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return QVariant();
  if (section < GASMIX_COMPONENTS_FIELDS_COUNT)
    return table_components_header[section];
  else
    return QVariant();
}

bool GasMixComponentModel::removeRows(int row, int count,
    const QModelIndex &parent = QModelIndex()) {
  beginRemoveRows(QModelIndex(), row, row + count - 1);
  for (int i = 0; i < count; ++i)
    data_.removeAt(row);
  endRemoveRows();
  return true;
}

void GasMixComponentModel::append(const GasMixComponent &component) {
  beginInsertRows(QModelIndex(), data_.count(), data_.count());
  data_.append(component);
  endInsertRows();
}

ResultHistory::ResultHistory(const state_log &st_log)
  : log_record_(st_log) {}

double ResultHistory::GetPressure() const {
  return log_record_.dyn_pars.parm.pressure;
}

double ResultHistory::GetVolume() const {
  return log_record_.dyn_pars.parm.volume;
}

double ResultHistory::GetTemperature() const {
  return log_record_.dyn_pars.parm.temperature;
}

double ResultHistory::GetHeatCapacityV() const {
  return log_record_.dyn_pars.heat_cap_vol;
}

double ResultHistory::GetHeatCapacityP() const {
  return log_record_.dyn_pars.heat_cap_pres;
}

double ResultHistory::GetInternalEnergy() const {
  return log_record_.dyn_pars.internal_energy;
}

double ResultHistory::GetEnthalpy() const {
  return log_record_.dyn_pars.internal_energy + GetPressure() * GetVolume();
}

QString ResultHistory::GetState() const {
  return QString(log_record_.state_phase.c_str());
}

ResultHistoryModel::ResultHistoryModel(QObject *parent = NULL)
  : QAbstractTableModel(parent) {}

int ResultHistoryModel::rowCount(const QModelIndex &) const {
  return data_.count();
}

int ResultHistoryModel::columnCount(const QModelIndex &) const {
  return GASMIX_HISTORY_FIELDS_COUNT;
}

QVariant ResultHistoryModel::data(const QModelIndex &index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole)
    return QVariant();
  const auto &result = data_[index.row()];
  switch (index.column()) {
    case HISTORY_P: return result.GetPressure();
    case HISTORY_V: return result.GetVolume();
    case HISTORY_T: return result.GetTemperature();
    case HISTORY_CV: return result.GetHeatCapacityV();
    case HISTORY_CP: return result.GetHeatCapacityP();
    case HISTORY_U: return result.GetInternalEnergy();
    case HISTORY_H: return result.GetEnthalpy();
    case HISTORY_ST: return result.GetState();
    default: return QVariant();
  }
}

QVariant ResultHistoryModel::headerData(int section,
    Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole)
    return QVariant();
  if (section < GASMIX_HISTORY_FIELDS_COUNT)
    return table_history_header[section];
  else
    return QVariant();
}

void ResultHistoryModel::append(const ResultHistory &st_log) {
  beginInsertRows(QModelIndex(), data_.count(), data_.count());
  data_.append(st_log);
  endInsertRows();
}
