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

#include "dlgprefrecorddlg.h"
#include "configobject.h"

#define PREF_KEY "[Recording]"
#define IDEX_WAVE 0
#define IDEX_FLAC 1
#define IDEX_AIFF 2
#define IDEX_OGG  3
#define IDEX_MP3  4

class QWidget;
/**
  *@author John Sully
  */

class DlgPrefRecord : public DlgPrefRecordDlg  {
    Q_OBJECT
public: 
    DlgPrefRecord(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefRecord();
public slots:
    /** Apply changes to widget */
    void slotApply();
    void slotBrowseSave();
    void slotEncoding();
    void slotSliderQuality();
    void slotRecordPathChange();
    int getSliderQualityVal();
    void updateTextQuality();
signals:
    void apply(const QString &);
private:
    void setMetaData();
    void loadMetaData();
    
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
    bool confirmOverwrite;
    QString fileTypeExtension;
};

#endif
