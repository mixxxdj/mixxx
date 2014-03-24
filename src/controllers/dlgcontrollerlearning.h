/**
* @file dlgcontrollerlearning.h
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/
#ifndef DLGCONTROLLERLEARNING_H
#define DLGCONTROLLERLEARNING_H

#include <QDialog>
#include <QList>
#include <QMenu>
#include <QSet>
#include <QSignalMapper>
#include <QString>
#include <QTimer>

#include "controllers/ui_dlgcontrollerlearning.h"
#include "controllers/midi/midicontroller.h"
#include "controllers/hid/hidcontroller.h"
#include "controllers/bulk/bulkcontroller.h"
#include "controllers/midi/midimessage.h"
#include "controllers/controller.h"
#include "controllers/controllervisitor.h"
#include "controllers/mixxxcontrol.h"
#include "configobject.h"

class ControllerPreset;

class DlgControllerLearning : public QDialog,
                              public ControllerVisitor,
                              public Ui::DlgControllerLearning {
    Q_OBJECT
  public:
    DlgControllerLearning(QWidget *parent, Controller *controller);
    virtual ~DlgControllerLearning();

    void visit(MidiController* pController);
    void visit(HidController* pController);
    void visit(BulkController* pController);

  signals:
    void learnTemporaryInputMappings(const MixxxControl& control,
                                     const MidiKeyAndOptionsList& mappings);
    void clearTemporaryInputMappings();
    void commitTemporaryInputMappings();

    void startLearning();
    void stopLearning();
    void listenForClicks();
    void stopListeningForClicks();

  public slots:
    // Triggered when user selects a control from the menu.
    void controlChosen(int controlIndex);
    // Triggered when user clicks a control from the GUI
    void controlClicked(ControlObject* pControl);

    void slotMessageReceived(unsigned char status,
                             unsigned char control,
                             unsigned char value);

    void slotTimerExpired();
    void slotUndo();
    void slotMidiOptionsChanged();

  private slots:
    void showControlMenu();

  private:
    QMenu* addSubmenu(QString title, QMenu* pParent=NULL);
    void loadControl(const MixxxControl& control);

    void addControl(QString group, QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addPlayerControl(QString control, QString helpText, QMenu* pMenu,
                          bool deckControls, bool samplerControls, bool previewdeckControls, bool addReset=false);
    void addDeckAndSamplerControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addDeckAndPreviewDeckControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addDeckAndSamplerAndPreviewDeckControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addDeckControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addSamplerControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void addPreviewDeckControl(QString control, QString helpText, QMenu* pMenu, bool addReset=false);
    void resetMapping(bool commit);

    QSet<MixxxControl> m_mappedControls;
    MidiController* m_pMidiController;
    QSignalMapper m_actionMapper;
    QMenu m_controlPickerMenu;
    QList<MixxxControl> m_controlsAvailable;
    MixxxControl m_currentControl;

    bool m_messagesLearned;
    QTimer m_lastMessageTimer;
    QList<QPair<MidiKey, unsigned char> > m_messages;
    MidiKeyAndOptionsList m_mappings;

    QString m_deckStr, m_previewdeckStr, m_samplerStr, m_resetStr;
};

#endif
