/***************************************************************************
                          dlgprefcrossfader.h  -  description
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

#ifndef DLGPREFCROSSFADER_H
#define DLGPREFCROSSFADER_H

#include <QWidget>

#include "ui_dlgprefcrossfaderdlg.h"
#include "configobject.h"
#include "controlobjectthread.h"
#include "preferences/dlgpreferencepage.h"

#define MIXXX_XFADER_STEEPNESS_COEFF 8.0

/**
  *@author John Sully
  */

class DlgPrefCrossfader : public DlgPreferencePage, public Ui::DlgPrefCrossfaderDlg  {
    Q_OBJECT
  public:
    DlgPrefCrossfader(QWidget *parent, ConfigObject<ConfigValue> *_config);
    virtual ~DlgPrefCrossfader();

  public slots:
    /** Update X-Fader */
    void slotUpdateXFader();
    /** Apply changes to widget */
    void slotApply();
    void slotUpdate();
    void slotResetToDefaults();

  signals:
    void apply(const QString &);

  private:
    void loadSettings();
    void drawXfaderDisplay();

    /** Pointer to config object */
    ConfigObject<ConfigValue> *config;

    QGraphicsScene *m_pxfScene;

    /** X-fader values */
    double m_xFaderMode, m_transform, m_cal;

    ControlObjectThread m_COTMode;
    ControlObjectThread m_COTCurve;
    ControlObjectThread m_COTCalibration;
    ControlObjectThread m_COTReverse;
    ControlObjectThread m_COTCrossfader;

    bool m_xFaderReverse;
};

#endif
