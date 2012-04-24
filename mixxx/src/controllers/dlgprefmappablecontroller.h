/**
* @file dlgprefmappablecontroller.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief Configuration dialog for a DJ controller that supports GUI mapping
*
*/
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
    virtual ~DlgPrefMappableController() {};

  signals:
    void clearInputs();
    void clearOutputs();

  private slots:
    void slotShowLearnDialog();
    void slotUpdate();
    void slotDeviceState(int state);

    // Input mappings
    void clearAllInputBindings();
    //void slotRemoveInputMapping() {};
    //void slotAddInputMapping() {};

    // Output mappings
    void clearAllOutputBindings();

  private:
    DlgControllerLearning* m_pDlgControllerLearning;
    Ui::ControllerMappingDlg m_ui;
};

#endif
