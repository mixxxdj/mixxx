#pragma once

#include <QSharedPointer>
#include <QWeakPointer>

#include "preferences/configkey.h"

// Forward declaration of ConfigObject to allow using UserSettingsPointer
// without pulling in the full configobject.h template.
template<typename T>
class ConfigObject;
class ConfigValue;

typedef ConfigObject<ConfigValue> UserSettings;
typedef QSharedPointer<UserSettings> UserSettingsPointer;
typedef QWeakPointer<UserSettings> UserSettingsWeakPointer;
