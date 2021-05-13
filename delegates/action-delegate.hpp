#pragma once

#include <QEvent>
#include <QStyledItemDelegate>
#include "index-aware-delegate.hpp"

class ActionDelegate : public QStyledItemDelegate
                     , public IndexAwareDelegate<QMetaObject::Connection>
{
Q_OBJECT

public:
    explicit ActionDelegate(QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void setEditorData(QWidget *editor, const QModelIndex &index) const override;
    void setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void updateIndex(QModelIndex &index, QAbstractItemModel *model, QWidget *widget) override;
private:
    void connect(QWidget *widget, QAbstractItemModel *model, QModelIndex index);
};