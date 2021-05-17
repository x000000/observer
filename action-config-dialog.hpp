#pragma once

#include "observer.hpp"
#include <QDialog>

class Ui_ActionConfigDialog;

class ActionConfigDialog : public QDialog
{
    Q_OBJECT

    Ui_ActionConfigDialog *ui;

public:
    explicit ActionConfigDialog(action_descriptor *settings, QWidget *parent = nullptr);
    ~ActionConfigDialog() override;

    void save(action_descriptor *settings);
};
