/**
* @file dlgcontrollerlearning.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DLGCONTROLLERLEARNING_H
#define DLGCONTROLLERLEARNING_H

// #include <QtCore>
// #include <QtGui>

#include "controllers/ui_dlgcontrollerlearning.h"
#include "configobject.h"
#include "mixxxcontrol.h"

class ControllerPreset;

class DlgControllerLearning : public QDialog, public Ui::DlgControllerLearning {
    Q_OBJECT
  public:
    DlgControllerLearning(QWidget *parent, ControllerPreset *preset);
    ~DlgControllerLearning();
  public slots:
    void begin();   /** Begin the learning process */
    void next();    /** Ask to map the next control */
    void prev();    /** Ask to map the previous control */
    void controlMapped(QString); /** Gets called when a control has just been mapped successfully */
  private:
    void addControl(QString group, QString control, QString helpText);
    void addDeckControl(QString control, QString helpText);
    void addSamplerControl(QString control, QString helpText);
    ControllerPreset* m_pPreset;
    QList<MixxxControl> m_controlsToMap;
    int iCurrentControl; /** Used to iterate through the controls list */
    QShortcut* m_pSkipShortcut;
};

#endif
