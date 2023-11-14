#pragma once

#include <QDialog>
#include <QList>
#include <QString>
#include <QTimer>

#include "controllers/ui_dlgcontrollerlearning.h"
#include "controllers/controlpickermenu.h"
#include "controllers/midi/midimessage.h"

class Controller;
class ControlObject;

//#define CONTROLLERLESSTESTING

/// The controller mapping learning wizard
class DlgControllerLearning : public QDialog,
                              public Ui::DlgControllerLearning {
    Q_OBJECT

  public:
    DlgControllerLearning(QWidget* parent,
            Controller* controller,
            ControlPickerMenu* pControlPickerMenu);
    virtual ~DlgControllerLearning();

  signals:
    void learnTemporaryInputMappings(const MidiInputMappings& mappings);
    void clearTemporaryInputMappings();
    void commitTemporaryInputMappings();

    // Used to notify DlgPrefController that we have learned some new input
    // mappings.
    void inputMappingsLearned(const MidiInputMappings& mappings);

    void startLearning();
    void stopLearning();
    void listenForClicks();
    void stopListeningForClicks();

  public slots:
    // Triggered when the user picks a control from the menu.
    void controlPicked(const ConfigKey& control);
    // Triggered when user clicks a control from the GUI
    void controlClicked(const ConfigKey& controlKey);
    void comboboxIndexChanged(int index);

    void slotMessageReceived(unsigned char status,
                             unsigned char control,
                             unsigned char value);

    void slotCancelLearn();
    void slotChooseControlPressed();
    void slotTimerExpired();
    void slotFirstMessageTimeout();
    void slotRetry();
    void slotStartLearningPressed();
    void slotMidiOptionsChanged();

  private slots:
    void showControlMenu();
#ifdef CONTROLLERLESSTESTING
    void DEBUGFakeMidiMessage();
    void DEBUGFakeMidiMessage();
#endif

  private:
    void loadControl(const ConfigKey& key, const QString& title, QString description);
    void startListening();
    void commitMapping();
    void resetWizard(bool keepCurrentControl = false);
    void populateComboBox();

    Controller* m_pController;
    ControlPickerMenu* m_pControlPickerMenu;
    ConfigKey m_currentControl;
    bool m_messagesLearned;
    QTimer m_firstMessageTimer;
    QTimer m_lastMessageTimer;
    QList<QPair<MidiKey, unsigned char>> m_messages;
    MidiInputMappings m_mappings;
};
