#include "preferences/dialog/dlgprefsound.h"

#include <QMessageBox>
#include <QtDebug>

#include "control/controlproxy.h"
#include "engine/enginebuffer.h"
#include "engine/enginemaster.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefsound.cpp"
#include "preferences/dialog/dlgprefsounditem.h"
#include "soundio/soundmanager.h"
#include "util/rlimit.h"
#include "util/scopedoverridecursor.h"

/// Construct a new sound preferences pane. Initializes and populates all the
/// all the controls to the values obtained from SoundManager.
DlgPrefSound::DlgPrefSound(QWidget* pParent,
        std::shared_ptr<SoundManager> pSoundManager,
        UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSoundManager(pSoundManager),
          m_pSettings(pSettings),
          m_soundConfig(pSoundManager.get()),
          m_settingsModified(false),
          m_bLatencyChanged(false),
          m_bSkipConfigClear(true),
          m_loading(false) {
    qDebug() << "  **";
    qDebug() << "  **";
    qDebug() << "  ** Dlg init";
    setupUi(this);
    // Create text color for the wiki links
    createLinkColor();

    connect(m_pSoundManager.get(),
            &SoundManager::devicesUpdated,
            this,
            &DlgPrefSound::refreshDevices);

    connect(profileComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::soundProfileSelected);

    connect(newProfileButton,
            &QAbstractButton::clicked,
            this,
            &DlgPrefSound::createNewProfile);
    connect(renameProfileButton,
            &QAbstractButton::clicked,
            this,
            &DlgPrefSound::renameProfile);
    connect(dupProfileButton,
            &QAbstractButton::clicked,
            this,
            &DlgPrefSound::duplicateProfile);
    connect(delProfileButton,
            &QAbstractButton::clicked,
            this,
            &DlgPrefSound::deleteProfile);

    //connect(m_pSoundManager.get(),
    //        &SoundManager::soundProfileChanged,
    //        this,
    //        //&DlgPrefSound::slotUpdate);
    //        [this]() {
    //            // load settings, update GUI
    //            slotUpdate();
    //            //m_settingsModified = true;
    //            //slotApply();
    //        });

    apiComboBox->clear();
    apiComboBox->addItem(SoundManagerConfig::kEmptyComboBox,
            SoundManagerConfig::kDefaultAPI);
    updateAPIs();
    connect(apiComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::apiChanged);

    sampleRateComboBox->clear();
    foreach (unsigned int srate, m_pSoundManager->getSampleRates()) {
        if (srate > 0) {
            // no ridiculous sample rate values. prohibiting zero means
            // avoiding a potential div-by-0 error in ::updateLatencies
            sampleRateComboBox->addItem(tr("%1 Hz").arg(srate), srate);
        }
    }
    connect(sampleRateComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::sampleRateChanged);
    connect(sampleRateComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::updateAudioBufferSizes);
    connect(audioBufferComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::audioBufferChanged);

    deviceSyncComboBox->clear();
    deviceSyncComboBox->addItem(tr("Default (long delay)"));
    deviceSyncComboBox->addItem(tr("Experimental (no delay)"));
    deviceSyncComboBox->addItem(tr("Disabled (short delay)"));
    deviceSyncComboBox->setCurrentIndex(2);
    connect(deviceSyncComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::syncBuffersChanged);

    engineClockComboBox->clear();
    engineClockComboBox->addItem(tr("Soundcard Clock"));
    engineClockComboBox->addItem(tr("Network Clock"));
    engineClockComboBox->setCurrentIndex(0);
    connect(engineClockComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::engineClockChanged);

    keylockComboBox->clear();
    for (const auto engine : EngineBuffer::kKeylockEngines) {
        if (EngineBuffer::isKeylockEngineAvailable(engine)) {
            keylockComboBox->addItem(
                    EngineBuffer::getKeylockEngineName(engine), QVariant::fromValue(engine));
        }
    }

    m_pLatencyCompensation = new ControlProxy("[Master]", "microphoneLatencyCompensation", this);
    m_pMasterDelay = new ControlProxy("[Master]", "delay", this);
    m_pHeadDelay = new ControlProxy("[Master]", "headDelay", this);
    m_pBoothDelay = new ControlProxy("[Master]", "boothDelay", this);

    latencyCompensationSpinBox->setValue(m_pLatencyCompensation->get());
    latencyCompensationWarningLabel->setWordWrap(true);
    masterDelaySpinBox->setValue(m_pMasterDelay->get());
    headDelaySpinBox->setValue(m_pHeadDelay->get());
    boothDelaySpinBox->setValue(m_pBoothDelay->get());

    // TODO Don't apply these settings instantly, instead connect to
    // &DlgPrefSound::settingChanged
    connect(latencyCompensationSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::latencyCompensationSpinboxChanged);
    connect(masterDelaySpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::masterDelaySpinboxChanged);
    connect(headDelaySpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::headDelaySpinboxChanged);
    connect(boothDelaySpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::boothDelaySpinboxChanged);

    m_pMicMonitorMode = new ControlProxy("[Master]", "talkover_mix", this);
    micMonitorModeComboBox->addItem(tr("Main output only"),
        QVariant(static_cast<int>(EngineMaster::MicMonitorMode::MASTER)));
    micMonitorModeComboBox->addItem(tr("Main and booth outputs"),
        QVariant(static_cast<int>(EngineMaster::MicMonitorMode::MASTER_AND_BOOTH)));
    micMonitorModeComboBox->addItem(tr("Direct monitor (recording and broadcasting only)"),
        QVariant(static_cast<int>(EngineMaster::MicMonitorMode::DIRECT_MONITOR)));
    int modeIndex = micMonitorModeComboBox->findData(
        static_cast<int>(m_pMicMonitorMode->get()));
    micMonitorModeComboBox->setCurrentIndex(modeIndex);
    micMonitorModeComboBoxChanged(modeIndex);
    connect(micMonitorModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::micMonitorModeComboBoxChanged);

    initializePaths();
    loadSettings();

    connect(apiComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::settingChanged);
    connect(sampleRateComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::settingChanged);
    connect(audioBufferComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::settingChanged);
    connect(deviceSyncComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::settingChanged);
    connect(engineClockComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::settingChanged);
    connect(keylockComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::settingChanged);

    connect(queryButton, &QAbstractButton::clicked, this, &DlgPrefSound::queryClicked);

    connect(m_pSoundManager.get(),
            &SoundManager::outputRegistered,
            this,
            [this](const AudioOutput& output, AudioSource* source) {
                Q_UNUSED(source);
                addPath(output);
                loadSettings();
            });

    connect(m_pSoundManager.get(),
            &SoundManager::inputRegistered,
            this,
            [this](const AudioInput& input, AudioDestination* dest) {
                Q_UNUSED(dest);
                addPath(input);
                loadSettings();
            });

    m_pMasterAudioLatencyOverloadCount =
            new ControlProxy("[Master]", "audio_latency_overload_count", this);
    m_pMasterAudioLatencyOverloadCount->connectValueChanged(this, &DlgPrefSound::bufferUnderflow);

    m_pMasterLatency = new ControlProxy("[Master]", "latency", this);
    m_pMasterLatency->connectValueChanged(this, &DlgPrefSound::masterLatencyChanged);

    // TODO: remove this option by automatically disabling/enabling the master mix
    // when recording, broadcasting, headphone, and master outputs are enabled/disabled
    m_pMasterEnabled = new ControlProxy("[Master]", "enabled", this);
    masterMixComboBox->addItem(tr("Disabled"));
    masterMixComboBox->addItem(tr("Enabled"));
    masterMixComboBox->setCurrentIndex(m_pMasterEnabled->toBool() ? 1 : 0);
    connect(masterMixComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::masterMixChanged);
    m_pMasterEnabled->connectValueChanged(this, &DlgPrefSound::masterEnabledChanged);

    m_pMasterMonoMixdown = new ControlProxy("[Master]", "mono_mixdown", this);
    masterOutputModeComboBox->addItem(tr("Stereo"));
    masterOutputModeComboBox->addItem(tr("Mono"));
    masterOutputModeComboBox->setCurrentIndex(m_pMasterMonoMixdown->toBool() ? 1 : 0);
    connect(masterOutputModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::masterOutputModeComboBoxChanged);
    m_pMasterMonoMixdown->connectValueChanged(this, &DlgPrefSound::masterMonoMixdownChanged);

    m_pKeylockEngine =
            new ControlProxy("[Master]", "keylock_engine", this);

#ifdef __LINUX__
    qDebug() << "RLimit Cur " << RLimit::getCurRtPrio();
    qDebug() << "RLimit Max " << RLimit::getMaxRtPrio();

    if (RLimit::isRtPrioAllowed()) {
        realtimeHint->setText(tr("Realtime scheduling is enabled."));
    } else {
        realtimeHint->setText(
                tr("To enable Realtime scheduling (currently disabled), see the %1.")
                        .arg(coloredLinkString(
                                m_pLinkColor,
                                QStringLiteral("Mixxx Wiki"),
                                MIXXX_WIKI_AUDIO_LATENCY_URL)));
    }
#else
    // the limits warning is a Linux only thing
    realtimeHint->hide();
#endif // __LINUX__

    // Set the focus policy for QComboBoxes (and wide QDoubleSpinBoxes) and
    // connect them to the custom event filter below so they don't accept focus
    // when we scroll the preferences page to avoid undesired value changes.
    QObjectList objList = children();
    for (int i = 0; i < objList.length(); ++i) {
        QComboBox* combo = qobject_cast<QComboBox*>(objList[i]);
        QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(objList[i]);
        if (combo) {
            combo->setFocusPolicy(Qt::StrongFocus);
            combo->installEventFilter(this);
        } else if (spin) {
            spin->setFocusPolicy(Qt::StrongFocus);
            spin->installEventFilter(this);
        }
    }

    hardwareGuide->setText(
            tr("The %1 lists sound cards and controllers you may want to "
               "consider for using Mixxx.")
                    .arg(coloredLinkString(
                            m_pLinkColor,
                            tr("Mixxx DJ Hardware Guide"),
                            MIXXX_WIKI_HARDWARE_COMPATIBILITY_URL)));
    qDebug() << "  **";
    qDebug() << "  **";
}

DlgPrefSound::~DlgPrefSound() {
    delete m_pLatencyCompensation;
}

// Catch scroll events over comboboxes and pass them to the scroll area instead.
bool DlgPrefSound::eventFilter(QObject* obj, QEvent* e) {
    if (e->type() == QEvent::Wheel) {
        // Reject scrolling only if widget is unfocused.
        // Object to widget cast is needed to check the focus state.
        QComboBox* combo = qobject_cast<QComboBox*>(obj);
        QDoubleSpinBox* spin = qobject_cast<QDoubleSpinBox*>(obj);
        if ((combo && !combo->hasFocus()) || (spin && !spin->hasFocus())) {
            QApplication::sendEvent(verticalLayout_2, e);
            return true;
        }
    }
    return QObject::eventFilter(obj, e);
}

void DlgPrefSound::slotUpdate() {
    qDebug() << "   #";
    qDebug() << "   # slotUpdate";
    // this is unfortunate, because slotUpdate is called every time
    // we change to this pane, we lose changed and unapplied settings
    // every time. There's no real way around this, just another argument
    // for a prefs rewrite -- bkgood
    m_bSkipConfigClear = true;
    loadSettings();
    checkLatencyCompensation();
    //updateProfileDeleteButton();
    m_bSkipConfigClear = false;
    m_settingsModified = false;
    qDebug() << "   #";
}

void DlgPrefSound::slotApply() {
    qDebug() << "   #";
    qDebug() << "   # slotApply";
    if (!m_settingsModified) {
        qDebug() << "   # !m_settingsModified, return";
        return;
    }

    m_soundConfig.clearInputs();
    m_soundConfig.clearOutputs();
    emit writePaths(&m_soundConfig);

    SoundDeviceStatus status = SoundDeviceStatus::Ok;
    {
        ScopedWaitCursor cursor;
        m_pKeylockEngine->set(keylockComboBox->currentData().toDouble());
        m_pSettings->set(ConfigKey("[Master]", "keylock_engine"),
                ConfigValue(keylockComboBox->currentData().toInt()));

        status = m_pSoundManager->setConfig(m_soundConfig);
    }
    if (status != SoundDeviceStatus::Ok) {
        QString error = m_pSoundManager->getLastErrorMessage(status);
        QMessageBox::warning(nullptr, tr("Configuration error"), error);
    } else {
        qDebug() << "   # config loaded";
        // SoundManager has now saved the profile name in mixxx.cfg
        m_settingsModified = false;
        m_bLatencyChanged = false;
    }
    m_bSkipConfigClear = true;
    loadSettings(); // in case SM decided to change anything it didn't like
    checkLatencyCompensation();
    m_bSkipConfigClear = false;
    qDebug() << "   #";
}

QUrl DlgPrefSound::helpUrl() const {
    return QUrl(MIXXX_MANUAL_SOUND_URL);
}

/// Initializes (and creates) all the path items. Each path item widget allows
/// the user to input a sound device name and channel number given a description
/// of what will be done with that info. Inputs and outputs are grouped by tab,
/// and each path item has an identifier (Master, Headphones, ...) and an index,
/// if necessary.
void DlgPrefSound::initializePaths() {
    foreach (AudioOutput out, m_pSoundManager->registeredOutputs()) {
        if (!out.isHidden()) {
            addPath(out);
        }
    }
    foreach (AudioInput in, m_pSoundManager->registeredInputs()) {
        addPath(in);
    }
}

void DlgPrefSound::addPath(const AudioOutput& output) {
    // if we already know about this output, don't make a new entry
    foreach (QObject *obj, outputTab->children()) {
        DlgPrefSoundItem *item = qobject_cast<DlgPrefSoundItem*>(obj);
        if (item) {
            if (item->type() == output.getType()) {
                if (AudioPath::isIndexed(item->type())) {
                    if (item->index() == output.getIndex()) {
                        return;
                    }
                } else {
                    return;
                }
            }
        }
    }

    DlgPrefSoundItem *toInsert;
    AudioPathType type = output.getType();
    if (AudioPath::isIndexed(type)) {
        toInsert = new DlgPrefSoundItem(outputTab, type,
            m_outputDevices, false, output.getIndex());
    } else {
        toInsert = new DlgPrefSoundItem(outputTab, type,
            m_outputDevices, false);
    }
    connect(this, &DlgPrefSound::refreshOutputDevices, toInsert, &DlgPrefSoundItem::refreshDevices);
    insertItem(toInsert, outputVLayout);
    connectSoundItem(toInsert);
}

void DlgPrefSound::addPath(const AudioInput& input) {
    DlgPrefSoundItem *toInsert;
    // if we already know about this input, don't make a new entry
    foreach (QObject *obj, inputTab->children()) {
        DlgPrefSoundItem *item = qobject_cast<DlgPrefSoundItem*>(obj);
        if (item) {
            if (item->type() == input.getType()) {
                if (AudioPath::isIndexed(item->type())) {
                    if (item->index() == input.getIndex()) {
                        return;
                    }
                } else {
                    return;
                }
            }
        }
    }
    AudioPathType type = input.getType();
    if (AudioPath::isIndexed(type)) {
        toInsert = new DlgPrefSoundItem(inputTab, type,
            m_inputDevices, true, input.getIndex());
    } else {
        toInsert = new DlgPrefSoundItem(inputTab, type,
            m_inputDevices, true);
    }
    connect(this, &DlgPrefSound::refreshInputDevices, toInsert, &DlgPrefSoundItem::refreshDevices);
    insertItem(toInsert, inputVLayout);
    connectSoundItem(toInsert);
}

void DlgPrefSound::connectSoundItem(DlgPrefSoundItem *item) {
    connect(item, &DlgPrefSoundItem::settingChanged, this, &DlgPrefSound::deviceSettingChanged);
    connect(this, &DlgPrefSound::loadPaths, item, &DlgPrefSoundItem::loadPath);
    connect(this, &DlgPrefSound::writePaths, item, &DlgPrefSoundItem::writePath);
    connect(this, &DlgPrefSound::updatingAPI, item, &DlgPrefSoundItem::save);
    connect(this, &DlgPrefSound::updatedAPI, item, &DlgPrefSoundItem::reload);
}

void DlgPrefSound::insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout) {
    int pos;
    for (pos = 0; pos < pLayout->count() - 1; ++pos) {
        DlgPrefSoundItem *pOther(qobject_cast<DlgPrefSoundItem*>(
            pLayout->itemAt(pos)->widget()));
        if (!pOther) {
            continue;
        }
        if (pItem->type() < pOther->type()) {
            break;
        } else if (pItem->type() == pOther->type()
            && AudioPath::isIndexed(pItem->type())
            && pItem->index() < pOther->index()) {
            break;
        }
    }
    pLayout->insertWidget(pos, pItem);
}

/// Convenience overload to load settings from the SoundManagerConfig owned by
/// SoundManager.
void DlgPrefSound::loadSettings() {
    qDebug() << " ....";
    qDebug() << "  ....";
    qDebug() << "   .... Dlg loadSettings:";
    loadSettings(m_pSoundManager->getConfig());
    qDebug() << "  ....";
    qDebug() << " ....";
}

/// Loads the settings in the given SoundManagerConfig into the dialog.
void DlgPrefSound::loadSettings(const SoundManagerConfig &config) {
    qDebug() << "     .";
    qDebug() << "     Dlg loadSettings:";
    m_loading = true; // so settingsChanged ignores all our modifications here
    m_soundConfig = config;

    profileComboBox->clear();
    profileComboBox->addItems(m_pSoundManager->getSoundProfileNames());
    QString currProfile = m_pSoundManager->getCurrentSoundProfileName();
    qDebug() << "       curr profile:" << currProfile;
    if (!currProfile.isEmpty() && profileComboBox->findText(currProfile) != -1) {
        qDebug() << "       select" << currProfile;
        profileComboBox->setCurrentText(currProfile);
    } else {
        qDebug() << "       ! profile" << currProfile << "is not in list";
    }
    qDebug() << "     .";

    //updateProfileDeleteButton();
    //refreshDevices();

    int apiIndex = apiComboBox->findData(m_soundConfig.getAPI());
    if (apiIndex != -1) {
        apiComboBox->setCurrentIndex(apiIndex);
        refreshDevices();
        maybeAdjustGuiToJackAPI();
    }

    int sampleRateIndex = sampleRateComboBox->findData(m_soundConfig.getSampleRate());
    if (sampleRateIndex != -1) {
        sampleRateComboBox->setCurrentIndex(sampleRateIndex);
        if (audioBufferComboBox->count() <= 0) {
            updateAudioBufferSizes(sampleRateIndex); // so the latency combo box is
            // sure to be populated, if setCurrentIndex is called with the
            // currentIndex, the currentIndexChanged signal won't fire and
            // the updateLatencies slot won't run -- bkgood lp bug 689373
        }
    }

    int sizeIndex = audioBufferComboBox->findData(m_soundConfig.getAudioBufferSizeIndex());
    if (sizeIndex != -1) {
        audioBufferComboBox->setCurrentIndex(sizeIndex);
    }

    // Setting the index of audioBufferComboBox here sets m_bLatencyChanged to true,
    // but m_bLatencyChanged should only be true when the user has edited the
    // buffer size or sample rate.
    m_bLatencyChanged = false;

    int syncBuffers = m_soundConfig.getSyncBuffers();
    if (syncBuffers == 0) {
        // "Experimental (no delay)"))
        deviceSyncComboBox->setCurrentIndex(1);
    } else if (syncBuffers == 1) {
        // "Disabled (short delay)")) = 1 buffer
        deviceSyncComboBox->setCurrentIndex(2);
    } else {
        // "Default (long delay)" = 2 buffer
        deviceSyncComboBox->setCurrentIndex(0);
    }

    if (m_soundConfig.getForceNetworkClock()) {
        engineClockComboBox->setCurrentIndex(1);
    } else {
        engineClockComboBox->setCurrentIndex(0);
    }

    // Default keylock engine is Rubberband Faster (v2)
    const auto keylockEngine = static_cast<EngineBuffer::KeylockEngine>(
            m_pSettings->getValue(ConfigKey("[Master]", "keylock_engine"),
                    static_cast<int>(EngineBuffer::defaultKeylockEngine())));
    const auto keylockEngineVariant = QVariant::fromValue(keylockEngine);
    const int index = keylockComboBox->findData(keylockEngineVariant);
    if (index >= 0) {
        keylockComboBox->setCurrentIndex(index);
    } else {
        keylockComboBox->addItem(
                EngineBuffer::getKeylockEngineName(keylockEngine), keylockEngineVariant);
        keylockComboBox->setCurrentIndex(keylockComboBox->count() - 1);
    }

    m_loading = false;
    // DlgPrefSoundItem has it's own inhibit flag
    emit loadPaths(m_soundConfig);
}

void DlgPrefSound::soundProfileSelected(int index) {
    Q_UNUSED(index);
    if (m_loading) {
        return;
    }
    QString currentProfile = m_pSoundManager->getCurrentSoundProfileName();
    QString newProfile = profileComboBox->currentText();
    qDebug() << "   .";
    qDebug() << "   .";
    qDebug() << "   profile selected";
    qDebug() << "     curr:" << currentProfile;
    qDebug() << "     new :" << newProfile;

    // ignore no-op
    if (newProfile == currentProfile) {
        qDebug() << "     ! ignore no-op";
        return;
    }

    // check for pending changes of current profile
    if (m_settingsModified) {
        qDebug() << "     (sett modified)";
        QString changeProfileTitle =
                tr("Pending changes")
                        .arg(currentProfile);
        QString changeProfileLabel =
                tr("Discard changes and load <b>%1</b>?")
                        .arg(newProfile);
        QMessageBox changeProfileMsgBox;
        changeProfileMsgBox.setIcon(QMessageBox::Question);
        changeProfileMsgBox.setWindowTitle(changeProfileTitle);
        changeProfileMsgBox.setText(changeProfileLabel);
        changeProfileMsgBox.setStandardButtons(
                // Third option might be "Save pending changes", though slotApply() may
                // also open error dialogs which would complicate the workflow unnecessarily.
                /* QMessageBox::Save | */ QMessageBox::Discard | QMessageBox::Cancel);
        int choice = changeProfileMsgBox.exec();

        switch (choice) {
        //case QMessageBox::Save:
        //    qDebug() << "     Save";
        //    qDebug() << "     .";
        //    slotApply();
        //    break;
        case QMessageBox::Discard:
            qDebug() << "     Discard";
            qDebug() << "     .";
            // Ignore pending changes, load new profile
            m_settingsModified = false;
            break;
        case QMessageBox::Cancel:
        default:
            qDebug() << "     Cancel";
            qDebug() << "     .";
            // restore previous profile selection
            profileComboBox->blockSignals(true);
            profileComboBox->setCurrentText(currentProfile);
            profileComboBox->blockSignals(false);
            return;
        }
    }

    // select and read new profile
    SoundDeviceStatus status = SOUNDDEVICE_OK;
    {
        ScopedWaitCursor cursor;
        status = m_pSoundManager->setSoundProfile(newProfile);
        if (status == SOUNDDEVICE_OK) {
            // clear device comboboxes
            m_outputDevices.clear();
            m_inputDevices.clear();
            emit refreshOutputDevices(m_outputDevices);
            emit refreshInputDevices(m_inputDevices);
            // get new config, load settings, update GUI
            qDebug() << "   //profile selected";
            qDebug() << "   .";
            slotUpdate();
            return;
        }
    }

    // notify
    QString errMsg = m_pSoundManager->getLastErrorMessage(status);
    // "Sound profile could not be loaded"
    // "Loading saved profile"
    QMessageBox::warning(nullptr, tr("Configuration error"), errMsg);
    // TODO(ronso0) Adopt MixxxMainWindow::soundDeviceErrorDlg to let uers choose
    // 'Reconfigure', 'Load previous profile' (Cancel?) or 'Load default profile'

    // Restore saved profile
    QString configuredProfile = m_pSoundManager->getConfiguredSoundProfileName();
    // This is safe (will never be empty) since since SoundManager loads (or creates)
    // a default config with default name during construction
    qDebug() << "   //profile selected";
    qDebug() << "   .";
    emit profileComboBox->currentTextChanged(configuredProfile);
}

void DlgPrefSound::createNewProfile() {
}

void DlgPrefSound::duplicateProfile() {
}

void DlgPrefSound::renameProfile() {
}

void DlgPrefSound::deleteProfile() {
    // prohibit deleting default profile
}

/// Slot called when the user selects a different API, or the
/// software changes it programmatically (for instance, when it
/// loads a value from SoundManager). Refreshes the device lists
/// for the new API and pushes those to the path items.
void DlgPrefSound::apiChanged(int index) {
    m_soundConfig.setAPI(apiComboBox->itemData(index).toString());
    refreshDevices();
    maybeAdjustGuiToJackAPI();
}

void DlgPrefSound::maybeAdjustGuiToJackAPI() {
    // JACK sets its own buffer size and sample rate that Mixxx cannot change.
    // TODO(Be): Get the buffer size from JACK and update audioBufferComboBox.
    // PortAudio does not have a way to get the buffer size from JACK as of July 2017.
    if (m_soundConfig.getAPI() == MIXXX_PORTAUDIO_JACK_STRING) {
        sampleRateComboBox->setEnabled(false);
        latencyLabel->setEnabled(false);
        audioBufferComboBox->setEnabled(false);
    } else {
        sampleRateComboBox->setEnabled(true);
        latencyLabel->setEnabled(true);
        audioBufferComboBox->setEnabled(true);
    }
}

/// Updates the list of APIs, trying to keep the API and device selections
/// constant if possible.
void DlgPrefSound::updateAPIs() {
    QString currentAPI(apiComboBox->itemData(apiComboBox->currentIndex()).toString());
    emit updatingAPI();
    while (apiComboBox->count() > 1) {
        apiComboBox->removeItem(apiComboBox->count() - 1);
    }
    foreach (QString api, m_pSoundManager->getHostAPIList()) {
        apiComboBox->addItem(api, api);
    }
    int newIndex = apiComboBox->findData(currentAPI);
    if (newIndex > -1) {
        apiComboBox->setCurrentIndex(newIndex);
    }
    emit updatedAPI();
}

/// Slot called when the sample rate combo box changes to update the
/// sample rate in the config.
void DlgPrefSound::sampleRateChanged(int index) {
    m_soundConfig.setSampleRate(
            sampleRateComboBox->itemData(index).toUInt());
    m_bLatencyChanged = true;
    checkLatencyCompensation();
}

/// Slot called when the latency combo box is changed to update the
/// latency in the config.
void DlgPrefSound::audioBufferChanged(int index) {
    m_soundConfig.setAudioBufferSizeIndex(
            audioBufferComboBox->itemData(index).toUInt());
    m_bLatencyChanged = true;
    checkLatencyCompensation();
}

void DlgPrefSound::syncBuffersChanged(int index) {
    if (index == 0) {
        // "Default (long delay)" = 2 buffer
        m_soundConfig.setSyncBuffers(2);
    } else if (index == 1) {
        // "Experimental (no delay)")) = 0 buffer
        m_soundConfig.setSyncBuffers(0);
    } else {
        // "Disabled (short delay)")) = 1 buffer
        m_soundConfig.setSyncBuffers(1);
    }
}

void DlgPrefSound::engineClockChanged(int index) {
    if (index == 0) {
        // "Soundcard Clock"
        m_soundConfig.setForceNetworkClock(false);
    } else {
        // "Network Clock"
        m_soundConfig.setForceNetworkClock(true);
    }
}

// Slot called whenever the selected sample rate is changed. Populates the
// audio buffer input box with SMConfig::kMaxLatency values, starting at 1ms,
// representing a number of frames per buffer, which will always be a power
// of 2 (so the values displayed in ms won't be constant between sample rates,
// but they'll be close).
void DlgPrefSound::updateAudioBufferSizes(int sampleRateIndex) {
    double sampleRate = sampleRateComboBox->itemData(sampleRateIndex).toDouble();
    int oldSizeIndex = audioBufferComboBox->currentIndex();
    unsigned int framesPerBuffer = 1; // start this at 0 and inf loop happens
    // we don't want to display any sub-1ms buffer sizes (well maybe we do but I
    // don't right now!), so we iterate over all the buffer sizes until we
    // find the first that gives us a buffer size >= 1 ms -- bkgood
    // no div-by-0 in the next line because we don't allow srates of 0 in our
    // srate list when we construct it in the ctor -- bkgood
    for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2) {
    }
    audioBufferComboBox->clear();
    for (unsigned int i = 0; i < SoundManagerConfig::kMaxAudioBufferSizeIndex; ++i) {
        const auto latency = static_cast<float>(framesPerBuffer / sampleRate * 1000);
        // i + 1 in the next line is a latency index as described in SSConfig
        audioBufferComboBox->addItem(tr("%1 ms").arg(latency,0,'g',3), i + 1);
        framesPerBuffer <<= 1; // *= 2
    }
    if (oldSizeIndex < audioBufferComboBox->count() && oldSizeIndex >= 0) {
        audioBufferComboBox->setCurrentIndex(oldSizeIndex);
    } else {
        // set it to the max, let the user dig if they need better latency. better
        // than having a user get the pops on first use and thinking poorly of mixxx
        // because of it -- bkgood
        audioBufferComboBox->setCurrentIndex(audioBufferComboBox->count() - 1);
    }
}

/// Slot called when device lists go bad to refresh them, or the API
/// just changes and we need to display new devices.
void DlgPrefSound::refreshDevices() {
    if (m_soundConfig.getAPI() == SoundManagerConfig::kDefaultAPI) {
        m_outputDevices.clear();
        m_inputDevices.clear();
    } else {
        m_outputDevices =
                m_pSoundManager->getDeviceList(m_soundConfig.getAPI(), true, false);
        m_inputDevices =
                m_pSoundManager->getDeviceList(m_soundConfig.getAPI(), false, true);
    }
    emit refreshOutputDevices(m_outputDevices);
    emit refreshInputDevices(m_inputDevices);
}

void DlgPrefSound::updateProfileDeleteButton() {
    // prohibit deleting the last remaining profile
    delProfileButton->setEnabled(profileComboBox->count() > 1);
}

/// Called when any of the combo boxes in this dialog are changed. Enables the
/// apply button and marks that settings have been changed so that
/// DlgPrefSound::slotApply knows to apply them.
void DlgPrefSound::settingChanged() {
    if (m_loading) {
        return; // doesn't count if we're just loading prefs
    }
    m_settingsModified = true;
}

void DlgPrefSound::deviceSettingChanged() {
    if (m_loading) {
        return;
    }
    checkLatencyCompensation();
    m_settingsModified = true;
}

/// Slot called when the "Query Devices" button is clicked.
void DlgPrefSound::queryClicked() {
    ScopedWaitCursor cursor;
    m_pSoundManager->clearAndQueryDevices();
    updateAPIs();
}

void DlgPrefSound::slotResetToDefaults() {
    qDebug() << "     .";
    qDebug() << "     Dlg defaults:";
    //m_soundConfig.loadDefaults(SoundManagerConfig::ALL);
    //loadSettings(m_soundConfig);
    SoundManagerConfig newConfig(m_pSoundManager.get());
    newConfig.loadDefaults(m_pSoundManager.get(), SoundManagerConfig::ALL);
    loadSettings(newConfig);

    const auto keylockEngine = EngineBuffer::defaultKeylockEngine();
    const int index = keylockComboBox->findData(QVariant::fromValue(keylockEngine));
    DEBUG_ASSERT(index >= 0);
    if (index >= 0) {
        keylockComboBox->setCurrentIndex(index);
    }
    m_pKeylockEngine->set(static_cast<double>(keylockEngine));

    masterMixComboBox->setCurrentIndex(1);
    m_pMasterEnabled->set(1.0);

    masterDelaySpinBox->setValue(0.0);
    m_pMasterDelay->set(0.0);

    headDelaySpinBox->setValue(0.0);
    m_pHeadDelay->set(0.0);

    boothDelaySpinBox->setValue(0.0);
    m_pBoothDelay->set(0.0);

    // Enable talkover master output
    m_pMicMonitorMode->set(
        static_cast<double>(
            static_cast<int>(EngineMaster::MicMonitorMode::MASTER)));
    micMonitorModeComboBox->setCurrentIndex(
        micMonitorModeComboBox->findData(
            static_cast<int>(EngineMaster::MicMonitorMode::MASTER)));

    latencyCompensationSpinBox->setValue(latencyCompensationSpinBox->minimum());

    //updateProfileDeleteButton();

    settingChanged(); // force the apply button to enable
    qDebug() << "     .";
}

void DlgPrefSound::bufferUnderflow(double count) {
    bufferUnderflowCount->setText(QString::number(count));
    update();
}

void DlgPrefSound::masterLatencyChanged(double latency) {
    currentLatency->setText(QString("%1 ms").arg(latency));
    update();
}

void DlgPrefSound::latencyCompensationSpinboxChanged(double value) {
    m_pLatencyCompensation->set(value);
    checkLatencyCompensation();
}

void DlgPrefSound::masterDelaySpinboxChanged(double value) {
    m_pMasterDelay->set(value);
}

void DlgPrefSound::headDelaySpinboxChanged(double value) {
    m_pHeadDelay->set(value);
}

void DlgPrefSound::boothDelaySpinboxChanged(double value) {
    m_pBoothDelay->set(value);
}

void DlgPrefSound::masterMixChanged(int value) {
    m_pMasterEnabled->set(value);
}

void DlgPrefSound::masterEnabledChanged(double value) {
    const bool masterEnabled = (value != 0);
    masterMixComboBox->setCurrentIndex(masterEnabled ? 1 : 0);
}

void DlgPrefSound::masterOutputModeComboBoxChanged(int value) {
    m_pMasterMonoMixdown->set((double)value);
}

void DlgPrefSound::masterMonoMixdownChanged(double value) {
    const bool masterMonoMixdownEnabled = (value != 0);
    masterOutputModeComboBox->setCurrentIndex(masterMonoMixdownEnabled ? 1 : 0);
}

void DlgPrefSound::micMonitorModeComboBoxChanged(int value) {
    EngineMaster::MicMonitorMode newMode =
        static_cast<EngineMaster::MicMonitorMode>(
            micMonitorModeComboBox->itemData(value).toInt());

    m_pMicMonitorMode->set(static_cast<double>(newMode));

    checkLatencyCompensation();
}

void DlgPrefSound::checkLatencyCompensation() {
    EngineMaster::MicMonitorMode configuredMicMonitorMode =
        static_cast<EngineMaster::MicMonitorMode>(
            static_cast<int>(m_pMicMonitorMode->get()));

    // Do not clear the SoundManagerConfig on startup, from slotApply, or from slotUpdate
    if (!m_bSkipConfigClear) {
        m_soundConfig.clearInputs();
        m_soundConfig.clearOutputs();
    }

    emit writePaths(&m_soundConfig);

    if (m_soundConfig.hasMicInputs() && !m_soundConfig.hasExternalRecordBroadcast()) {
        micMonitorModeComboBox->setEnabled(true);
        if (configuredMicMonitorMode == EngineMaster::MicMonitorMode::DIRECT_MONITOR) {
            latencyCompensationSpinBox->setEnabled(true);
            QString warningIcon("<html><img src=':/images/preferences/ic_preferences_warning.png' width='20' height='20'></html> ");
            QString lineBreak("<br/>");
            // TODO(Be): Make the "User Manual" text link to the manual.
            if (m_pLatencyCompensation->get() == 0.0) {
                latencyCompensationWarningLabel->setText(
                      warningIcon +
                      tr("Microphone inputs are out of time in the record & broadcast signal compared to what you hear.") + lineBreak +
                      tr("Measure round trip latency and enter it above for Microphone Latency Compensation to align microphone timing.") + lineBreak +
                      tr("Refer to the Mixxx User Manual for details.") + "</html>");
                latencyCompensationWarningLabel->show();
            } else if (m_bLatencyChanged) {
                latencyCompensationWarningLabel->setText(
                  warningIcon +
                  tr("Configured latency has changed.") + lineBreak +
                  tr("Remeasure round trip latency and enter it above for Microphone Latency Compensation to align microphone timing.") + lineBreak +
                  tr("Refer to the Mixxx User Manual for details.") + "</html>");
                latencyCompensationWarningLabel->show();
            } else {
                latencyCompensationWarningLabel->hide();
            }
        } else {
            latencyCompensationSpinBox->setEnabled(false);
            latencyCompensationWarningLabel->hide();
        }
    } else {
        micMonitorModeComboBox->setEnabled(false);
        latencyCompensationSpinBox->setEnabled(false);
        latencyCompensationWarningLabel->hide();
    }
}
