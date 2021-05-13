#pragma once

#include <QVariant>
#include "observer.hpp"

class ActionConfig
{
public:
    explicit ActionConfig();
    explicit ActionConfig(const ActionType type, const QStringList sources);

    ActionType type() { return _type; }
    void setType(const ActionType type) { _type = type; }

    QStringList sources() { return _sources; }
    void setSceneitems(const QStringList sources) { _sources = sources; }

private:
    ActionType _type;
    QStringList _sources;
};

Q_DECLARE_METATYPE(ActionConfig)