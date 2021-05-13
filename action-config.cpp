#include "action-config.hpp"

ActionConfig::ActionConfig()
{
}

ActionConfig::ActionConfig(const ActionType type, const QStringList sources) : _type(type), _sources(sources)
{
}