#include "action-config.hpp"
#include "settings-model.hpp"
#include <obs-module.h>
#include <QSize>

#define tr(token) obs_module_text("observer_settings." token)
#define item_field(field) _settings.actions[index.row()].field

static QString implode(const std::vector<std::string> &vec, const char *delim = "\n")
{
    QString str;
    for (auto it = vec.begin(); it < vec.end(); it++) {
        str.append(it->c_str()).append(delim);
    }
    if (!str.isEmpty()) {
        auto len = strlen(delim);
        str.remove(str.length() - len, len);
    }
    return str;
}

#define add_enum(item) { item, #item }

static QMap<int, std::string> action_labels {
    add_enum(ActionType::Toggle),
    add_enum(ActionType::Hide),
    add_enum(ActionType::Show),
};

ObserverSettingsModel::ObserverSettingsModel(observer_settings settings, QObject *parent)
    : QAbstractTableModel(parent), _settings(settings)
{
}

int ObserverSettingsModel::columnCount(const QModelIndex & /* parent */) const
{
    return Columns::COUNT;
}

int ObserverSettingsModel::rowCount(const QModelIndex & /* parent */) const
{
    return _settings.actions.size();
}

QVariant ObserverSettingsModel::data(const QModelIndex &index, int role) const
{
    int row = index.row();
    int col = index.column();

    switch (role) {
        case Qt::DisplayRole:
            switch (col) {
                case Columns::Users: {
                    auto users = implode(item_field(users));
                    return users.isEmpty() ? tr("any_user") : users;
                }

                case Columns::Expr: {
                    auto str = QString::fromStdString(item_field(expression)).prepend('/').append('/');
                    return item_field(ignore_case) ? str.append('i') : str;
                }

                case Columns::Action: {
                    auto sources = implode(item_field(sceneitems));
                    if (sources.isEmpty()) {
                        return QString();
                    }

                    auto timeout = item_field(timeout);

                    return QString::fromStdString(timeout > 0 ? tr("action_to") : tr("action"))
                        .arg(obs_module_text(action_labels.value(item_field(type)).c_str()))
                        .arg(sources)
                        .arg(timeout);
                }

                case Columns::Buttons:
                case Columns::Active:
                    return QString();

                default:
                    return QString("Row%1, Column%2").arg(row + 1).arg(col + 1);
            }

        case Qt::CheckStateRole:
            if (col == Columns::Active) {
                return item_field(active) ? Qt::Checked : Qt::Unchecked;
            }
            break;
    }
    return QVariant();
}

bool ObserverSettingsModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    switch (index.column()) {
        case Columns::Active:
            if (role == Qt::CheckStateRole) {
                item_field(active) = value.toBool();
                return true;
            }
            break;
    }
    return false;
}

QVariant ObserverSettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            case Columns::Users:
                return QString(tr("head_users"));
            case Columns::Expr:
                return QString(tr("head_expression"));
            case Columns::Action:
                return QString(tr("head_action"));
        }
    }
    return QVariant();
}

Qt::ItemFlags ObserverSettingsModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
        case Columns::Active:
            return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
    }
    return Qt::ItemIsEnabled;
}

void ObserverSettingsModel::addItem(const action_descriptor action)
{
    auto row = rowCount();
    beginInsertRows(QModelIndex(), row, row);

    _settings.actions.push_back(action);

    endInsertRows();
}

void ObserverSettingsModel::removeItem(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    _settings.actions.erase(_settings.actions.begin() + index);
    endRemoveRows();
}
