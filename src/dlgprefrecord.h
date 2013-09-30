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

#include "ui_dlgprefrecorddlg.h"
#include "configobject.h"


class ControlObject;
class ControlObjectThreadMain;



class QWidget;
/**
  *@author John Sully
  */

class DlgPrefRecord : public QWidget, public Ui::DlgPrefRecordDlg  {
    Q_OBJECT
public: 
    DlgPrefRecord(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefRecord();
public slots:
    /** Apply changes to widget */
    void slotApply();
    void slotUpdate();
    void slotEncoding();
    void slotSliderQuality();
    void slotRecordPathChange();
    void slotEnableCueFile(int);
    void slotChangeSplitSize();
    /** Dialog to browse for recordings directory */
    void slotBrowseRecordingsDir();
signals:
    void apply(const QString &);
private:
    void setRecordingFolder();
    void setMetaData();
    void loadMetaData();
    int getSliderQualityVal();
    void updateTextQuality();

    /** Pointer to config object */
    ConfigObject<ConfigValue>* m_pConfig;
    ControlObjectThreadMain* m_pRecordControl;
    bool m_bConfirmOverwrite;
    QString fileTypeExtension;
    QRadioButton* m_pRadioOgg;
    QRadioButton* m_pRadioMp3;
    QRadioButton* m_pRadioAiff;
    QRadioButton* m_pRadioFlac;
    QRadioButton* m_pRadioWav;
};

#endif
