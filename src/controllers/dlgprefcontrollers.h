#ifndef DLGPREFCONTROLLERS_H
#define DLGPREFCONTROLLERS_H

#include <QTreeWidgetItem>
#include <QSignalMapper>

#include "preferences/usersettings.h"
#include "controllers/ui_dlgprefcontrollersdlg.h"
#include "preferences/dlgpreferencepage.h"

class DlgPreferences;
class DlgPrefController;
class ControllerManager;

class DlgPrefControllers : public DlgPreferencePage, public Ui::DlgPrefControllersDlg {
    Q_OBJECT
  public:
    DlgPrefControllers(DlgPreferences* pDlgPreferences,
                       UserSettingsPointer pConfig,
                       ControllerManager* pControllerManager,
                       QTreeWidgetItem* pControllerTreeItem);
    virtual ~DlgPrefControllers();

    bool handleTreeItemClick(QTreeWidgetItem* clickedItem);

  public slots:
    void slotUpdate();
    void slotApply();
    void slotCancel();

  private slots:
    void rescanControllers();
    void slotHighlightDevice(DlgPrefController* dialog, bool enabled);
    void slotOpenLocalFile(const QString& file);

  private:
    void destroyControllerWidgets();
    void setupControllerWidgets();

    DlgPreferences* m_pDlgPreferences;
    UserSettingsPointer m_pConfig;
    ControllerManager* m_pControllerManager;
    QTreeWidgetItem* m_pControllerTreeItem;
    QList<DlgPrefController*> m_controllerWindows;
    QList<QTreeWidgetItem*> m_controllerTreeItems;
    QSignalMapper m_buttonMapper;
};

#endif /* DLGPREFCONTROLLERS_H */
