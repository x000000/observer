#include <QEvent>
#include "action-config.hpp"
#include "action-delegate.hpp"
#include "action-delegate-editor.hpp"

ActionDelegate::ActionDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *ActionDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &/* option */,
                                      const QModelIndex &/* index */) const
{
    return new ActionDelegateEditor(parent);
}

bool ActionDelegate::editorEvent(QEvent *ev, QAbstractItemModel *model,
                                 const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (ev->type() == QEvent::Show) {
        auto widget = static_cast<EditorReferencedEvent *>(ev)->editor();
        connect(widget, model, index);
        return false;
    }
    return QStyledItemDelegate::editorEvent(ev, model, option, index);
}

void ActionDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto value = qvariant_cast<ActionConfig>(index.model()->data(index, Qt::EditRole));
    auto input = static_cast<ActionDelegateEditor*>(editor);
    input->setValue(value.type(), value.sources());
}

void ActionDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto input   = static_cast<ActionDelegateEditor*>(editor);
    auto type    = input->type();
    auto sources = input->sources();
    model->setData(index, QVariant::fromValue(ActionConfig(type, sources)), Qt::EditRole);
}

void ActionDelegate::connect(QWidget *widget, QAbstractItemModel *model, QModelIndex index)
{
    auto editor = static_cast<ActionDelegateEditor *>(widget);

    _dataByIndex.insert(
        widget,
        QObject::connect(editor, &ActionDelegateEditor::typeChanged, [=]() {
            setModelData(editor, model, index);
        })
    );
    _dataByIndex.insert(
        widget,
        QObject::connect(editor, &ActionDelegateEditor::sourcesChanged, [=]() {
            setModelData(editor, model, index);
        })
    );
}

void ActionDelegate::updateIndex(QModelIndex &index, QAbstractItemModel *model, QWidget *widget)
{
    auto it = _dataByIndex.constFind(widget);
    while (it != _dataByIndex.end() && it.key() == widget) {
        QObject::disconnect(it.value());
        ++it;
    }
    _dataByIndex.remove(widget);
    connect(widget, model, index);
}

void ActionDelegate::updateEditorGeometry(QWidget *editor,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &/* index */) const
{
    editor->setGeometry(option.rect);
}