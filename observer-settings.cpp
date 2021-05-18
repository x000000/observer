#include <obs-module.h>
#include <obs-frontend-api.h>
#include "observer-settings.hpp"
#include "action-config-dialog.hpp"
#include "ui_observer-settings.h"
#include <functional>

#define tr(token) obs_module_text("observer_settings." token)

ObserverSettings::ObserverSettings(observer_settings settings, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui_ObserverSettings)
    , _model(settings)
    , _buttonsDelegate(std::vector<QString> { tr("edit"), tr("delete") })
{
    ui->setupUi(this);

    auto t = ui->tableView;
    t->setItemDelegateForColumn(Columns::Buttons, &_buttonsDelegate);
    t->setModel(&_model);
    t->horizontalHeader()->setSectionResizeMode(Columns::Buttons, QHeaderView::ResizeToContents);
    t->horizontalHeader()->setSectionResizeMode(Columns::Active,  QHeaderView::ResizeToContents);
    t->setColumnWidth(Columns::Users, 160);
    t->setColumnWidth(Columns::Expr,  300);

    for (auto row = 0; row < _model.rowCount(); row++) {
        openEditors(row);
    }
    t->resizeColumnsToContents();

    ui->channel_input->setText(QString::fromStdString(_model.settings()->channel));

    QObject::connect(&_model, &QAbstractItemModel::dataChanged, this, &ObserverSettings::onDataChanged);
    QObject::connect(&_model, &QAbstractItemModel::rowsInserted, this, &ObserverSettings::onRowsInserted);
    QObject::connect(&_buttonsDelegate, &ButtonDelegate::clicked, this, &ObserverSettings::onButtonClicked);

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

void ObserverSettings::openEditors(int row)
{
    auto t = ui->tableView;
    auto deleteIndex = _model.index(row, Columns::Buttons);

    t->openPersistentEditor(deleteIndex);

    EditorReferencedEvent ev(QEvent::Show, t->indexWidget(deleteIndex));
    t->itemDelegate(deleteIndex)->editorEvent(&ev, &_model, QStyleOptionViewItem(), deleteIndex);

    t->resizeRowToContents(row);
}

void edit_action(action_descriptor *action, std::function<void()> fn)
{
    obs_frontend_push_ui_translation(obs_module_get_string);
    auto dialog = new ActionConfigDialog(action);
    obs_frontend_pop_ui_translation();

    QObject::connect(dialog, &QDialog::accepted, [dialog, action, fn] {
        dialog->save(action);
        fn();
    });

    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->open();
}

void ObserverSettings::addActionItem()
{
    std::shared_ptr<action_descriptor> ptr(new action_descriptor {
       .type = Toggle,
    });
    edit_action(ptr.get(), [this, ptr]() {
        _model.addItem(*ptr.get());
    });
}

void ObserverSettings::onButtonClicked(const QModelIndex &index, const int buttonIndex, bool)
{
    switch (buttonIndex) {
        case 0: {
            auto row = index.row();
            auto action = &_model.settings()->actions[row];

            edit_action(action, [row, this]() {
                emit _model.dataChanged(_model.index(row, 0), _model.index(row, Columns::COUNT - 1));
                ui->tableView->resizeColumnsToContents();
            });
        }
            break;

        case 1:
            _model.removeItem(index.row());

            auto t = ui->tableView;
            for (int row = index.row(), l = _model.rowCount(); row < l; row++) {
                auto deleteIndex = _model.index(row, Columns::Buttons);
                _buttonsDelegate.updateIndex(deleteIndex, &_model, t->indexWidget(deleteIndex));
            }
            break;
    }
}
