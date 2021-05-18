#pragma once

#include <QAbstractItemModel>
#include "observer.hpp"

enum Columns {
    Buttons = 0,
    Active  = 1,
    Users   = 2,
    Expr    = 3,
    Action  = 4,
    COUNT   = 5,
};

class ObserverSettingsModel : public QAbstractTableModel
{
//Q_OBJECT
public:
    ObserverSettingsModel(observer_settings settings, QObject *parent = nullptr);

    [[nodiscard]] int rowCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    [[nodiscard]] QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;
    [[nodiscard]] QVariant headerData(int section, Qt::Orientation orientation, int role) const override;

    bool setData(const QModelIndex &index, const QVariant &value, int role = Qt::EditRole) override;
    [[nodiscard]] Qt::ItemFlags flags(const QModelIndex &index) const override;

    void addItem(const action_descriptor action);
    void removeItem(const int index);

    observer_settings *settings() { return &_settings; }
private:
    observer_settings _settings;
};