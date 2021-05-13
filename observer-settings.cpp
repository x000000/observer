#include "observer-settings.hpp"
#include "ui_observer-settings.h"

ObserverSettings::ObserverSettings(observer_settings settings, QWidget *parent)
    : QWidget(parent), ui(new Ui_ObserverSettings), _model(settings), _deleteButtonDelegate("Delete")
{
    ui->setupUi(this);

    auto t = ui->tableView;
    t->setItemDelegateForColumn(Columns::Del,     &_deleteButtonDelegate);
    t->setItemDelegateForColumn(Columns::Timeout, &_spinboxDelegate);
    t->setItemDelegateForColumn(Columns::Users,   &_multilineDelegate);
    t->setItemDelegateForColumn(Columns::Action,  &_actionDelegate);
    t->setModel(&_model);
    t->horizontalHeader()->setSectionResizeMode(Columns::Del,    QHeaderView::ResizeToContents);
    t->horizontalHeader()->setSectionResizeMode(Columns::Active, QHeaderView::ResizeToContents);
    t->setColumnWidth(Columns::Users,   160);
    t->setColumnWidth(Columns::Expr,    300);
    t->setColumnWidth(Columns::Timeout, 80);

    for (auto row = 0; row < _model.rowCount(); row++) {
        openEditors(row);
    }
    t->resizeColumnToContents(Columns::Action);

    ui->channel_input->setText(QString::fromStdString(_model.settings()->channel));

    QObject::connect(&_model, &QAbstractItemModel::dataChanged, this, &ObserverSettings::onDataChanged);
    QObject::connect(&_model, &QAbstractItemModel::rowsInserted, this, &ObserverSettings::onRowsInserted);
    QObject::connect(&_deleteButtonDelegate, &ButtonDelegate::clicked, this, &ObserverSettings::onDeleteClicked);

    QObject::connect(ui->channel_input, &QLineEdit::textChanged, [this](const QString &str) {
        _model.settings()->channel = str.toStdString();
    });
}

ObserverSettings::~ObserverSettings()
{
    delete ui;
}

void ObserverSettings::onDataChanged(const QModelIndex &tl, const QModelIndex &br, const QVector<int> &)
{
    if (tl == br) {
        ui->tableView->resizeRowToContents(tl.row());
        if (tl.column() == Columns::Users || tl.column() == Columns::Action) {
            ui->tableView->resizeColumnToContents(tl.column());
        }
    }
}

void ObserverSettings::onRowsInserted(const QModelIndex &/* parent */, int first, int last)
{
    for (auto row = first; row <= last; row++) {
        openEditors(row);
    }
    ui->tableView->resizeColumnToContents(Columns::Action);
}

void ObserverSettings::closeEvent(QCloseEvent *event)
{
    QWidget::closeEvent(event);
    emit closed(_dialogResult);
}

void ObserverSettings::openEditors(int row)
{
    auto t = ui->tableView;
    auto deleteIndex = _model.index(row, Columns::Del);
    auto actionIndex = _model.index(row, Columns::Action);

    t->openPersistentEditor(deleteIndex);
    t->openPersistentEditor(actionIndex);

    EditorReferencedEvent ev(QEvent::Show, t->indexWidget(deleteIndex));
    t->itemDelegate(deleteIndex)->editorEvent(&ev, &_model, QStyleOptionViewItem(), deleteIndex);

    ev = EditorReferencedEvent(QEvent::Show, t->indexWidget(actionIndex));
    t->itemDelegate(actionIndex)->editorEvent(&ev, &_model, QStyleOptionViewItem(), actionIndex);

    t->resizeRowToContents(row);
}

void ObserverSettings::addActionItem()
{
    _model.addItem();
}

void ObserverSettings::onDeleteClicked(const QModelIndex &index, bool)
{
    _model.removeItem(index.row());

    auto t = ui->tableView;
    for (int row = index.row(), l = _model.rowCount(); row < l; row++) {
        auto deleteIndex = _model.index(row, Columns::Del);
        auto actionIndex = _model.index(row, Columns::Action);

        _deleteButtonDelegate.updateIndex(deleteIndex, &_model, t->indexWidget(deleteIndex));
        _actionDelegate.updateIndex(actionIndex, &_model, t->indexWidget(actionIndex));
    }
}
