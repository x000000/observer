#include "action-delegate-editor.hpp"
#include "ui_action-delegate-editor.h"
#include "action-config.hpp"
#include <QMetaMethod>
#include <QTextBlock>

static const QRegExp line_split_regex(R"([\r\n]+)", Qt::CaseSensitive, QRegExp::RegExp2);

#define add_item(item) ui->type_select->addItem(QString(#item).replace("::", "_"), item)

typedef void (QComboBox::*IntFn)(int index);

qreal get_full_height(QPlainTextEdit *input)
{
    auto doc    = input->document();
    auto layout = doc->documentLayout();
    auto block  = doc->firstBlock();

    qreal height = doc->documentMargin() * 2;
    while (block.isValid()) {
        height += layout->blockBoundingRect(block).height();
        block = block.next();
    }

    return height;
}

ActionDelegateEditor::ActionDelegateEditor(QWidget *parent)
    : QWidget(parent), ui(new Ui::ActionDelegateEditor)
{
    ui->setupUi(this);

    auto input = ui->sources_txt;
    input->setWordWrapMode(QTextOption::NoWrap);

    auto rgb = input->palette().mid().color().name(QColor::HexRgb);
    input->setStyleSheet(QString("padding:0;background-color:%1;").arg(rgb));

    add_item(ActionType::Toggle);
    add_item(ActionType::Hide);
    add_item(ActionType::Show);

    QObject::connect(input, &QPlainTextEdit::textChanged, [this, input]() {
        input->setFixedHeight(get_full_height(input));
        adjustSize();
        emit sourcesChanged();
    });

    IntFn sig = &QComboBox::currentIndexChanged;
    QObject::connect(ui->type_select, sig, [this](int index) {
        emit typeChanged();
    });
}

ActionDelegateEditor::~ActionDelegateEditor()
{
    delete ui;
}

ActionType ActionDelegateEditor::type() const
{
    return static_cast<ActionType>(ui->type_select->currentData().toInt());
}

QStringList ActionDelegateEditor::sources() const
{
    return ui->sources_txt->toPlainText().split(line_split_regex, Qt::SkipEmptyParts);
}

void ActionDelegateEditor::setValue(const ActionType type, const QStringList sources)
{
    auto index = ui->type_select->findData(type);
    if (ui->type_select->currentIndex() != index) {
        ui->type_select->setCurrentIndex(index);
    }
    auto text  = sources.join("\n");
    auto input = ui->sources_txt;
    if (_pristine || !input->hasFocus() && input->toPlainText() != text) {
        _pristine = false;
        input->setPlainText(text);
        input->setFixedHeight(get_full_height(input));
        adjustSize();
    }
}