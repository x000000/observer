#pragma once

#include <QWidget>
#include "observer.hpp"
#include "delegates/button-delegate.hpp"
#include "delegates/multiline-delegate.hpp"
#include "delegates/spinbox-delegate.hpp"
#include "delegates/action-delegate.hpp"
#include "settings-model.hpp"

class Ui_ObserverSettings;

class ObserverSettings : public QWidget
{
    Q_OBJECT
    Ui_ObserverSettings *ui;

public:
    explicit ObserverSettings(observer_settings settings, QWidget *parent = nullptr);
    ~ObserverSettings() override;

    ObserverSettingsModel *model() { return &_model; }

Q_SIGNALS:
    void closed(bool result);

public Q_SLOTS:
    void addActionItem();
    void accept() {
        _dialogResult = true;
        close();
    }

protected:
    void closeEvent(QCloseEvent *event) override;

private:
    bool _dialogResult = false;
    ObserverSettingsModel _model;
    ButtonDelegate _deleteButtonDelegate;
    SpinBoxDelegate _spinboxDelegate;
    MultilineDelegate _multilineDelegate;
    ActionDelegate _actionDelegate;

    void onDataChanged(const QModelIndex &topLeft, const QModelIndex &bottomRight, const QVector<int> &roles);
    void onRowsInserted(const QModelIndex &parent, int first, int last);
    void onDeleteClicked(const QModelIndex &index, const bool checked);

    void openEditors(int row);
};
