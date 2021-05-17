#pragma once

#include <QDialog>
#include "observer.hpp"
#include "delegates/buttons-delegate.hpp"
#include "delegates/multiline-delegate.hpp"
#include "delegates/spinbox-delegate.hpp"
#include "delegates/action-delegate.hpp"
#include "settings-model.hpp"

class Ui_ObserverSettings;

class ObserverSettings : public QDialog
{
    Q_OBJECT

    Ui_ObserverSettings *ui;

public:
    explicit ObserverSettings(observer_settings settings, QWidget *parent = nullptr);
    ~ObserverSettings() override;

    ObserverSettingsModel *model() { return &_model; }

public Q_SLOTS:
    void addActionItem();

private:
    ObserverSettingsModel _model;
    ButtonDelegate _buttonsDelegate;

    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void onButtonClicked(const QModelIndex &index, const int buttonIndex, const bool checked);

    void openEditors(int row);
};
