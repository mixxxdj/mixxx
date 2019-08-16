/**
* @file dlgcontrollerlearning.cpp
* @author Sean M. Pappalardo  spappalardo@mixxx.org
* @date Thu 12 Apr 2012
* @brief The controller mapping learning wizard
*
*/

#include <QCompleter>

#include "control/controlobject.h"
#include "controllers/dlgcontrollerlearning.h"
#include "controllers/learningutils.h"
#include "controllers/midi/midiutils.h"
#include "util/version.h"

namespace {
typedef QPair<QString, ConfigKey> NamedControl;
bool namedControlComparator(const NamedControl& l1, const NamedControl& l2) {
    return l1.first < l2.first;
}
}

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

    QString helpTitle(tr("Click anywhere in Mixxx or choose a control to learn"));
    QString helpBody(tr("You can click on any button, slider, or knob in Mixxx "
                        "to teach it that control.  You can also type in the "
                        "box to search for a control by name, or click the "
                        "Choose Control button to select from a list."));
    labelMappingHelp->setTextFormat(Qt::RichText);
    labelMappingHelp->setText(QString(
            "<p><span style=\"font-weight:600;\">%1</span></p>"
            "<p>%2</p>").arg(
                    helpTitle, helpBody));


    QString nextTitle(tr("Now test it out!"));
    QString nextInstructionBody(tr(
            "If you manipulate the control, you should see the Mixxx user interface "
            "respond the way you expect."));
    QString nextTroubleshootTitle(tr("Not quite right?"));
    QString nextTroubleshootBody(tr(
            "If the mapping is not working try enabling an advanced option "
            "below and then try the control again. Or click Retry to redetect "
            "the midi control."));

    labelNextHelp->setTextFormat(Qt::RichText);
    labelNextHelp->setText(QString(
            "<p><span style=\"font-weight:600;\">%1</span></p>"
            "<p>%2</p><p><span style=\"font-weight:600;\">%3</span></p>"
            "<p>%4</p>").arg(
                    nextTitle, nextInstructionBody,
                    nextTroubleshootTitle, nextTroubleshootBody));

    // Ensure the first page is always shown regardless of the last page shown
    // when the .ui file was saved.
    stackedWidget->setCurrentWidget(page1Choose);

    // Delete this dialog when its closed. We don't want any persistence.
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(Qt::Tool | Qt::WindowStaysOnTopHint);

    connect(&m_controlPickerMenu, SIGNAL(controlPicked(ConfigKey)),
            this, SLOT(controlPicked(ConfigKey)));

    comboBoxChosenControl->completer()->setCompletionMode(
        QCompleter::PopupCompletion);
    populateComboBox();
    connect(comboBoxChosenControl, SIGNAL(currentIndexChanged(int)),
            this, SLOT(comboboxIndexChanged(int)));

    connect(pushButtonChooseControl, SIGNAL(clicked()), this, SLOT(showControlMenu()));
    connect(pushButtonClose, SIGNAL(clicked()), this, SLOT(close()));
    connect(pushButtonClose_2, SIGNAL(clicked()), this, SLOT(close()));
    connect(pushButtonCancelLearn, SIGNAL(clicked()), this, SLOT(slotCancelLearn()));
    connect(pushButtonRetry, SIGNAL(clicked()), this, SLOT(slotRetry()));
    connect(pushButtonStartLearn, SIGNAL(clicked()), this, SLOT(slotStartLearningPressed()));
    connect(pushButtonLearnAnother, SIGNAL(clicked()), this, SLOT(slotChooseControlPressed()));
#ifdef CONTROLLERLESSTESTING
    connect(pushButtonFakeControl, SIGNAL(clicked()), this, SLOT(DEBUGFakeMidiMessage()));
    connect(pushButtonFakeControl2, SIGNAL(clicked()), this, SLOT(DEBUGFakeMidiMessage2()));
#else
    pushButtonFakeControl->hide();
    pushButtonFakeControl2->hide();
#endif

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

void DlgControllerLearning::populateComboBox() {
    // Sort all of the controls and add them to the combo box
    comboBoxChosenControl->clear();
    comboBoxChosenControl->addItem("", QVariant::fromValue(ConfigKey()));
    QList<NamedControl> sorted_controls;
    foreach(ConfigKey key, m_controlPickerMenu.controlsAvailable())
    {
        sorted_controls.push_back(
                NamedControl(m_controlPickerMenu.controlTitleForConfigKey(key),
                             key));
    }
    std::sort(sorted_controls.begin(), sorted_controls.end(),
          namedControlComparator);
    foreach(NamedControl control, sorted_controls)
    {
        comboBoxChosenControl->addItem(control.first,
                                       QVariant::fromValue(control.second));
    }
}

void DlgControllerLearning::resetWizard(bool keepCurrentControl) {
    m_firstMessageTimer.stop();
    m_lastMessageTimer.stop();
    emit(clearTemporaryInputMappings());

    if (!keepCurrentControl) {
        m_currentControl = ConfigKey();
        comboBoxChosenControl->setCurrentIndex(0);
        labelDescription->setText("");
        pushButtonStartLearn->setDisabled(true);
    }
    m_messagesLearned = false;
    m_messages.clear();
    m_mappings.clear();
    midiOptionInvert->setChecked(false);
    midiOptionSelectKnob->setChecked(false);
    midiOptionSoftTakeover->setChecked(false);
    midiOptionSwitchMode->setChecked(false);

    progressBarWiggleFeedback->setValue(0);
    progressBarWiggleFeedback->setMinimum(0);
    progressBarWiggleFeedback->setMaximum(200);
    progressBarWiggleFeedback->hide();

    labelMappedTo->setText("");
    labelErrorText->setText("");
}

void DlgControllerLearning::slotChooseControlPressed() {
    // If we learned messages, commit them.
    if (m_messagesLearned) {
        commitMapping();
    }
    resetWizard();
    stackedWidget->setCurrentWidget(page1Choose);
    startListening();
}

void DlgControllerLearning::startListening() {
    // Start listening as soon as we're on this page -- that way advanced
    // users don't have to specifically click the "Learn" button.
    // Get the underlying type of the Controller. This will call
    // one of the visit() methods below immediately.
    m_pController->accept(this);
    emit(listenForClicks());
}

void DlgControllerLearning::slotStartLearningPressed() {
    if (m_currentControl.isNull()) {
        return;
    }
    m_firstMessageTimer.start();
    stackedWidget->setCurrentWidget(page2Learn);
}

#ifdef CONTROLLERLESSTESTING
void DlgControllerLearning::DEBUGFakeMidiMessage() {
    slotMessageReceived(MIDI_CC, 0x20, 0x41);
}

void DlgControllerLearning::DEBUGFakeMidiMessage2() {
    slotMessageReceived(MIDI_CC, 0x20, 0x3F);
}
#endif

void DlgControllerLearning::slotMessageReceived(unsigned char status,
                                                unsigned char control,
                                                unsigned char value) {
    // Ignore message since we don't have a control yet.
    if (m_currentControl.isNull()) {
        return;
    }

    // Ignore message since we already learned a mapping for this control.
    if (m_messagesLearned) {
        return;
    }

    // NOTE(rryan): We intend to use MidiKey(status, control) here rather than
    // setting fields individually since we will use the MidiKey with an input
    // mapping. See Bug #1532297
    MidiKey key(status, control);

    // Ignore all standard MIDI System Real-Time Messages because they
    // are continuously sent and prevent mapping of the pressed key.
    if (MidiUtils::isClockSignal(key)) {
        return;
    }

    if (m_messages.length() == 0) {
        // If an advanced user started wiggling a control without bothering to
        // click the Learn button, take them to the learning screen.
        stackedWidget->setCurrentWidget(page2Learn);
    }

    // If we get a few messages, it's probably a rotation control so let's give
    // feedback.  If we only get one or two messages, it's probably a button
    // and we shouldn't show the progress bar.
    if (m_messages.length() > 10) {
        if (progressBarWiggleFeedback->isVisible()) {
            progressBarWiggleFeedback->setValue(
                    progressBarWiggleFeedback->value() + 1);
        } else {
            progressBarWiggleFeedback->show();
        }
    }

    m_messages.append(QPair<MidiKey, unsigned char>(key, value));
    // We got a message, so we can cancel the taking-too-long timeout.
    m_firstMessageTimer.stop();

    // Unless this is a MIDI_CC and the progress bar is full, restart the
    // timer.  That way the user won't just push buttons forever and wonder
    // why the wizard never advances.
    unsigned char opCode = MidiUtils::opCodeFromStatus(status);
    if (opCode != MIDI_CC || progressBarWiggleFeedback->value() != 10) {
        m_lastMessageTimer.start();
    }
}

void DlgControllerLearning::slotCancelLearn() {
    resetWizard(true);
    stackedWidget->setCurrentWidget(page1Choose);
    startListening();
}

void DlgControllerLearning::slotFirstMessageTimeout() {
    resetWizard(true);
    if (m_messages.length() == 0) {
        labelErrorText->setText(tr("Didn't get any midi messages.  Please try again."));
    } else {
        qWarning() << "we shouldn't time out if we got something";
        m_messages.clear();
    }
    stackedWidget->setCurrentWidget(page1Choose);
    startListening();
}

void DlgControllerLearning::slotTimerExpired() {
    // It's been a timer interval since we last got a message. Let's try to
    // detect mappings.
    MidiInputMappings mappings =
            LearningUtils::guessMidiInputMappings(m_currentControl, m_messages);

    if (mappings.isEmpty()) {
        labelErrorText->setText(tr("Unable to detect a mapping -- please try again. Be sure to only touch one control at once."));
        m_messages.clear();
        // Don't reset the wizard.
        stackedWidget->setCurrentWidget(page1Choose);
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

    QString mapMessage = QString("<i>%1 %2</i>").arg(
            tr("Successfully mapped control:"), midiControl);
    labelMappedTo->setText(mapMessage);
    stackedWidget->setCurrentWidget(page3Confirm);
}

void DlgControllerLearning::slotRetry() {
    // If the user hit undo, instruct the controller to forget the mapping we
    // just added. So reset, but keep the control currently being learned.
    resetWizard(true);
    slotStartLearningPressed();
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
    // Disconnect everything in both directions so we don't end up with duplicate connections
    // after pressing the "Learn Another" button
    pMidiController->disconnect(this);
    this->disconnect(pMidiController);

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
    qWarning() << "ERROR: DlgControllerLearning does not support HID devices.";
    Q_UNUSED(pHidController);
}

void DlgControllerLearning::visit(BulkController* pBulkController) {
    qWarning() << "ERROR: DlgControllerLearning does not support Bulk devices.";
    Q_UNUSED(pBulkController);
}

DlgControllerLearning::~DlgControllerLearning() {
    // If the user hit done, we should save any pending mappings.
    if (m_messagesLearned) {
        commitMapping();
        resetWizard();
        stackedWidget->setCurrentWidget(page1Choose);
    }

    //If there was any ongoing learning, cancel it (benign if there wasn't).
    emit(stopLearning());
    emit(stopListeningForClicks());
}

void DlgControllerLearning::showControlMenu() {
    m_controlPickerMenu.exec(pushButtonChooseControl->mapToGlobal(QPoint(0,0)));
}

void DlgControllerLearning::loadControl(const ConfigKey& key,
                                        QString title,
                                        QString description) {
    // If we have learned a mapping and the user picked a new control then we
    // should tell the controller to commit the existing ones.
    if (m_messagesLearned) {
        commitMapping();
        resetWizard();
        stackedWidget->setCurrentWidget(page1Choose);
        startListening();
    }
    m_currentControl = key;

    if (description.isEmpty()) {
        description = key.group + "," + key.item;
    }
    comboBoxChosenControl->setEditText(title);

    labelDescription->setText(tr("<i>Ready to learn %1</i>").arg(description));
    QString learnmessage = tr("Learning: %1. Now move a control on your controller.")
            .arg(title);
    controlToMapMessage->setText(learnmessage);
    labelMappedTo->setText("");
    pushButtonStartLearn->setDisabled(false);
    pushButtonStartLearn->setFocus();
}

void DlgControllerLearning::controlPicked(ConfigKey control) {
    QString title = m_controlPickerMenu.controlTitleForConfigKey(control);
    QString description = m_controlPickerMenu.descriptionForConfigKey(control);
    loadControl(control, title, description);
}

void DlgControllerLearning::controlClicked(ControlObject* pControl) {
    if (!pControl) {
        return;
    }

    ConfigKey key = pControl->getKey();
    if (!m_controlPickerMenu.controlExists(key)) {
        qWarning() << "Mixxx UI element clicked for which there is no "
                      "learnable control " << key.group << " " << key.item;
        QMessageBox::warning(
                    this,
                    Version::applicationName(),
                    tr("The control you clicked in Mixxx is not learnable.\n"
                       "This could be because you are using an old skin"
                       " and this control is no longer supported.\n"
                       "\nYou tried to learn: %1,%2").arg(key.group, key.item),
                    QMessageBox::Ok, QMessageBox::Ok);
        return;
    }
    controlPicked(key);
}

void DlgControllerLearning::comboboxIndexChanged(int index) {
    ConfigKey control =
            comboBoxChosenControl->itemData(index).value<ConfigKey>();
    if (control.isNull()) {
        labelDescription->setText(tr(""));
        pushButtonStartLearn->setDisabled(true);
        return;
    }
    controlPicked(control);
}
