/**
* @file dlgcontrollerlearning.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/
#ifndef DLGCONTROLLERLEARNING_H
#define DLGCONTROLLERLEARNING_H

#include <QtGui>
#include <QMenu>
#include <QSignalMapper>

#include "controllers/ui_dlgcontrollerlearning.h"
#include "controllers/controller.h"
#include "controllers/mixxxcontrol.h"
#include "configobject.h"

class ControllerPreset;

class DlgControllerLearning : public QDialog, public Ui::DlgControllerLearning {
    Q_OBJECT
  public:
    DlgControllerLearning(QWidget *parent, Controller *controller);
    virtual ~DlgControllerLearning();

  signals:
    void cancelLearning();
    void learn(MixxxControl control);
    void listenForClicks();
    void stopListeningForClicks();

  public slots:
    // Triggered when user selects a control from the menu.
    void controlChosen(int controlIndex);
    // Triggered when user clicks a control from the GUI
    void controlClicked(ControlObject* pControl);

    // Gets called when a control has just been mapped successfully
    void controlMapped(QString);

  private slots:
    void showControlMenu();

  private:
    QMenu* addSubmenu(QString title, QMenu* pParent=NULL);
    void loadControl(const MixxxControl& control);

    void addControl(QString group, QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addPlayerControl(QString control, QString helpText, QMenu* pMenu,
                          bool deckControls, bool samplerControls, bool addReset=false);
    void addDeckAndSamplerControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addDeckControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addSamplerControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);

    QSet<MixxxControl> m_mappedControls;
    Controller* m_pController;
    QSignalMapper m_actionMapper;
    QMenu m_controlPickerMenu;
    QList<MixxxControl> m_controlsAvailable;
    MixxxControl m_currentControl;
    QString m_deckStr, m_samplerStr, m_resetStr;
};

#endif
