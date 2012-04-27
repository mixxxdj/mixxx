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
    // Begin the learning process
    void begin();

    void pickControlNext();
    void pickControlDone();
    void mapControlContinue();
    void mapControlBack();
    void controlChosen(int controlIndex);
    void controlClicked(ControlObject* pControl);

    // Gets called when a control has just been mapped successfully
    void controlMapped(QString);
  private:
    void showPickControl();
    void showMapControl();
    void populateComboBox();
    void loadControl(const MixxxControl& control);

    void addControl(QString group, QString control, QString helpText, bool addReset=false);
    void addDeckAndSamplerControl(QString control, QString helpText, bool addReset=false);
    void addDeckControl(QString control, QString helpText, bool addReset=false);
    void addSamplerControl(QString control, QString helpText, bool addReset=false);

    QSet<MixxxControl> m_mappedControls;
    Controller* m_pController;
    QList<MixxxControl> m_controlsAvailable;
    MixxxControl m_currentControl;
    QString m_deckStr, m_samplerStr, m_resetStr;
};

#endif
