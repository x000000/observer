#pragma once

#include <QStyledItemDelegate>
#include "index-aware-delegate.hpp"

class ButtonDelegate : public QStyledItemDelegate
                     , public IndexAwareDelegate<QMetaObject::Connection>
{
Q_OBJECT

public:
    ButtonDelegate(QString text, QObject *parent = nullptr);

    QWidget *createEditor(QWidget *parent, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    void updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &index) const override;

    bool editorEvent(QEvent *event, QAbstractItemModel *model,
                     const QStyleOptionViewItem &option, const QModelIndex &index) override;

    void updateIndex(QModelIndex &index, QAbstractItemModel *model, QWidget *widget) override;
Q_SIGNALS:
    void clicked(QModelIndex index, bool checked);

private:
    void connect(QWidget *widget, QModelIndex index);

    QString _text;
};
