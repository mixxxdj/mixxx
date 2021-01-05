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

/**
 * Construct a new sound preferences pane. Initializes and populates all the
 * all the controls to the values obtained from SoundManager.
 */
DlgPrefSound::DlgPrefSound(QWidget* pParent,
        SoundManager* pSoundManager,
        UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSoundManager(pSoundManager),
          m_pSettings(pSettings),
          m_config(pSoundManager),
          m_settingsModified(false),
          m_bLatencyChanged(false),
          m_bSkipConfigClear(true),
          m_loading(false) {
    setupUi(this);

    connect(m_pSoundManager, &SoundManager::devicesUpdated, this, &DlgPrefSound::refreshDevices);

    apiComboBox->clear();
    apiComboBox->addItem(tr("None"), "None");
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
    for (int i = 0; i < EngineBuffer::KEYLOCK_ENGINE_COUNT; ++i) {
        keylockComboBox->addItem(
                EngineBuffer::getKeylockEngineName(
                        static_cast<EngineBuffer::KeylockEngine>(i)));
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
    micMonitorModeComboBox->addItem(tr("Master output only"),
        QVariant(static_cast<int>(EngineMaster::MicMonitorMode::MASTER)));
    micMonitorModeComboBox->addItem(tr("Master and booth outputs"),
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

    connect(m_pSoundManager,
            &SoundManager::outputRegistered,
            this,
            [this](const AudioOutput& output, AudioSource* source) {
                Q_UNUSED(source);
                addPath(output);
                loadSettings();
            });

    connect(m_pSoundManager,
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
        limitsHint->setText(tr("Realtime scheduling is enabled."));
    }
#else
    // the limits warning is a Linux only thing
    limitsHint->hide();
#endif // __LINUX__

    // Set the focus policy for QComboBoxes (and wide QDoubleSpinBoxes) and
    // connect them to the custom event filter below so they don't accept focus
    // when we scroll the preferences page.
    QObjectList objList = this->children();
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

/**
 * Slot called when the preferences dialog is opened or this pane is
 * selected.
 */
void DlgPrefSound::slotUpdate() {
    // this is unfortunate, because slotUpdate is called every time
    // we change to this pane, we lose changed and unapplied settings
    // every time. There's no real way around this, just another argument
    // for a prefs rewrite -- bkgood
    m_bSkipConfigClear = true;
    loadSettings();
    checkLatencyCompensation();
    m_bSkipConfigClear = false;
    m_settingsModified = false;
}

/**
 * Slot called when the Apply or OK button is pressed.
 */
void DlgPrefSound::slotApply() {
    if (!m_settingsModified) {
        return;
    }

    m_config.clearInputs();
    m_config.clearOutputs();
    emit writePaths(&m_config);

    SoundDeviceError err = SOUNDDEVICE_ERROR_OK;
    {
        ScopedWaitCursor cursor;
        m_pKeylockEngine->set(keylockComboBox->currentIndex());
        m_pSettings->set(ConfigKey("[Master]", "keylock_engine"),
                       ConfigValue(keylockComboBox->currentIndex()));

        err = m_pSoundManager->setConfig(m_config);
    }
    if (err != SOUNDDEVICE_ERROR_OK) {
        QString error = m_pSoundManager->getLastErrorMessage(err);
        QMessageBox::warning(nullptr, tr("Configuration error"), error);
    } else {
        m_settingsModified = false;
        m_bLatencyChanged = false;
    }
    m_bSkipConfigClear = true;
    loadSettings(); // in case SM decided to change anything it didn't like
    checkLatencyCompensation();
    m_bSkipConfigClear = false;
}

QUrl DlgPrefSound::helpUrl() const {
    return QUrl(MIXXX_MANUAL_SOUND_URL);
}

/**
 * Initializes (and creates) all the path items. Each path item widget allows
 * the user to input a sound device name and channel number given a description
 * of what will be done with that info. Inputs and outputs are grouped by tab,
 * and each path item has an identifier (Master, Headphones, ...) and an index,
 * if necessary.
 */
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

/**
 * Convenience overload to load settings from the SoundManagerConfig owned by
 * SoundManager.
 */
void DlgPrefSound::loadSettings() {
    loadSettings(m_pSoundManager->getConfig());
}

/**
 * Loads the settings in the given SoundManagerConfig into the dialog.
 */
void DlgPrefSound::loadSettings(const SoundManagerConfig &config) {
    m_loading = true; // so settingsChanged ignores all our modifications here
    m_config = config;
    int apiIndex = apiComboBox->findData(m_config.getAPI());
    if (apiIndex != -1) {
        apiComboBox->setCurrentIndex(apiIndex);
    }
    int sampleRateIndex = sampleRateComboBox->findData(m_config.getSampleRate());
    if (sampleRateIndex != -1) {
        sampleRateComboBox->setCurrentIndex(sampleRateIndex);
        if (audioBufferComboBox->count() <= 0) {
            updateAudioBufferSizes(sampleRateIndex); // so the latency combo box is
            // sure to be populated, if setCurrentIndex is called with the
            // currentIndex, the currentIndexChanged signal won't fire and
            // the updateLatencies slot won't run -- bkgood lp bug 689373
        }
    }
    int sizeIndex = audioBufferComboBox->findData(m_config.getAudioBufferSizeIndex());
    if (sizeIndex != -1) {
        audioBufferComboBox->setCurrentIndex(sizeIndex);
    }

    // Setting the index of audioBufferComboBox here sets m_bLatencyChanged to true,
    // but m_bLatencyChanged should only be true when the user has edited the
    // buffer size or sample rate.
    m_bLatencyChanged = false;

    int syncBuffers = m_config.getSyncBuffers();
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

    if (m_config.getForceNetworkClock()) {
        engineClockComboBox->setCurrentIndex(1);
    } else {
        engineClockComboBox->setCurrentIndex(0);
    }

    // Default keylock is Rubberband.
    int keylock_engine = m_pSettings->getValue(
            ConfigKey("[Master]", "keylock_engine"), 1);
    keylockComboBox->setCurrentIndex(keylock_engine);

    m_loading = false;
    // DlgPrefSoundItem has it's own inhibit flag
    emit loadPaths(m_config);
}

/**
 * Slot called when the user selects a different API, or the
 * software changes it programmatically (for instance, when it
 * loads a value from SoundManager). Refreshes the device lists
 * for the new API and pushes those to the path items.
 */
void DlgPrefSound::apiChanged(int index) {
    m_config.setAPI(apiComboBox->itemData(index).toString());
    refreshDevices();
    // JACK sets its own buffer size and sample rate that Mixxx cannot change.
    // TODO(Be): Get the buffer size from JACK and update audioBufferComboBox.
    // PortAudio does not have a way to get the buffer size from JACK as of July 2017.
    if (m_config.getAPI() == MIXXX_PORTAUDIO_JACK_STRING) {
        sampleRateComboBox->setEnabled(false);
        latencyLabel->setEnabled(false);
        audioBufferComboBox->setEnabled(false);
    } else {
        sampleRateComboBox->setEnabled(true);
        latencyLabel->setEnabled(true);
        audioBufferComboBox->setEnabled(true);
    }
}

/**
 * Updates the list of APIs, trying to keep the API and device selections
 * constant if possible.
 */
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

/**
 * Slot called when the sample rate combo box changes to update the
 * sample rate in the config.
 */
void DlgPrefSound::sampleRateChanged(int index) {
    m_config.setSampleRate(
            sampleRateComboBox->itemData(index).toUInt());
    m_bLatencyChanged = true;
    checkLatencyCompensation();
}

/**
 * Slot called when the latency combo box is changed to update the
 * latency in the config.
 */
void DlgPrefSound::audioBufferChanged(int index) {
    m_config.setAudioBufferSizeIndex(
            audioBufferComboBox->itemData(index).toUInt());
    m_bLatencyChanged = true;
    checkLatencyCompensation();
}

void DlgPrefSound::syncBuffersChanged(int index) {
    if (index == 0) {
        // "Default (long delay)" = 2 buffer
        m_config.setSyncBuffers(2);
    } else if (index == 1) {
        // "Experimental (no delay)")) = 0 buffer
        m_config.setSyncBuffers(0);
    } else {
        // "Disabled (short delay)")) = 1 buffer
        m_config.setSyncBuffers(1);
    }
}

void DlgPrefSound::engineClockChanged(int index) {
    if (index == 0) {
        // "Soundcard Clock"
        m_config.setForceNetworkClock(false);
    } else {
        // "Network Clock"
        m_config.setForceNetworkClock(true);
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

/**
 * Slot called when device lists go bad to refresh them, or the API
 * just changes and we need to display new devices.
 */
void DlgPrefSound::refreshDevices() {
    if (m_config.getAPI() == "None") {
        m_outputDevices.clear();
        m_inputDevices.clear();
    } else {
        m_outputDevices =
            m_pSoundManager->getDeviceList(m_config.getAPI(), true, false);
        m_inputDevices =
            m_pSoundManager->getDeviceList(m_config.getAPI(), false, true);
    }
    emit refreshOutputDevices(m_outputDevices);
    emit refreshInputDevices(m_inputDevices);
}

/**
 * Called when any of the combo boxes in this dialog are changed. Enables the
 * apply button and marks that settings have been changed so that
 * DlgPrefSound::slotApply knows to apply them.
 */
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

/**
 * Slot called when the "Query Devices" button is clicked.
 */
void DlgPrefSound::queryClicked() {
    ScopedWaitCursor cursor;
    m_pSoundManager->clearAndQueryDevices();
    updateAPIs();
}

/**
 * Slot called when the "Reset to Defaults" button is clicked.
 */
void DlgPrefSound::slotResetToDefaults() {
    SoundManagerConfig newConfig(m_pSoundManager);
    newConfig.loadDefaults(m_pSoundManager, SoundManagerConfig::ALL);
    loadSettings(newConfig);
    keylockComboBox->setCurrentIndex(EngineBuffer::RUBBERBAND);
    m_pKeylockEngine->set(EngineBuffer::RUBBERBAND);

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

    settingChanged(); // force the apply button to enable
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
        m_config.clearInputs();
        m_config.clearOutputs();
    }

    emit writePaths(&m_config);

    if (m_config.hasMicInputs() && !m_config.hasExternalRecordBroadcast()) {
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
