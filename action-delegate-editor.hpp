#pragma once

#include <QWidget>
#include "observer.hpp"

class Ui_ActionDelegateEditor;

class ActionDelegateEditor : public QWidget
{
    Q_OBJECT

    Ui_ActionDelegateEditor *ui;

public:
    explicit ActionDelegateEditor(QWidget *parent = nullptr);
    ~ActionDelegateEditor() override;

    ActionType type() const;
    QStringList sources() const;

    void setValue(const ActionType type, const QStringList sources);

Q_SIGNALS:
    void typeChanged();
    void sourcesChanged();

private:
    bool _pristine = true;
};
