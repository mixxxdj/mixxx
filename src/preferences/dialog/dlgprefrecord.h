/***************************************************************************
                          dlgprefrecord.h  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
    email                : jsully@scs.ryerson.ca
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFRECORD_H
#define DLGPREFRECORD_H

#include <QRadioButton>
#include <QWidget>

#include "preferences/dialog/ui_dlgprefrecorddlg.h"
#include "preferences/usersettings.h"
#include "preferences/dlgpreferencepage.h"

class ControlObject;
class ControlProxy;

class DlgPrefRecord : public DlgPreferencePage, public Ui::DlgPrefRecordDlg  {
    Q_OBJECT
  public:
    DlgPrefRecord(QWidget *parent, UserSettingsPointer _config);
    virtual ~DlgPrefRecord();

  public slots:
    // Apply changes to widget
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();


    void slotEncoding();
    void slotSliderQuality();
    void slotRecordPathChange();
    void slotEnableCueFile(int);
    void slotChangeSplitSize();
    // Dialog to browse for recordings directory
    void slotBrowseRecordingsDir();

  signals:
    void apply(const QString &);

  private:
    void setRecordingFolder();
    void setMetaData();
    void loadMetaData();
    int getSliderQualityVal();
    void updateTextQuality();

    // Pointer to config object
    UserSettingsPointer m_pConfig;
    ControlProxy* m_pRecordControl;
    bool m_bConfirmOverwrite;
    QString fileTypeExtension;
    QRadioButton* m_pRadioOgg;
    QRadioButton* m_pRadioMp3;
    QRadioButton* m_pRadioAiff;
    QRadioButton* m_pRadioFlac;
    QRadioButton* m_pRadioWav;
};

#endif
