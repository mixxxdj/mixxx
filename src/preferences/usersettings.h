#pragma once

#include <QSharedPointer>
#include <QWeakPointer>

#include "preferences/configobject.h"

typedef ConfigObject<ConfigValue> UserSettings;
typedef QSharedPointer<UserSettings> UserSettingsPointer;
typedef QWeakPointer<UserSettings> UserSettingsWeakPointer;

// Helper for defining preference getters/setters.
//
// Produces a method for getting, setting, getting the default value, and
// resetting to default. Assumes m_pConfig is a pointer to the user's
// configuration.
//
// Arguments:
// - name: The prefix to use for constructing the helper methods.
// - type: The type of the preference.
// - preference_group: The ConfigKey group the preference is stored in.
// - preference_item: The ConfigKey item the preference is stored in.
// - default_value: The default value for the preference.
#define DEFINE_PREFERENCE_HELPERS(name, type, preference_group,             \
                                  preference_item, default_value)           \
    inline type get##name() const {                                         \
        return m_pConfig->getValue<type>(                                   \
            ConfigKey(preference_group, preference_item),                   \
            default_value);                                                 \
    }                                                                       \
    inline type get##name##Default() const { return default_value; }        \
    inline void set##name##ToDefault() { set##name(get##name##Default()); } \
    inline void set##name(const type& value) {                              \
        m_pConfig->setValue(ConfigKey(preference_group, preference_item),   \
                            value);                                         \
    }
