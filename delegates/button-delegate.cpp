#include <QEvent>
#include <QPushButton>
#include "button-delegate.hpp"

ButtonDelegate::ButtonDelegate(QString text, QObject *parent)
    : QStyledItemDelegate(parent), _text(text)
{
}

QWidget *ButtonDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &/* option */,
                                      const QModelIndex &/* index */) const
{
    auto button = new QPushButton(_text, parent);
    button->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    button->setStyleSheet("margin: 4;");
    return button;
}

bool ButtonDelegate::editorEvent(QEvent *ev, QAbstractItemModel *model,
                                 const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (ev->type() == QEvent::Show) {
        auto widget = static_cast<EditorReferencedEvent *>(ev)->editor();
        connect(widget, index);
        return false;
    }
    return QStyledItemDelegate::editorEvent(ev, model, option, index);
}

void ButtonDelegate::updateEditorGeometry(QWidget *editor,
                                          const QStyleOptionViewItem &option,
                                          const QModelIndex &) const
{
    QRect rect(option.rect);

    auto size = editor->sizeHint();
    auto diff = rect.height() - size.height();

    rect.setTop(rect.top() + diff / 2);
    rect.setHeight(size.height());

    editor->setGeometry(rect);
}

void ButtonDelegate::connect(QWidget *widget, QModelIndex index)
{
    auto button = static_cast<QPushButton *>(widget);
    _dataByIndex.insert(
        widget,
        QObject::connect(button, &QPushButton::clicked, [this, index](bool checked) {
            emit clicked(index, checked);
        })
    );
}

void ButtonDelegate::updateIndex(QModelIndex &index, QAbstractItemModel */*model*/, QWidget *widget)
{
    auto it = _dataByIndex.constFind(widget);
    while (it != _dataByIndex.end() && it.key() == widget) {
        QObject::disconnect(it.value());
        ++it;
    }
    _dataByIndex.remove(widget);
    connect(widget, index);
}