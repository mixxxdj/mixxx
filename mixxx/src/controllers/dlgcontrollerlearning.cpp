#include "controllers/dlgcontrollerlearning.h"

#include <QCompleter>

#include "control/controlobject.h"
#include "controllers/learningutils.h"
#include "controllers/midi/midicontroller.h"
#include "controllers/midi/midiutils.h"
#include "moc_dlgcontrollerlearning.cpp"
#include "util/versionstore.h"

namespace {
typedef QPair<QString, ConfigKey> NamedControl;
bool namedControlComparator(const NamedControl& l1, const NamedControl& l2) {
    return l1.first < l2.first;
}
} // namespace

DlgControllerLearning::DlgControllerLearning(QWidget* parent,
        Controller* controller,
        ControlPickerMenu* pControlPickerMenu)
        : QDialog(parent),
          m_pController(controller),
          m_pControlPickerMenu(pControlPickerMenu),
          m_messagesLearned(false) {
    qRegisterMetaType<MidiInputMappings>("MidiInputMappings");

    setupUi(this);
    labelDescription->setWordWrap(true);
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

    connect(m_pControlPickerMenu,
            &ControlPickerMenu::controlPicked,
            this,
            &DlgControllerLearning::controlPicked);

    comboBoxChosenControl->completer()->setCompletionMode(
        QCompleter::PopupCompletion);
    comboBoxChosenControl->completer()->setCaseSensitivity(Qt::CaseInsensitive);
    comboBoxChosenControl->completer()->setFilterMode(Qt::MatchContains);

    populateComboBox();
    connect(comboBoxChosenControl,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgControllerLearning::comboboxIndexChanged);

    connect(pushButtonChooseControl,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::showControlMenu);
    connect(pushButtonClose, &QAbstractButton::clicked, this, &DlgControllerLearning::close);
    connect(pushButtonClose_2, &QAbstractButton::clicked, this, &DlgControllerLearning::close);
    connect(pushButtonCancelLearn,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::slotCancelLearn);
    connect(pushButtonRetry, &QAbstractButton::clicked, this, &DlgControllerLearning::slotRetry);
    connect(pushButtonStartLearn,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::slotStartLearningPressed);
    connect(pushButtonLearnAnother,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::slotChooseControlPressed);
#ifdef CONTROLLERLESSTESTING
    connect(pushButtonFakeControl,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::DEBUGFakeMidiMessage);
    connect(pushButtonFakeControl2,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::DEBUGFakeMidiMessage2);
#else
    pushButtonFakeControl->hide();
    pushButtonFakeControl2->hide();
#endif

    // We only want to listen to clicked() so we don't fire
    // slotMidiOptionsChanged when we change the checkboxes programmatically.
    connect(midiOptionSwitchMode,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::slotMidiOptionsChanged);
    connect(midiOptionSoftTakeover,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::slotMidiOptionsChanged);
    connect(midiOptionInvert,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::slotMidiOptionsChanged);
    connect(midiOptionSelectKnob,
            &QAbstractButton::clicked,
            this,
            &DlgControllerLearning::slotMidiOptionsChanged);

    slotChooseControlPressed();

    // Wait 1 second until we detect the control the user moved.
    m_lastMessageTimer.setInterval(1500);
    m_lastMessageTimer.setSingleShot(true);
    connect(&m_lastMessageTimer, &QTimer::timeout, this, &DlgControllerLearning::slotTimerExpired);

    m_firstMessageTimer.setInterval(7000);
    m_firstMessageTimer.setSingleShot(true);
    connect(&m_firstMessageTimer,
            &QTimer::timeout,
            this,
            &DlgControllerLearning::slotFirstMessageTimeout);
}

void DlgControllerLearning::populateComboBox() {
    // Sort all of the controls and add them to the combo box
    comboBoxChosenControl->clear();
    // Add a blank item so the lineedit is initially empty
    comboBoxChosenControl->addItem("", QVariant::fromValue(ConfigKey()));
    // Note: changes here might break the completer, so remember to test that
    QList<NamedControl> sorted_controls;
    for (const ConfigKey& key : m_pControlPickerMenu->controlsAvailable()) {
        sorted_controls.push_back(NamedControl(
                m_pControlPickerMenu->controlTitleForConfigKey(key),
                key));
    }
    std::sort(sorted_controls.begin(), sorted_controls.end(), namedControlComparator);
    for (const NamedControl& control : std::as_const(sorted_controls)) {
        comboBoxChosenControl->addItem(control.first,
                                       QVariant::fromValue(control.second));
    }
}

void DlgControllerLearning::resetWizard(bool keepCurrentControl) {
    m_firstMessageTimer.stop();
    m_lastMessageTimer.stop();
    emit clearTemporaryInputMappings();

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
    // Disconnect everything in both directions so we don't end up with duplicate connections
    // after pressing the "Learn Another" button
    MidiController* pMidiController = qobject_cast<MidiController*>(m_pController);
    VERIFY_OR_DEBUG_ASSERT(pMidiController) {
        // DlgControllerLearning should have only been created by DlgController if
        // the Controller was a MidiController.
        qWarning() << "Only MIDI controllers are supported by the learning wizard.";
        return;
    }
    pMidiController->disconnect(this);
    this->disconnect(pMidiController);

    connect(pMidiController,
            &MidiController::messageReceived,
            this,
            &DlgControllerLearning::slotMessageReceived);

    connect(this,
            &DlgControllerLearning::learnTemporaryInputMappings,
            pMidiController,
            &MidiController::learnTemporaryInputMappings);
    connect(this,
            &DlgControllerLearning::clearTemporaryInputMappings,
            pMidiController,
            &MidiController::clearTemporaryInputMappings);

    connect(this,
            &DlgControllerLearning::commitTemporaryInputMappings,
            pMidiController,
            &MidiController::commitTemporaryInputMappings);
    connect(this,
            &DlgControllerLearning::startLearning,
            pMidiController,
            &MidiController::startLearning);
    connect(this,
            &DlgControllerLearning::stopLearning,
            pMidiController,
            &MidiController::stopLearning);

    emit startLearning();
    emit listenForClicks();
}

void DlgControllerLearning::slotStartLearningPressed() {
    if (!m_currentControl.isValid()) {
        return;
    }
    m_firstMessageTimer.start();
    stackedWidget->setCurrentWidget(page2Learn);
}

#ifdef CONTROLLERLESSTESTING
void DlgControllerLearning::DEBUGFakeMidiMessage() {
    slotMessageReceived(MidiOpCode::ControlChange, 0x20, 0x41);
}

void DlgControllerLearning::DEBUGFakeMidiMessage2() {
    slotMessageReceived(MidiOpCode::ControlChange, 0x20, 0x3F);
}
#endif

void DlgControllerLearning::slotMessageReceived(unsigned char status,
                                                unsigned char control,
                                                unsigned char value) {
    // Ignore message since we don't have a control yet.
    if (!m_currentControl.isValid()) {
        return;
    }

    // Ignore message since we already learned a mapping for this control.
    if (m_messagesLearned) {
        return;
    }

    // NOTE(rryan): We intend to use MidiKey(status, control) here rather than
    // setting fields individually since we will use the MidiKey with an input
    // mapping. See Issue #8432
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

    // two conditions for letting the timer expire and map received control
    // * The progress bar is full
    // * It is a button (NoteOff/NoteOn) not a knob/slider (ControlChange)
    // That way the user won't just push buttons forever and wonder why the
    // wizard never advances.
    MidiOpCode opCode = MidiUtils::opCodeFromStatus(status);
    if (m_messages.length() == 1 ||
            (opCode == MidiOpCode::ControlChange &&
                    progressBarWiggleFeedback->value() !=
                            progressBarWiggleFeedback->maximum())) {
        m_lastMessageTimer.start();
    }
    // else: let timer expire
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
    emit learnTemporaryInputMappings(m_mappings);

    QString midiControl = "";
    bool first = true;
    foreach (const MidiInputMapping& mapping, m_mappings) {
        MidiOpCode opCode = MidiUtils::opCodeFromStatus(mapping.key.status);
        bool twoBytes = MidiUtils::isMessageTwoBytes(opCode);
        QString mappingStr = twoBytes
                ? QString("Status: 0x%1 Control: 0x%2 Options: 0x%03")
                          .arg(QString::number(mapping.key.status, 16)
                                          .toUpper(),
                                  QString::number(mapping.key.control, 16)
                                          .toUpper()
                                          .rightJustified(2, '0'),
                                  QString::number(mapping.options, 16)
                                          .toUpper()
                                          .rightJustified(2, '0'))
                : QString("0x%1 0x%2")
                          .arg(QString::number(mapping.key.status, 16)
                                          .toUpper(),
                                  QString::number(mapping.options, 16)
                                          .toUpper()
                                          .rightJustified(2, '0'));

        // Set the debug string and "Advanced MIDI Options" group using the
        // first mapping.
        if (first) {
            midiControl = mappingStr;
            MidiOptions options = mapping.options;
            midiOptionInvert->setChecked(options.testFlag(MidiOption::Invert));
            midiOptionSelectKnob->setChecked(options.testFlag(MidiOption::SelectKnob));
            midiOptionSoftTakeover->setChecked(options.testFlag(MidiOption::SoftTakeover));
            midiOptionSwitchMode->setChecked(options.testFlag(MidiOption::Switch));
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

    emit clearTemporaryInputMappings();

    // Go over every mapping and set its MIDI options to match the user's
    // choices.
    for (MidiInputMappings::iterator it = m_mappings.begin();
         it != m_mappings.end(); ++it) {
        MidiOptions& options = it->options;
        options.setFlag(MidiOption::Switch, midiOptionSwitchMode->isChecked());
        options.setFlag(MidiOption::SoftTakeover, midiOptionSoftTakeover->isChecked());
        options.setFlag(MidiOption::Invert, midiOptionInvert->isChecked());
        options.setFlag(MidiOption::SelectKnob, midiOptionSelectKnob->isChecked());
    }

    emit learnTemporaryInputMappings(m_mappings);
}

void DlgControllerLearning::commitMapping() {
    emit commitTemporaryInputMappings();
    emit inputMappingsLearned(m_mappings);
}

DlgControllerLearning::~DlgControllerLearning() {
    // If the user hit done, we should save any pending mappings.
    if (m_messagesLearned) {
        commitMapping();
        resetWizard();
        stackedWidget->setCurrentWidget(page1Choose);
    }

    //If there was any ongoing learning, cancel it (benign if there wasn't).
    emit stopLearning();
    emit stopListeningForClicks();
}

void DlgControllerLearning::showControlMenu() {
    m_pControlPickerMenu->exec(pushButtonChooseControl->mapToGlobal(QPoint(0, 0)));
}

void DlgControllerLearning::loadControl(const ConfigKey& key,
        const QString& title,
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

void DlgControllerLearning::controlPicked(const ConfigKey& control) {
    if (!ControlObject::exists(control)) {
        QMessageBox msg(QMessageBox::Warning,
                VersionStore::applicationName(),
                tr("The selected control does not exist.<br>"
                   "This likely a bug. Please report it on the Mixxx bug "
                   "tracker.<br>"
                   "<a href='https://github.com/mixxxdj/mixxx/issues'>"
                   "https://github.com/mixxxdj/mixxx/issues</a>"
                   "<br><br>"
                   "You tried to learn: %1,%2")
                        .arg(control.group, control.item));
        msg.setTextFormat(Qt::RichText);              // make the link clickable
        msg.setWindowFlags(Qt::WindowStaysOnTopHint); // position it above the Learning dialog
        msg.exec();
        return;
    }
    QString title = m_pControlPickerMenu->controlTitleForConfigKey(control);
    QString description = m_pControlPickerMenu->descriptionForConfigKey(control);
    loadControl(control, title, description);
}

void DlgControllerLearning::controlClicked(const ConfigKey& controlKey) {
    if (!m_pControlPickerMenu->controlExists(controlKey)) {
        qWarning() << "Mixxx UI element clicked for which there is no "
                      "learnable control "
                   << controlKey.group << " " << controlKey.item;
        QMessageBox::warning(
                this,
                VersionStore::applicationName(),
                tr("The control you clicked in Mixxx is not learnable.\n"
                   "This could be because you are either using an old skin"
                   " and this control is no longer supported, "
                   "or you clicked a control that provides visual feedback"
                   " and can only be mapped to outputs like LEDs via"
                   " scripts.\n"
                   "\nYou tried to learn: %1,%2")
                        .arg(controlKey.group, controlKey.item),
                QMessageBox::Ok,
                QMessageBox::Ok);
        return;
    }
    controlPicked(controlKey);
}

void DlgControllerLearning::comboboxIndexChanged(int index) {
    ConfigKey control =
            comboBoxChosenControl->itemData(index).value<ConfigKey>();
    if (!control.isValid()) {
        labelDescription->setText(tr(""));
        pushButtonStartLearn->setDisabled(true);
        return;
    }
    controlPicked(control);
}
