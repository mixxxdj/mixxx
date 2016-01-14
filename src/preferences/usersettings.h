#ifndef PREFERENCES_USERSETTINGS_H
#define PREFERENCES_USERSETTINGS_H

#include <QSharedPointer>
#include <QWeakPointer>

#include "configobject.h"

typedef ConfigObject<ConfigValue> UserSettings;
typedef QSharedPointer<UserSettings> UserSettingsPointer;
typedef QWeakPointer<UserSettings> UserSettingsWeakPointer;

#endif /* PREFERENCES_USERSETTINGS_H */
