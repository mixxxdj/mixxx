/***************************************************************************
                          dlgprefshoutcast.h  -  description
                             -------------------
    begin                : Thu Jun 7 2007
    copyright            : (C) 2007 by John Sully
                           (C) 2007 by Albert Santoni
    email                : 
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGPREFSHOUTCAST_H
#define DLGPREFSHOUTCAST_H

#include "ui_dlgprefshoutcastdlg.h"
#include "controlobject.h"
#include "configobject.h"

#define PREF_KEY "[Shoutcast]"
#define IDEX_WAVE 0
#define IDEX_FLAC 1
#define IDEX_AIFF 2
#define IDEX_OGG  3
#define IDEX_MP3  4

#define RECORD_READY 0.50f
#define RECORD_ON 1.0f
#define RECORD_OFF 0.0f

class QWidget;
/**
  *@author John Sully
  */

class DlgPrefShoutcast : public QWidget, public Ui::DlgPrefShoutcastDlg  {
    Q_OBJECT
public: 
    DlgPrefShoutcast(QWidget *parent, ConfigObject<ConfigValue> *_config);
    ~DlgPrefShoutcast();
public slots:
    /** Apply changes to widget */
    void slotApply();
/*    void slotBrowseSave();
    void slotEncoding();
    void slotSliderQuality();
    void slotRecordPathChange();
    int getSliderQualityVal();
    void updateTextQuality();*/
signals:
    void apply(const QString &);
private:
/*    void setMetaData();
    void loadMetaData();*/
    
    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;
//    ControlObject* recordControl;
//    bool confirmOverwrite;
//    QString fileTypeExtension;
};

#endif
