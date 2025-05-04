#pragma once


// Starts a backup of the Mixxx settings directory if enabled in config.
// Excludes the "analysis" subfolder, and saves the archive to ~/Documents/Mixxx-BackUps.
void createSettingsBackUp(UserSettingsPointer m_pConfig);
