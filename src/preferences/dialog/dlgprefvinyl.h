/***************************************************************************
                          dlgprefvinyl.h  -  description
                             -------------------
    begin                : Thu Oct 23 2006
    copyright            : (C) 2006 by Stefan Langhammer
    email                : stefan.langhammer@9elements.com
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFVINYL_H
#define DLGPREFVINYL_H

#include <QSpinBox>
#include <QWidget>

#include "preferences/dialog/ui_dlgprefvinyldlg.h"
#include "preferences/dlgpreferencepage.h"
#include "preferences/usersettings.h"
#include "vinylcontrol/vinylcontrolsignalwidget.h"

class ControlProxy;
class VinylControlManager;

class DlgPrefVinyl : public DlgPreferencePage, Ui::DlgPrefVinylDlg  {
    Q_OBJECT
  public:
    DlgPrefVinyl(QWidget* pParent, VinylControlManager* m_pVCMan, UserSettingsPointer _config);
    virtual ~DlgPrefVinyl();

    QUrl helpUrl() const override;

  public slots:
    void slotUpdate() override;
    void slotApply() override;
    void slotResetToDefaults() override;

    void slotHide() override;
    void slotShow() override;
    void VinylTypeSlotApply();
    void slotVinylGainApply();
    void slotUpdateVinylGain();

  private slots:
    void slotNumDecksChanged(double);
    void slotVinylType1Changed(QString);
    void slotVinylType2Changed(QString);
    void slotVinylType3Changed(QString);
    void slotVinylType4Changed(QString);

  private:
    void setDeckWidgetsVisible(int deck, bool visible);
    void setDeck1WidgetsVisible(bool visible);
    void setDeck2WidgetsVisible(bool visible);
    void setDeck3WidgetsVisible(bool visible);
    void setDeck4WidgetsVisible(bool visible);
    void verifyAndSaveLeadInTime(QSpinBox* widget, QString group, QString vinyl_type);
    int getDefaultLeadIn(QString vinyl_type) const;


    QList<VinylControlSignalWidget*> m_signalWidgets;

    VinylControlManager* m_pVCManager;
    UserSettingsPointer config;
    QList<ControlProxy*> m_COSpeeds;
    ControlProxy* m_pNumDecks;
};

#endif
