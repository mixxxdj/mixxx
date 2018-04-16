/***************************************************************************
                          upgrade.h  -  description
                             -------------------
    begin                : Mon Apr 13 2009
    copyright            : (C) 2009 by Sean M. Pappalardo
    email                : pegasus@c64.org
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#ifndef PREFERENCES_UPGRADE_H
#define PREFERENCES_UPGRADE_H

#include "preferences/usersettings.h"

class Upgrade {
  public:
    Upgrade();
    ~Upgrade();

    UserSettingsPointer versionUpgrade(const QString& settingsPath);
    bool isFirstRun() { return m_bFirstRun; };
    bool rescanLibrary() {return m_bRescanLibrary; };

  private:
    bool askReanalyzeBeats();
    bool askReScanLibrary();
    bool m_bFirstRun;
    bool m_bRescanLibrary;
};

#endif /* PREFERENCES_UPGRADE_H */
