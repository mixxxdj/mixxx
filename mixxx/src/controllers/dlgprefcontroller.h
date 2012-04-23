/**
* @file dlgprefcontroller.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
*
*/
#ifndef DLGPREFCONTROLLER_H_
#define DLGPREFCONTROLLER_H_

#include <QtGui>
#include "controllers/controller.h"
#include "controllers/ui_dlgprefcontrollerdlg.h"
#include "configobject.h"

//Forward declarations
class ControllerManager;

class DlgPrefController : public QWidget {
    Q_OBJECT
public:
    DlgPrefController(QWidget *parent, Controller* controller,
                        ControllerManager* controllerManager,
                        ConfigObject<ConfigValue> *pConfig);
    virtual ~DlgPrefController();

public slots:
    void slotApply();
    virtual void slotUpdate();
    virtual void slotDeviceState(int state);
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

    Controller* m_pController;

    Ui::DlgPrefControllerDlg m_ui;

private:
    void savePreset(QString path);
    void enumeratePresets();

    void enableDevice();
    void disableDevice();

    bool m_bDirty;
    int currentGroupRow;
    ConfigObject<ConfigValue> *m_pConfig;
    ControllerManager* m_pControllerManager;
};

#endif /*DLGPREFCONTROLLER_H_*/
