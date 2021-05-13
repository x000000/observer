#include "action-config.hpp"
#include "settings-model.hpp"
#include <QSize>

#define item_object() _settings.actions[index.row()]
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

static std::vector<std::string> to_std_vector(const QStringList &vec)
{
    std::vector<std::string> vector;
    vector.reserve(vec.size());

    for (auto it = vec.begin(); it < vec.end(); it++) {
        vector.push_back(it->toStdString());
    }
    return vector;
}

static QStringList to_string_list(const std::vector<std::string> &vec)
{
    QStringList list;
    list.reserve(vec.size());

    for (auto it = vec.begin(); it < vec.end(); it++) {
        list.push_back(QString::fromStdString(*it));
    }
    return list;
}


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
                    return users.isEmpty() ? "[Any]" : users;
                }

                case Columns::Expr:
                    return QString::fromStdString(item_field(expression));

                case Columns::Timeout:
                    return QString("%1").arg(item_field(timeout));

                case Columns::Del:
                case Columns::Active:
                case Columns::Action:
                    return QString();

                default:
                    return QString("Row%1, Column%2").arg(row + 1).arg(col + 1);
            }

        case Qt::CheckStateRole:
            if (col == Columns::Active) {
                return item_field(is_active) ? Qt::Checked : Qt::Unchecked;
            }
            break;

        case Qt::EditRole:
            switch (col) {
                case Columns::Users: {
                    auto list = to_string_list(item_field(users));
                    return list.isEmpty() ? list : list << "";
                }

                case Columns::Expr:
                    return QString::fromStdString(item_field(expression));

                case Columns::Timeout:
                    return item_field(timeout);

                case Columns::Action:
                    auto type  = item_field(type);
                    auto items = to_string_list(item_field(sceneitems));
                    return QVariant::fromValue(ActionConfig(type, items));
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
                item_field(is_active) = value.toBool();
                return true;
            }
            break;

        case Columns::Users: {
            item_field(users) = to_std_vector(value.toStringList());
            emit dataChanged(index, index, { role });
            return true;
        }

        case Columns::Expr:
            item_field(expression) = value.toString().trimmed().toStdString();
            return true;

        case Columns::Timeout:
            item_field(timeout) = value.toInt();
            return true;

        case Columns::Action:
            auto config = qvariant_cast<ActionConfig>(value);
            item_field(type)       = config.type();
            item_field(sceneitems) = to_std_vector(config.sources());
            emit dataChanged(index, index, { role });
            return true;
    }
    return false;
}

QVariant ObserverSettingsModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (role == Qt::DisplayRole && orientation == Qt::Horizontal) {
        switch (section) {
            /*
            case Columns::Active:
                return QString("");
             */
            case Columns::Users:
                return QString("Users");
            case Columns::Expr:
                return QString("Pattern");
            case Columns::Timeout:
                return QString("Timeout");
            case Columns::Action:
                return QString("Action");
        }
    }
    return QVariant();
}

Qt::ItemFlags ObserverSettingsModel::flags(const QModelIndex &index) const
{
    switch (index.column()) {
        case Columns::Active:
            return Qt::ItemIsEnabled | Qt::ItemIsUserCheckable;
        case Columns::Users:
        case Columns::Expr:
        case Columns::Timeout:
            return Qt::ItemIsEnabled | Qt::ItemIsEditable | Qt::ItemIsSelectable;
    }
    return Qt::ItemIsEnabled;
}

void ObserverSettingsModel::addItem()
{
    auto row = rowCount();
    beginInsertRows(QModelIndex(), row, row);

    _settings.actions.push_back({
        .type = Toggle,
    });

    endInsertRows();
}

void ObserverSettingsModel::removeItem(int index)
{
    beginRemoveRows(QModelIndex(), index, index);
    _settings.actions.erase(_settings.actions.begin() + index);
    endRemoveRows();
}
