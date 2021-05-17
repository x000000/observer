#include <QEvent>
#include <QVBoxLayout>
#include <QPushButton>
#include "buttons-delegate.hpp"

const static QRegExp regex(R"(^.+[/\\])", Qt::CaseSensitive, QRegExp::RegExp2);
const static QString editor_name = QString(__FILE__).replace('.', '-').replace(regex, "");

ButtonDelegate::ButtonDelegate(std::vector<QString> buttonLabels, QObject *parent)
    : QStyledItemDelegate(parent), _buttonLabels(buttonLabels)
{
}

QWidget *ButtonDelegate::createEditor(QWidget *parent,
                                      const QStyleOptionViewItem &/* option */,
                                      const QModelIndex &/* index */) const
{
    auto layout = new QVBoxLayout();
    layout->setSpacing(4);
    layout->setContentsMargins(4, 4, 4, 4);

    for (auto &text : _buttonLabels) {
        auto button = new QPushButton(text);
        button->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Fixed);
        layout->addWidget(button, 0, Qt::AlignmentFlag::AlignTop);
    }

    auto wrapper = new QWidget(parent);
    wrapper->setSizePolicy(QSizePolicy::Preferred, QSizePolicy::Ignored);
    wrapper->setAccessibleName(editor_name);
    wrapper->setStyleSheet(QString("[accessibleName=\"%1\"] { background: none; }").arg(editor_name));
    wrapper->setLayout(layout);

    return wrapper;
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
    int i = 0;
    for (auto &o : widget->children()) {
        if (strcmp(o->metaObject()->className(), "QPushButton") == 0) {
            auto button = static_cast<QPushButton *>(o);
            auto buttonIndex = i++;

            _dataByIndex.insert(
                widget,
                QObject::connect(button, &QPushButton::clicked, [this, index, buttonIndex](bool checked) {
                    emit clicked(index, buttonIndex, checked);
                })
            );
        }
    }
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