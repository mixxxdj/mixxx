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
    void slotBrowseSave();
    void slotBrowseCueSave();
    void slotEnableCueFile(int);
    void slotRecordPathChanged(QString);
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
    ControlObjectThreadMain* recordControl;
    bool confirmOverwrite;
    QString fileTypeExtension;
};

#endif
