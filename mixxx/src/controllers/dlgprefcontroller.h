/**
* @file dlgprefcontroller.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Mon May 2 2011
* @brief Configuration dialog for a DJ controller
*/

#ifndef DLGPREFCONTROLLER_H_
#define DLGPREFCONTROLLER_H_

#include <QtGui>
#include <QHash>

#include "controllers/controllerpreset.h"
#include "controllers/controllerpresetinfo.h"
#include "controllers/ui_dlgprefcontrollerdlg.h"
#include "configobject.h"

// Forward declarations
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
    // Called when the OK button is pressed.
    virtual void slotApply();
    // Called when the dialog is displayed.
    virtual void slotUpdate();
    virtual void slotDeviceState(int state);
    // Loads the specified XML preset.
    void slotLoadPreset(const QString &name);
    // Mark that we need to apply the settings.
    void slotDirty ();
    // Reload the mappings in the dropdown dialog
    void enumeratePresets();

  signals:
    void deviceStateChanged(DlgPrefController*, bool);
    void openController(Controller* pController);
    void closeController(Controller* pController);
    void loadPreset(Controller* pController, QString controllerName, bool force);

  protected:
    Controller* getController() const {
        return m_pController;
    }
    ControllerManager* getControllerManager() const {
        return m_pControllerManager;
    }
    void addWidgetToLayout(QWidget* pWidget);
    bool isEnabled() const {
        return m_ui.chkEnabledDevice->isChecked();
    }
    Ui::DlgPrefControllerDlg& getUi() {
        return m_ui;
    }

  private slots:
    void slotPresetLoaded(ControllerPresetPointer preset);

  private:
    QString presetShortName(const ControllerPresetPointer pPreset) const;
    QString presetDescription(const ControllerPresetPointer pPreset) const;
    QString presetForumLink(const ControllerPresetPointer pPreset) const;
    QString presetWikiLink(const ControllerPresetPointer pPreset) const;
    void savePreset(QString path);

    void enableDevice();
    void disableDevice();

    Ui::DlgPrefControllerDlg m_ui;
    ConfigObject<ConfigValue>* m_pConfig;
    ControllerManager* m_pControllerManager;
    Controller* m_pController;
    QGridLayout* m_pLayout;
    QSpacerItem* m_pVerticalSpacer;
    bool m_bDirty;
};

#endif /*DLGPREFCONTROLLER_H_*/
