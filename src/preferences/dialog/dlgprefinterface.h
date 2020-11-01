/***************************************************************************
                          dlgprefcontrols.h  -  description
                             -------------------
    begin                : Sat Jul 5 2003
    copyright            : (C) 2003 by Tue & Ken Haste Andersen
    email                : haste@diku.dk
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFCONTROLS_H
#define DLGPREFCONTROLS_H

#include <QWidget>

#include "preferences/constants.h"
#include "preferences/dialog/ui_dlgprefinterfacedlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class ControlProxy;
class ControlPotmeter;
class SkinLoader;
class PlayerManager;
class MixxxMainWindow;
class ControlObject;

/**
  *@author Tue & Ken Haste Andersen
  */

class DlgPrefInterface : public DlgPreferencePage, public Ui::DlgPrefControlsDlg  {
    Q_OBJECT
  public:
    DlgPrefInterface(QWidget *parent, MixxxMainWindow *mixxx,
                    SkinLoader* pSkinLoader, UserSettingsPointer pConfig);
    ~DlgPrefInterface() override = default;

  public slots:
    void slotUpdate();
    void slotApply();
    void slotResetToDefaults();

    void slotSetTooltips();
    void slotSetSkinDescription(QString skin);
    void slotSetSkin(int);
    void slotSetScheme(int);
    void slotUpdateSchemes();
    void slotSetScaleFactor(double newValue);
    void slotSetScaleFactorAuto(bool checked);

  private:
    void notifyRebootNecessary();
    void loadTooltipPreferenceFromConfig();

    // Because the CueDefault list is out of order, we have to set the combo
    // box using the user data, not the index.  Returns the index of the item
    // that has the corresponding userData. If the userdata is not in the list,
    // returns zero.
    int cueDefaultIndexByData(int userData) const;
    QScreen* getScreen() const;

    UserSettingsPointer m_pConfig;
    ControlObject* m_pControlTrackTimeDisplay;
    MixxxMainWindow *m_mixxx;
    SkinLoader* m_pSkinLoader;
    PlayerManager* m_pPlayerManager;

    QString m_skin;
    QString m_skinOnUpdate;
    QString m_colorScheme;
    QString m_localeOnUpdate;
    mixxx::TooltipsPreference m_tooltipMode;
    double m_dScaleFactorAuto;
    bool m_bUseAutoScaleFactor;
    double m_dScaleFactor;
    bool m_bStartWithFullScreen;
    mixxx::ScreenSaverPreference m_screensaverMode;

    bool m_bRebootMixxxView;
};

#endif
