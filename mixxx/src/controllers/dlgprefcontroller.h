/**
* @file dlgprefcontroller.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
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
#ifndef DLGPREFCONTROLLER_H_
#define DLGPREFCONTROLLER_H_

#include <QtGui>
#include "controllers/ui_dlgprefcontrollerdlg.h"
#include "configobject.h"

//Forward declarations
class Controller;
class ControllerManager;

class DlgPrefController : public QWidget {
    Q_OBJECT
public:
    DlgPrefController(QWidget *parent, Controller* controller,
                        ControllerManager* controllerManager,
                        ConfigObject<ConfigValue> *pConfig);
    virtual ~DlgPrefController();

public slots:
    void slotUpdate();
    void slotApply();
    void slotDeviceState(int state);
    void slotLoadPreset(const QString &name);

    //Mark that we need to apply the settings.
    void slotDirty ();
    
signals:
    void deviceStateChanged(DlgPrefController*, bool);
    void openController(bool);
    void closeController(bool);
    void loadPreset(Controller* pController, QString controllerName, bool force);
    void applyPreset();

protected:
    QGridLayout *layout;
    QSpacerItem *verticalSpacer;

private:
    void savePreset(QString path);
    void enumeratePresets();

    void enableDevice();
    void disableDevice();
    
    bool m_bDirty;
    int currentGroupRow;
    ConfigObject<ConfigValue> *m_pConfig;
    Controller* m_pController;
    ControllerManager* m_pControllerManager;

    Ui::DlgPrefControllerDlg m_ui;
};

#endif /*DLGPREFCONTROLLER_H_*/
