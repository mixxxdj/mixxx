/**
* @file dlgprefmappablecontroller.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief Configuration dialog for a DJ controller that supports GUI mapping
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
#ifndef DLGPREFMAPPABLECONTROLLER_H_
#define DLGPREFMAPPABLECONTROLLER_H_

#include "controllers/dlgprefcontroller.h"
#include "controllers/dlgcontrollerlearning.h"
#include "controllers/ui_dlgprefmappablecontrollerdlg.h"

class DlgPrefMappableController : public DlgPrefController {
    Q_OBJECT
    public:
        DlgPrefMappableController(QWidget *parent, Controller* controller,
                          ControllerManager* controllerManager,
                          ConfigObject<ConfigValue> *pConfig);
        ~DlgPrefMappableController() {};

    public slots:
        void slotShowLearnDialog();
        void slotUpdate();
        void slotDeviceState(int state);

        //Input mappings
//         void slotClearAllInputMappings() {};
//         void slotRemoveInputMapping() {};
//         void slotAddInputMapping() {};

    private:
        DlgControllerLearning* m_pDlgControllerLearning;
        Ui::ControllerMappingDlg m_ui;
};

#endif
