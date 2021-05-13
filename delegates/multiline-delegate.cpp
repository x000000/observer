#include "multiline-delegate.hpp"
#include <QPlainTextEdit>

static const QRegExp line_split_regex(R"([\r\n]+)", Qt::CaseSensitive, QRegExp::RegExp2);

MultilineDelegate::MultilineDelegate(QObject *parent) : QStyledItemDelegate(parent)
{
}

QWidget *MultilineDelegate::createEditor(QWidget *parent,
                                       const QStyleOptionViewItem &option,
                                       const QModelIndex &index) const
{
    auto input = new QPlainTextEdit(parent);
    input->setWordWrapMode(QTextOption::NoWrap);
    input->setLineWrapMode(QPlainTextEdit::NoWrap);

    QObject::connect(input, &QPlainTextEdit::textChanged, [this, input, option, index]() {
        updateEditorGeometry(input, option, index);
    });
    return input;
}

void MultilineDelegate::setEditorData(QWidget *editor, const QModelIndex &index) const
{
    auto value = index.model()->data(index, Qt::EditRole).toStringList();
    auto input = static_cast<QPlainTextEdit*>(editor);
    input->setPlainText(value.join("\n"));
    input->moveCursor(QTextCursor::End);
}

void MultilineDelegate::setModelData(QWidget *editor, QAbstractItemModel *model, const QModelIndex &index) const
{
    auto input = static_cast<QPlainTextEdit*>(editor);
    auto value = input->toPlainText();
    model->setData(index, value.split(line_split_regex, Qt::SkipEmptyParts), Qt::EditRole);
}

void MultilineDelegate::updateEditorGeometry(QWidget *editor,
                                           const QStyleOptionViewItem &option,
                                           const QModelIndex &/* index */) const
{
    auto size = editor->sizeHint();
    auto rect = QRect(option.rect);

    if (size.height() > rect.height()) {
        rect.setHeight(size.height());
    }
    if (size.width() > rect.width()) {
        rect.setHeight(size.width());
    }

    editor->setGeometry(rect);
}
