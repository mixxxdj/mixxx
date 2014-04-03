/**
* @file dlgcontrollerlearning.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/

#include "controlobject.h"
#include "controllers/dlgcontrollerlearning.h"
#include "controllers/learningutils.h"
#include "controllers/midi/midiutils.h"

DlgControllerLearning::DlgControllerLearning(QWidget * parent,
                                             Controller* controller)
        : QDialog(parent),
          m_pController(controller),
          m_pMidiController(NULL),
          m_controlPickerMenu(this),
          m_messagesLearned(false) {
    qRegisterMetaType<MidiInputMappings>("MidiInputMappings");

    setupUi(this);
    labelMappedTo->setText("");
    // Ensure the first page is always shown regardless of the last page shown
    // when the .ui file was saved.
    stackedWidget->setCurrentIndex(0);

    // Delete this dialog when its closed. We don't want any persistence.
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    connect(&m_controlPickerMenu, SIGNAL(controlPicked(ConfigKey)),
            this, SLOT(controlPicked(ConfigKey)));

    connect(pushButtonChooseControl, SIGNAL(clicked()), this, SLOT(showControlMenu()));
    connect(pushButtonClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(pushButtonClose_2, SIGNAL(clicked()), this, SLOT(close()));
    connect(pushButtonRetry, SIGNAL(clicked()), this, SLOT(slotRetry()));
    connect(pushButtonStartLearn, SIGNAL(clicked()), this, SLOT(slotStartLearningPressed()));
    connect(pushButtonLearnAnother, SIGNAL(clicked()), this, SLOT(slotChooseControlPressed()));
    connect(pushButtonFakeControl, SIGNAL(clicked()), this, SLOT(DEBUGFakeMidiMessage()));
    connect(pushButtonFakeControl2, SIGNAL(clicked()), this, SLOT(DEBUGFakeMidiMessage2()));

    // We only want to listen to clicked() so we don't fire
    // slotMidiOptionsChanged when we change the checkboxes programmatically.
    connect(midiOptionSwitchMode, SIGNAL(clicked()),
            this, SLOT(slotMidiOptionsChanged()));
    connect(midiOptionSoftTakeover, SIGNAL(clicked()),
            this, SLOT(slotMidiOptionsChanged()));
    connect(midiOptionInvert, SIGNAL(clicked()),
            this, SLOT(slotMidiOptionsChanged()));
    connect(midiOptionSelectKnob, SIGNAL(clicked()),
            this, SLOT(slotMidiOptionsChanged()));

    slotChooseControlPressed();

    // Wait 1 second until we detect the control the user moved.
    m_lastMessageTimer.setInterval(1500);
    m_lastMessageTimer.setSingleShot(true);
    connect(&m_lastMessageTimer, SIGNAL(timeout()),
            this, SLOT(slotTimerExpired()));

    m_firstMessageTimer.setInterval(7000);
    m_firstMessageTimer.setSingleShot(true);
    connect(&m_firstMessageTimer, SIGNAL(timeout()),
            this, SLOT(slotFirstMessageTimeout()));
}

void DlgControllerLearning::resetWizard(bool keepCurrentControl) {
    qDebug() << "resetting wizard";
    emit(clearTemporaryInputMappings());

    if (!keepCurrentControl) {
        m_currentControl = ConfigKey();
        m_currentDescription = "";
    }
    m_messagesLearned = false;
    m_messages.clear();
    m_mappings.clear();
    midiOptionInvert->setChecked(false);
    midiOptionSelectKnob->setChecked(false);
    midiOptionSoftTakeover->setChecked(false);
    midiOptionSwitchMode->setChecked(false);

    progressBarWiggleFeedback->hide();

    labelMappedTo->setText("");
    labelErrorText->setText("");
}

void DlgControllerLearning::slotChooseControlPressed() {
    qDebug() << "choose button pressed";
    resetWizard();
    stackedWidget->setCurrentIndex(page1Choose);
    startListening();
}

void DlgControllerLearning::startListening() {
    // Start listening as soon as we're on this page -- that way advanced
    // users don't have to specifically click the "Learn" button.
    // Get the underlying type of the Controller. This will call
    // one of the visit() methods below immediately.
    qDebug() << "listening for midi input";
    m_pController->accept(this);
    emit(listenForClicks());
}

void DlgControllerLearning::slotStartLearningPressed() {
    qDebug() << "starting to learn.... (initiate timeout)";
    if (m_currentControl.isNull()) {
        qDebug() << "nothing selected...";
        return;
    }
    m_firstMessageTimer.start();
    stackedWidget->setCurrentIndex(page2Learn);
}

void DlgControllerLearning::DEBUGFakeMidiMessage() {
    slotMessageReceived(MIDI_CC, 0x20, 0x41);
}

void DlgControllerLearning::DEBUGFakeMidiMessage2() {
    slotMessageReceived(MIDI_CC, 0x20, 0x3F);
}

void DlgControllerLearning::slotMessageReceived(unsigned char status,
                                                unsigned char control,
                                                unsigned char value) {
    qDebug() << "got a message";
    // Ignore message since we don't have a control yet.
    if (m_currentControl.isNull()) {
        return;
    }

    // Ignore message since we already learned a mapping for this control.
    if (m_messagesLearned) {
        return;
    }

    MidiKey key;
    key.status = status;
    key.control = control;

    // Ignore all standard MIDI System Real-Time Messages because they
    // are continuously sent and prevent mapping of the pressed key.
    if (MidiUtils::isClockSignal(key)) {
        return;
    }

    if (m_messages.length() == 0) {
        // If an advanced user started wiggling a control without bothering to
        // click the Learn button, take them to the learning screen.
        stackedWidget->setCurrentIndex(page2Learn);
    }

    if (status == MIDI_CC) {
        if (progressBarWiggleFeedback->isVisible()) {
            progressBarWiggleFeedback->setValue(
                    progressBarWiggleFeedback->value() + 1);
        } else {
            progressBarWiggleFeedback->setValue(0);
            progressBarWiggleFeedback->setMinimum(0);
            progressBarWiggleFeedback->setMaximum(10);
            progressBarWiggleFeedback->show();
        }
    }

    m_messages.append(QPair<MidiKey, unsigned char>(key, value));
    // We got a message, so we can cancel the taking-too-long timeout.
    m_firstMessageTimer.stop();

    // Unless this is a MIDI_CC and the progress bar is full, restart the
    // timer.  That way the user won't just push buttons forever and wonder
    // why the wizard never advances.
    if (status != MIDI_CC || progressBarWiggleFeedback->value() != 10) {
        m_lastMessageTimer.start();
    }
}

void DlgControllerLearning::slotFirstMessageTimeout() {
    qDebug() << "never got anything?";
    if (m_messages.length() == 0) {
        qDebug() << "didn't heard anything";
        labelErrorText->setText(tr("Didn't get any midi messages.  Please try again."));
        m_messages.clear();
        // No need to reset everything.
        stackedWidget->setCurrentIndex(page1Choose);
        startListening();
    } else {
        qWarning() << "we shouldn't time out if we never got anything";
    }
}

void DlgControllerLearning::slotTimerExpired() {
    // It's been a timer interval since we last got a message. Let's try to
    // detect mappings.
    qDebug() << "timer expired";
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(m_messages);

    // Add control and description info to each learned input mapping.
    for (MidiInputMappings::iterator it = mappings.begin();
         it != mappings.end(); ++it) {
        MidiInputMapping& mapping = *it;
        mapping.control = m_currentControl;
        mapping.description = QString("MIDI Learned from %1 messages.")
                .arg(m_messages.size());
    }

    if (mappings.isEmpty()) {
        qDebug() << "didn't heard anything.";
        labelErrorText->setText(tr("Unable to detect a mapping -- please try again. Be sure to only touch one control at once."));
        m_messages.clear();
        // Don't reset the wizard.
        stackedWidget->setCurrentIndex(page1Choose);
        startListening();
        return;
    }

    m_messagesLearned = true;
    m_mappings = mappings;
    pushButtonRetry->setEnabled(true);
    emit(learnTemporaryInputMappings(m_mappings));

    QString midiControl = "";
    bool first = true;
    foreach (const MidiInputMapping& mapping, m_mappings) {
        unsigned char opCode = MidiUtils::opCodeFromStatus(mapping.key.status);
        bool twoBytes = MidiUtils::isMessageTwoBytes(opCode);
        QString mappingStr = twoBytes ? QString("Status: 0x%1 Control: 0x%2 Options: 0x%03")
                .arg(QString::number(mapping.key.status, 16).toUpper(),
                     QString::number(mapping.key.control, 16).toUpper()
                     .rightJustified(2, '0'),
                     QString::number(mapping.options.all, 16).toUpper()
                     .rightJustified(2, '0')) :
                QString("0x%1 0x%2")
                .arg(QString::number(mapping.key.status, 16).toUpper(),
                     QString::number(mapping.options.all, 16).toUpper()
                     .rightJustified(2, '0'));

        // Set the debug string and "Advanced MIDI Options" group using the
        // first mapping.
        if (first) {
            midiControl = mappingStr;
            MidiOptions options = mapping.options;
            midiOptionInvert->setChecked(options.invert);
            midiOptionSelectKnob->setChecked(options.selectknob);
            midiOptionSoftTakeover->setChecked(options.soft_takeover);
            midiOptionSwitchMode->setChecked(options.sw);
            first = false;
        }

        qDebug() << "DlgControllerLearning learned input mapping:" << mappingStr;
    }
    qDebug() << "I think we got something!";

    QString mapMessage = QString("%1 %2").arg(tr("Successfully mapped to:"),
                                              midiControl);
    labelMappedTo->setText(mapMessage);
    stackedWidget->setCurrentIndex(page3Confirm);
}

void DlgControllerLearning::slotRetry() {
    // If the user hit undo, instruct the controller to forget the mapping we
    // just added.
    if (m_messagesLearned) {
        // Reset, but keep the control currently being learned.
        resetWizard(false);
    }
    stackedWidget->setCurrentIndex(page2Learn);
}

void DlgControllerLearning::slotMidiOptionsChanged() {
    if (!m_messagesLearned) {
        // This shouldn't happen because we disable the MIDI options when a
        // message has not been learned.
        return;
    }

    emit(clearTemporaryInputMappings());

    // Go over every mapping and set its MIDI options to match the user's
    // choices.
    for (MidiInputMappings::iterator it = m_mappings.begin();
         it != m_mappings.end(); ++it) {
        MidiOptions& options = it->options;
        options.sw = midiOptionSwitchMode->isChecked();
        options.soft_takeover = midiOptionSoftTakeover->isChecked();
        options.invert = midiOptionInvert->isChecked();
        options.selectknob = midiOptionSelectKnob->isChecked();
    }

    emit(learnTemporaryInputMappings(m_mappings));
}

void DlgControllerLearning::commitMapping() {
    emit(commitTemporaryInputMappings());
    emit(inputMappingsLearned(m_mappings));
}

void DlgControllerLearning::visit(MidiController* pMidiController) {
    m_pMidiController = pMidiController;

    connect(m_pMidiController, SIGNAL(messageReceived(unsigned char, unsigned char, unsigned char)),
            this, SLOT(slotMessageReceived(unsigned char, unsigned char, unsigned char)));

    connect(this, SIGNAL(learnTemporaryInputMappings(MidiInputMappings)),
            m_pMidiController, SLOT(learnTemporaryInputMappings(MidiInputMappings)));
    connect(this, SIGNAL(clearTemporaryInputMappings()),
            m_pMidiController, SLOT(clearTemporaryInputMappings()));

    connect(this, SIGNAL(commitTemporaryInputMappings()),
            m_pMidiController, SLOT(commitTemporaryInputMappings()));
    connect(this, SIGNAL(startLearning()),
            m_pMidiController, SLOT(startLearning()));
    connect(this, SIGNAL(stopLearning()),
            m_pMidiController, SLOT(stopLearning()));

    emit(startLearning());
}

void DlgControllerLearning::visit(HidController* pHidController) {
    qDebug() << "ERROR: DlgControllerLearning does not support HID devices.";
    Q_UNUSED(pHidController);
}

void DlgControllerLearning::visit(BulkController* pBulkController) {
    qDebug() << "ERROR: DlgControllerLearning does not support Bulk devices.";
    Q_UNUSED(pBulkController);
}

DlgControllerLearning::~DlgControllerLearning() {
    // If the user hit done, we should save any pending mappings.
    if (m_messagesLearned) {
        commitMapping();
        resetWizard();
        stackedWidget->setCurrentIndex(page1Choose);
    }

    //If there was any ongoing learning, cancel it (benign if there wasn't).
    emit(stopLearning());
    emit(stopListeningForClicks());
}

void DlgControllerLearning::showControlMenu() {
    m_controlPickerMenu.exec(pushButtonChooseControl->mapToGlobal(QPoint(0,0)));
}

void DlgControllerLearning::loadControl(const ConfigKey& key, QString description) {
    // If we have learned a mapping and the user picked a new control then we
    // should tell the controller to commit the existing ones.
    if (m_messagesLearned) {
        commitMapping();
        resetWizard();
        stackedWidget->setCurrentIndex(page1Choose);
        startListening();
    }
    m_currentControl = key;

    if (description.isEmpty()) {
        description = key.group + "," + key.item;
    }
    m_currentDescription = description;

    QString message = tr("Learning: %1. Now move a control on your controller.")
            .arg(description);
    controlToMapMessage->setText(message);
    comboBoxChosenControl->setEditText(
            m_controlPickerMenu.controlTitleForConfigKey(key));
    labelMappedTo->setText("");
}

void DlgControllerLearning::controlPicked(ConfigKey control) {
    QString description = m_controlPickerMenu.descriptionForConfigKey(control);
    loadControl(control, description);
}

void DlgControllerLearning::controlClicked(ControlObject* pControl) {
    if (!pControl) {
        return;
    }

    controlPicked(pControl->getKey());
}
