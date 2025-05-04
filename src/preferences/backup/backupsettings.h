#pragma once

#include <QString>

#include "preferences/usersettings.h"

namespace mixxxbackupsettings {
static const QString kConfigGroup = "[BackUp]";
static const QString kBackUpEnabled = "BackUpEnabled";
static const QString kBackUpFrequency = "BackUpFrequency";
static const QString kLastBackUp = "LastBackUp";

} // namespace mixxxbackupsettings

void createSettingsBackUp(UserSettingsPointer pConfig);
