#pragma once

#include <QEvent>
#include <QAbstractItemModel>

template <typename T>
class IndexAwareDelegate
{
public:
    virtual void updateIndex(QModelIndex &index, QAbstractItemModel *model, QWidget *widget) = 0;

protected:
    QMultiHash<QWidget *, T> _dataByIndex;
};


class EditorReferencedEvent : public QEvent
{
public:
    explicit EditorReferencedEvent(QEvent::Type type, QWidget *editor)
            : QEvent(type), _editor(editor)
    {
    }

    QWidget *editor() { return _editor; }
private:
    QWidget *_editor;
};