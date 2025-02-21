#include "preferences/dialog/dlgprefsound.h"

#include <QMessageBox>
#include <QtDebug>
#include <algorithm>
#include <vector>

#include "control/controlproxy.h"
#include "engine/enginebuffer.h"
#include "engine/enginemixer.h"
#include "mixer/playermanager.h"
#include "moc_dlgprefsound.cpp"
#include "preferences/dialog/dlgprefsounditem.h"
#include "soundio/soundmanager.h"
#include "util/rlimit.h"
#include "util/scopedoverridecursor.h"

#ifdef __RUBBERBAND__
#include "engine/bufferscalers/rubberbandworkerpool.h"
#endif

namespace {

const QString kAppGroup = QStringLiteral("[App]");
const QString kMasterGroup = QStringLiteral("[Master]");
const ConfigKey kKeylockEngingeCfgkey =
        ConfigKey(kAppGroup, QStringLiteral("keylock_engine"));
const ConfigKey kKeylockMultiThreadingCfgkey =
        ConfigKey(kAppGroup, QStringLiteral("keylock_multithreading"));

bool soundItemAlreadyExists(const AudioPath& output, const QWidget& widget) {
    for (const QObject* pObj : widget.children()) {
        const auto* pItem = qobject_cast<const DlgPrefSoundItem*>(pObj);
        if (!pItem || pItem->type() != output.getType()) {
            continue;
        }
        if (!AudioPath::isIndexed(pItem->type()) || pItem->index() == output.getIndex()) {
            return true;
        }
    }
    return false;
}

#ifdef __RUBBERBAND__
const QString kKeylockMultiThreadedAvailable = QStringLiteral("<p>") +
        QObject::tr(
                "Distribute stereo channels into mono channels processed in "
                "parallel.") +
        QStringLiteral("</p><p><span style=\"font-weight:600;\">") +
        QObject::tr("Warning!") + QStringLiteral("</span></p><p>") +
        QObject::tr(
                "Processing stereo signal as mono channel "
                "may result in pitch and tone imperfection, and this "
                "is "
                "mono-incompatible, due to third party limitations.") +
        QStringLiteral("</p>");
const QString kKeylockMultiThreadedUnavailableMono = QStringLiteral("<i>") +
        QObject::tr(
                "Dual threading mode is incompatible with mono main mix.") +
        QStringLiteral("</i>");
const QString kKeylockMultiThreadedUnavailableRubberband =
        QStringLiteral("<i>") +
        QObject::tr("Dual threading mode is only available with RubberBand.") +
        QStringLiteral("</i>");
#endif
} // namespace

/// Construct a new sound preferences pane. Initializes and populates
/// all the controls to the values obtained from SoundManager.
DlgPrefSound::DlgPrefSound(QWidget* pParent,
        std::shared_ptr<SoundManager> pSoundManager,
        UserSettingsPointer pSettings)
        : DlgPreferencePage(pParent),
          m_pSoundManager(pSoundManager),
          m_pSettings(pSettings),
          m_config(pSoundManager.get()),
          m_pLatencyCompensation(kMasterGroup, QStringLiteral("microphoneLatencyCompensation")),
          m_pMainDelay(kMasterGroup, QStringLiteral("delay")),
          m_pHeadDelay(kMasterGroup, QStringLiteral("headDelay")),
          m_pBoothDelay(kMasterGroup, QStringLiteral("boothDelay")),
          m_pMicMonitorMode(kMasterGroup, QStringLiteral("talkover_mix")),
          m_pKeylockEngine(kKeylockEngingeCfgkey),
          m_settingsModified(false),
          m_bLatencyChanged(false),
          m_bSkipConfigClear(true),
          m_loading(false) {
    setupUi(this);
    // Create text color for the wiki links
    createLinkColor();

    connect(m_pSoundManager.get(),
            &SoundManager::devicesUpdated,
            this,
            &DlgPrefSound::refreshDevices);

    apiComboBox->clear();
    apiComboBox->addItem(SoundManagerConfig::kEmptyComboBox,
            SoundManagerConfig::kDefaultAPI);
    updateAPIs();
    connect(apiComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::apiChanged);

    sampleRateComboBox->clear();
    const auto sampleRates = m_pSoundManager->getSampleRates();
    for (const auto& sampleRate : sampleRates) {
        if (sampleRate.isValid()) {
            // no ridiculous sample rate values. prohibiting zero means
            // avoiding a potential div-by-0 error in ::updateLatencies
            sampleRateComboBox->addItem(tr("%1 Hz").arg(sampleRate.value()),
                    QVariant::fromValue(sampleRate));
        }
    }
    connect(sampleRateComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::sampleRateChanged);
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

    latencyCompensationSpinBox->setValue(m_pLatencyCompensation.get());
    latencyCompensationWarningLabel->setWordWrap(true);
    mainDelaySpinBox->setValue(m_pMainDelay.get());
    headDelaySpinBox->setValue(m_pHeadDelay.get());
    boothDelaySpinBox->setValue(m_pBoothDelay.get());

    // TODO These settings are applied immediately via ControlProxies.
    // While this is handy for testing the delays, it breaks the rule to
    // apply only in slotApply(). Add hint to UI?
    connect(latencyCompensationSpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::latencyCompensationSpinboxChanged);
    connect(mainDelaySpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::mainDelaySpinboxChanged);
    connect(headDelaySpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::headDelaySpinboxChanged);
    connect(boothDelaySpinBox,
            QOverload<double>::of(&QDoubleSpinBox::valueChanged),
            this,
            &DlgPrefSound::boothDelaySpinboxChanged);

    micMonitorModeComboBox->addItem(tr("Main output only"),
            QVariant(static_cast<int>(EngineMixer::MicMonitorMode::Main)));
    micMonitorModeComboBox->addItem(tr("Main and booth outputs"),
            QVariant(static_cast<int>(EngineMixer::MicMonitorMode::MainAndBooth)));
    micMonitorModeComboBox->addItem(tr("Direct monitor (recording and broadcasting only)"),
            QVariant(static_cast<int>(EngineMixer::MicMonitorMode::DirectMonitor)));
    int modeIndex = micMonitorModeComboBox->findData(
            static_cast<int>(m_pMicMonitorMode.get()));
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
#ifdef __RUBBERBAND__
    connect(keylockComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::updateKeylockDualThreadingCheckbox);
    connect(keylockDualthreadedCheckBox,
            &QCheckBox::clicked,
            this,
            &DlgPrefSound::updateKeylockMultithreading);
#else
    keylockDualthreadedCheckBox->hide();
#endif

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

    m_pAudioLatencyOverloadCount = make_parented<ControlProxy>(
            kAppGroup, QStringLiteral("audio_latency_overload_count"), this);
    m_pAudioLatencyOverloadCount->connectValueChanged(this, &DlgPrefSound::bufferUnderflow);

    m_pOutputLatencyMs = make_parented<ControlProxy>(
            kAppGroup, QStringLiteral("output_latency_ms"), this);
    m_pOutputLatencyMs->connectValueChanged(this, &DlgPrefSound::outputLatencyChanged);

    // TODO: remove this option by automatically disabling/enabling the main mix
    // when recording, broadcasting, headphone, and main outputs are enabled/disabled
    m_pMainEnabled =
            make_parented<ControlProxy>(kMasterGroup, QStringLiteral("enabled"), this);
    mainMixComboBox->addItem(tr("Disabled"));
    mainMixComboBox->addItem(tr("Enabled"));
    mainMixComboBox->setCurrentIndex(m_pMainEnabled->toBool() ? 1 : 0);
    connect(mainMixComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::mainMixChanged);
    m_pMainEnabled->connectValueChanged(this, &DlgPrefSound::mainEnabledChanged);

    m_pMainMonoMixdown =
            make_parented<ControlProxy>(kMasterGroup, QStringLiteral("mono_mixdown"), this);
    mainOutputModeComboBox->addItem(tr("Stereo"));
    mainOutputModeComboBox->addItem(tr("Mono"));
    mainOutputModeComboBox->setCurrentIndex(m_pMainMonoMixdown->toBool() ? 1 : 0);
    connect(mainOutputModeComboBox,
            QOverload<int>::of(&QComboBox::currentIndexChanged),
            this,
            &DlgPrefSound::mainOutputModeComboBoxChanged);
    m_pMainMonoMixdown->connectValueChanged(this, &DlgPrefSound::mainMonoMixdownChanged);

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

    setScrollSafeGuardForAllInputWidgets(this);

    hardwareGuide->setText(
            tr("The %1 lists sound cards and controllers you may want to "
               "consider for using Mixxx.")
                    .arg(coloredLinkString(
                            m_pLinkColor,
                            tr("Mixxx DJ Hardware Guide"),
                            MIXXX_WIKI_HARDWARE_COMPATIBILITY_URL)));
}

/// Slot called when the preferences dialog is opened.
void DlgPrefSound::slotUpdate() {
    m_bSkipConfigClear = true;
    loadSettings();
    checkLatencyCompensation();
    m_bSkipConfigClear = false;
}

/// Slot called when the Apply or OK button is pressed.
void DlgPrefSound::slotApply() {
    if (!m_settingsModified) {
        return;
    }

    m_config.clearInputs();
    m_config.clearOutputs();
    emit writePaths(&m_config);

    SoundDeviceStatus status = SoundDeviceStatus::Ok;
    {
        ScopedWaitCursor cursor;
        const auto keylockEngine =
                keylockComboBox->currentData().value<EngineBuffer::KeylockEngine>();

        // Temporary set an empty config to force the audio thread to stop and
        // stay off while we are swapping the keylock settings. This is
        // necessary because the audio thread doesn't have any synchronisation
        // mechanism due to its realtime nature and editing the RubberBand
        // config while it is running leads to race conditions.
        m_pSoundManager->closeActiveConfig();

        m_pKeylockEngine.set(static_cast<double>(keylockEngine));
        m_pSettings->set(kKeylockEngingeCfgkey,
                ConfigValue(static_cast<int>(keylockEngine)));

#ifdef __RUBBERBAND__
        bool keylockMultithreading = m_pSettings->getValue(
                kKeylockMultiThreadingCfgkey, false);
        m_pSettings->setValue(kKeylockMultiThreadingCfgkey,
                keylockDualthreadedCheckBox->isChecked() &&
                        keylockDualthreadedCheckBox->isEnabled());
        if (keylockMultithreading !=
                (keylockDualthreadedCheckBox->isChecked() &&
                        keylockDualthreadedCheckBox->isEnabled())) {
            QMessageBox::information(this,
                    tr("Information"),
                    tr("Mixxx must be restarted before the multi-threaded "
                       "RubberBand setting change will take effect."));
        }
#endif
        status = m_pSoundManager->setConfig(m_config);
    }
    if (status != SoundDeviceStatus::Ok) {
        QString error = m_pSoundManager->getLastErrorMessage(status);
        QMessageBox::warning(nullptr, tr("Configuration error"), error);
    } else {
        m_settingsModified = false;
        m_bLatencyChanged = false;
    }
    m_bSkipConfigClear = true;
    loadSettings(); // in case SM decided to change anything it didn't like
    checkLatencyCompensation();
#ifdef __RUBBERBAND__
    updateKeylockDualThreadingCheckbox();
#endif
    m_bSkipConfigClear = false;
}

QUrl DlgPrefSound::helpUrl() const {
    return QUrl(MIXXX_MANUAL_SOUND_URL);
}

void DlgPrefSound::selectIOTab(mixxx::preferences::SoundHardwareTab tab) {
    switch (tab) {
    case mixxx::preferences::SoundHardwareTab::Input:
        ioTabs->setCurrentWidget(inputTab);
        return;
    case mixxx::preferences::SoundHardwareTab::Output:
        ioTabs->setCurrentWidget(outputTab);
        return;
    }
}
/// Initializes (and creates) all the path items. Each path item widget allows
/// the user to input a sound device name and channel number given a description
/// of what will be done with that info. Inputs and outputs are grouped by tab,
/// and each path item has an identifier (Master, Headphones, ...) and an index,
/// if necessary.
void DlgPrefSound::initializePaths() {
    // Pre-sort paths so they're added in the order they'll appear later on
    // so Tab key order matches order in layout:
    // * by AudioPathType
    // * identical types by index
    auto sortFilterAdd = [this]<typename T>(const QList<T>& l) {
        // we use a vec of ref_wrappers since copying the path is unnecessary
        // and we really just want to change the order
        auto ref_vec_to_sort = std::vector<std::reference_wrapper<const T>>(l.begin(), l.end());
        std::sort(ref_vec_to_sort.begin(), ref_vec_to_sort.end());
        for (const T& path : ref_vec_to_sort) {
            if (!path.isHidden()) {
                addPath(path);
            }
        }
    };

    sortFilterAdd(m_pSoundManager->registeredOutputs());
    sortFilterAdd(m_pSoundManager->registeredInputs());
}

void DlgPrefSound::addPath(const AudioOutput& output) {
    // if we already know about this output, don't make a new entry

    if (soundItemAlreadyExists(output, *outputTab)) {
        return;
    }
    AudioPathType type = output.getType();
    // TODO who owns this?
    DlgPrefSoundItem* pSoundItem = new DlgPrefSoundItem(outputTab,
            type,
            m_outputDevices,
            false,
            AudioPath::isIndexed(type) ? output.getIndex() : 0);
    insertItem(pSoundItem, outputVLayout);
    connectSoundItem(pSoundItem);

    setScrollSafeGuardForAllInputWidgets(pSoundItem);
}

void DlgPrefSound::addPath(const AudioInput& input) {
    if (soundItemAlreadyExists(input, *inputTab)) {
        return;
    }
    AudioPathType type = input.getType();
    // TODO: who owns this?
    DlgPrefSoundItem* pSoundItem = new DlgPrefSoundItem(inputTab,
            type,
            m_inputDevices,
            true,
            AudioPath::isIndexed(type) ? input.getIndex() : 0);
    connectSoundItem(pSoundItem);
    insertItem(pSoundItem, inputVLayout);

    setScrollSafeGuardForAllInputWidgets(pSoundItem);
}

void DlgPrefSound::connectSoundItem(DlgPrefSoundItem* pItem) {
    connect(pItem,
            &DlgPrefSoundItem::selectedDeviceChanged,
            this,
            &DlgPrefSound::deviceChanged);
    connect(pItem,
            &DlgPrefSoundItem::selectedChannelsChanged,
            this,
            &DlgPrefSound::deviceChannelsChanged);
    connect(pItem,
            &DlgPrefSoundItem::configuredDeviceNotFound,
            this,
            &DlgPrefSound::configuredDeviceNotFound);
    connect(this, &DlgPrefSound::loadPaths, pItem, &DlgPrefSoundItem::loadPath);
    connect(this, &DlgPrefSound::writePaths, pItem, &DlgPrefSoundItem::writePath);
    if (pItem->isInput()) {
        connect(this, &DlgPrefSound::refreshInputDevices, pItem, &DlgPrefSoundItem::refreshDevices);
    } else {
        connect(this,
                &DlgPrefSound::refreshOutputDevices,
                pItem,
                &DlgPrefSoundItem::refreshDevices);
    }
    connect(this, &DlgPrefSound::updatingAPI, pItem, &DlgPrefSoundItem::save);
    connect(this, &DlgPrefSound::updatedAPI, pItem, &DlgPrefSoundItem::reload);
}

void DlgPrefSound::insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout) {
    int pos;
    for (pos = 0; pos < pLayout->count() - 1; ++pos) {
        DlgPrefSoundItem *pOther(qobject_cast<DlgPrefSoundItem*>(
            pLayout->itemAt(pos)->widget()));
        if (!pOther) { // not a sound item, skip
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
    loadSettings(m_pSoundManager->getConfig());
}

/// Loads the settings in the given SoundManagerConfig into the dialog.
void DlgPrefSound::loadSettings(const SoundManagerConfig& config) {
    m_loading = true; // so settingsChanged ignores all our modifications here
    m_config = config;
    int apiIndex = apiComboBox->findData(m_config.getAPI());
    if (apiIndex != -1) {
        apiComboBox->setCurrentIndex(apiIndex);
    }
    int sampleRateIndex = sampleRateComboBox->findData(
            QVariant::fromValue(m_config.getSampleRate()));
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

    // Default keylock engine is Rubberband Faster (v2)
    const auto keylockEngine = static_cast<EngineBuffer::KeylockEngine>(
            m_pSettings->getValue(kKeylockEngingeCfgkey,
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

#ifdef __RUBBERBAND__
    // Default is no multi threading on keylock
    keylockDualthreadedCheckBox->setChecked(m_pSettings->getValue(
            kKeylockMultiThreadingCfgkey,
            false));
#endif

    // Collect selected I/O channel indices for all non-empty device comboboxes
    // in order to allow auto-selecting free channels when different devices are
    // selected later on, when a different device is selected for any I/O.
    m_selectedOutputChannelIndices.clear();
    m_selectedInputChannelIndices.clear();
    for (auto* ch : std::as_const(outputTab->children())) {
        DlgPrefSoundItem* pItem = qobject_cast<DlgPrefSoundItem*>(ch);
        if (pItem) {
            auto id = pItem->getDeviceId();
            if (id == SoundDeviceId()) {
                continue;
            }
            m_selectedOutputChannelIndices.insert(pItem,
                    QPair<SoundDeviceId, int>(id, pItem->getChannelIndex()));
        }
    }
    for (auto* ch : std::as_const(inputTab->children())) {
        DlgPrefSoundItem* pItem = qobject_cast<DlgPrefSoundItem*>(ch);
        if (pItem) {
            auto id = pItem->getDeviceId();
            if (id == SoundDeviceId()) {
                continue;
            }
            m_selectedInputChannelIndices.insert(pItem,
                    QPair<SoundDeviceId, int>(id, pItem->getChannelIndex()));
        }
    }
    m_loading = false;
    // DlgPrefSoundItem has it's own inhibit flag
    emit loadPaths(m_config);
}

/// Slot called when the user selects a different API, or the
/// software changes it programmatically (for instance, when it
/// loads a value from SoundManager). Refreshes the device lists
/// for the new API and pushes those to the path items.
void DlgPrefSound::apiChanged(int index) {
    m_config.setAPI(apiComboBox->itemData(index).toString());
    refreshDevices();
    // JACK sets its own buffer size and sample rate that Mixxx cannot change.
    // PortAudio is able to chop/combine the buffer but that will mess up the
    // timing in Mixxx. When we request 0 (paFramesPerBufferUnspecified)
    // https://github.com/PortAudio/portaudio/blob/v19.7.0/src/common/pa_process.c#L54
    // PortAudio passes buffers up to 1024 frames through.
    // For bigger buffers the user has to manually match the value with Jack.
    // TODO(Be): Get the buffer size from JACK and update audioBufferComboBox.
    // PortAudio as off v19.7.0 does not have a way to get the buffer size from JACK.
    bool enable = m_config.getAPI() == MIXXX_PORTAUDIO_JACK_STRING ? false : true;
    sampleRateComboBox->setEnabled(enable);
    deviceSyncComboBox->setEnabled(enable);
    engineClockComboBox->setEnabled(enable);
    updateAudioBufferSizes(sampleRateComboBox->currentIndex());
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
    m_config.setSampleRate(sampleRateComboBox->itemData(index).value<mixxx::audio::SampleRate>());
    m_bLatencyChanged = true;
    updateAudioBufferSizes(index);
    checkLatencyCompensation();
}

/// Slot called when the latency combo box is changed to update the
/// latency in the config.
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
    QVariant oldSizeIndex = audioBufferComboBox->currentData();
    audioBufferComboBox->clear();
    if (m_config.getAPI() == MIXXX_PORTAUDIO_JACK_STRING) {
        // in case of jack we configure the frames/period
        // we cannot calc the resulting buffer size in ms because the
        // Sample rate is not known yet. We assume 48000 KHz here
        // to calculate the buffer size index
        audioBufferComboBox->addItem(tr("auto (<= 1024 frames/period)"),
                static_cast<unsigned int>(SoundManagerConfig::
                                JackAudioBufferSizeIndex::SizeAuto));
        audioBufferComboBox->addItem(tr("2048 frames/period"),
                static_cast<unsigned int>(SoundManagerConfig::
                                JackAudioBufferSizeIndex::Size2048fpp));
        audioBufferComboBox->addItem(tr("4096 frames/period"),
                static_cast<unsigned int>(SoundManagerConfig::
                                JackAudioBufferSizeIndex::Size4096fpp));
    } else {
        DEBUG_ASSERT(sampleRateComboBox->itemData(sampleRateIndex)
                             .canConvert<mixxx::audio::SampleRate>());
        double sampleRate = sampleRateComboBox->itemData(sampleRateIndex)
                                    .value<mixxx::audio::SampleRate>()
                                    .toDouble();
        unsigned int framesPerBuffer = 1; // start this at 0 and inf loop happens
        // we don't want to display any sub-1ms buffer sizes (well maybe we do but I
        // don't right now!), so we iterate over all the buffer sizes until we
        // find the first that gives us a buffer size >= 1 ms -- bkgood
        // no div-by-0 in the next line because we don't allow srates of 0 in our
        // srate list when we construct it in the ctor -- bkgood
        for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2) {
        }
        for (unsigned int i = 0; i < SoundManagerConfig::kMaxAudioBufferSizeIndex; ++i) {
            const auto latency = static_cast<float>(framesPerBuffer / sampleRate * 1000);
            // i + 1 in the next line is a latency index as described in SSConfig
            audioBufferComboBox->addItem(tr("%1 ms").arg(latency, 0, 'g', 3), i + 1);
            framesPerBuffer <<= 1; // *= 2
        }
    }
    int selectionIndex = audioBufferComboBox->findData(oldSizeIndex);
    if (selectionIndex > -1) {
        audioBufferComboBox->setCurrentIndex(selectionIndex);
    } else {
        // use our default of 5 (23 ms @ 48 kHz)
        selectionIndex = audioBufferComboBox->findData(
                SoundManagerConfig::kDefaultAudioBufferSizeIndex);
        VERIFY_OR_DEBUG_ASSERT(selectionIndex > -1) {
            return;
        }
        audioBufferComboBox->setCurrentIndex(selectionIndex);
    }
}

/// Slot called when device lists go bad to refresh them, or the API
/// just changes and we need to display new devices.
void DlgPrefSound::refreshDevices() {
    if (m_config.getAPI() == SoundManagerConfig::kDefaultAPI) {
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

/// Called when any of the combo boxes in this dialog are changed. Enables the
/// apply button and marks that settings have been changed so that
/// DlgPrefSound::slotApply knows to apply them.
void DlgPrefSound::settingChanged() {
    if (m_loading) {
        return; // doesn't count if we're just loading prefs
    }
    m_settingsModified = true;
}

#ifdef __RUBBERBAND__
void DlgPrefSound::updateKeylockDualThreadingCheckbox() {
    bool supportedScaler = keylockComboBox->currentData()
                                   .value<EngineBuffer::KeylockEngine>() !=
            EngineBuffer::KeylockEngine::SoundTouch;
    bool monoMix = mainOutputModeComboBox->currentIndex() == 1;
    keylockDualthreadedCheckBox->setEnabled(!monoMix && supportedScaler);
    keylockDualthreadedCheckBox->setToolTip(monoMix
                    ? kKeylockMultiThreadedUnavailableMono
                    : (supportedScaler
                                      ? kKeylockMultiThreadedAvailable
                                      : kKeylockMultiThreadedUnavailableRubberband));
}

void DlgPrefSound::updateKeylockMultithreading(bool enabled) {
    m_settingsModified = true;
    if (!enabled) {
        return;
    }
    QMessageBox msg;
    msg.setIcon(QMessageBox::Warning);
    msg.setWindowTitle(tr("Are you sure?"));
    msg.setText(
            QStringLiteral("<p>%1</p><p>%2</p>")
                    .arg(tr("Distribute stereo channels into mono channels for "
                            "parallel processing will result in a loss of "
                            "mono compatibility and a diffuse stereo "
                            "image. It is not recommended during "
                            "broadcasting or recording."),
                            tr("Are you sure you wish to proceed?")));
    QPushButton* pNoBtn = msg.addButton(tr("No"), QMessageBox::AcceptRole);
    QPushButton* pYesBtn = msg.addButton(
            tr("Yes, I know what I am doing"), QMessageBox::RejectRole);
    msg.setDefaultButton(pNoBtn);
    msg.exec();
    keylockDualthreadedCheckBox->setChecked(msg.clickedButton() == pYesBtn);

    updateKeylockDualThreadingCheckbox();
}
#endif

/// Slot called when a device from the config can not be selected, i.e. is
/// currently not available. This may happen during startup when MixxxMainWindow
/// opens this page to allow users to make adjustments in case configured
/// devices are busy/missing.
/// The issue is that the visual state (combobox(es) with 'None') does not match
/// the untouched config state. This sets the modified flag so slotApply() will
/// apply the (seemingly) unchanged configuration if users simply click Apply/Okay
/// because they are okay to continue without these devices.
void DlgPrefSound::configuredDeviceNotFound() {
    m_settingsModified = true;
}

void DlgPrefSound::deviceChanged() {
    if (m_loading) {
        return;
    }

    DlgPrefSoundItem* pItem = qobject_cast<DlgPrefSoundItem*>(sender());
    if (!pItem) {
        return;
    }
    QHash<DlgPrefSoundItem*, QPair<SoundDeviceId, int>>* channels;
    if (pItem->isInput()) {
        channels = &m_selectedInputChannelIndices;
    } else {
        channels = &m_selectedOutputChannelIndices;
    }
    auto id = pItem->getDeviceId();
    if (id == SoundDeviceId()) {
        if (channels->contains(pItem)) {
            channels->remove(pItem);
        }
    } else {
        QList<int> selectedChannelsForDevice;
        QHashIterator<DlgPrefSoundItem*, QPair<SoundDeviceId, int>> it(
                pItem->isInput()
                        ? m_selectedInputChannelIndices
                        : m_selectedOutputChannelIndices);
        while (it.hasNext()) {
            it.next();
            if (it.value().first == id) {
                selectedChannelsForDevice.append(it.value().second);
            }
        }
        pItem->selectFirstUnusedChannelIndex(selectedChannelsForDevice);
    }

    checkLatencyCompensation();
    m_settingsModified = true;
}

void DlgPrefSound::deviceChannelsChanged() {
    if (m_loading) {
        return;
    }
    DlgPrefSoundItem* pItem = qobject_cast<DlgPrefSoundItem*>(sender());
    if (!pItem) {
        return;
    }
    auto id = pItem->getDeviceId();
    int index = pItem->getChannelIndex();
    if (id != SoundDeviceId()) {
        if (pItem->isInput()) {
            m_selectedInputChannelIndices.insert(pItem, QPair<SoundDeviceId, int>(id, index));
        } else {
            m_selectedOutputChannelIndices.insert(pItem, QPair<SoundDeviceId, int>(id, index));
        }
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

/// Slot called when the "Reset to Defaults" button is clicked.
void DlgPrefSound::slotResetToDefaults() {
    SoundManagerConfig newConfig(m_pSoundManager.get());
    newConfig.loadDefaults(m_pSoundManager.get(), SoundManagerConfig::ALL);
    loadSettings(newConfig);

    const auto keylockEngine = EngineBuffer::defaultKeylockEngine();
    const int index = keylockComboBox->findData(QVariant::fromValue(keylockEngine));
    DEBUG_ASSERT(index >= 0);
    if (index >= 0) {
        keylockComboBox->setCurrentIndex(index);
    }
    m_pKeylockEngine.set(static_cast<double>(keylockEngine));

    mainMixComboBox->setCurrentIndex(1);
    m_pMainEnabled->set(1.0);

    mainDelaySpinBox->setValue(0.0);
    m_pMainDelay.set(0.0);

    headDelaySpinBox->setValue(0.0);
    m_pHeadDelay.set(0.0);

    boothDelaySpinBox->setValue(0.0);
    m_pBoothDelay.set(0.0);

    // Enable talkover main output
    m_pMicMonitorMode.set(
            static_cast<double>(
                    static_cast<int>(EngineMixer::MicMonitorMode::Main)));
    micMonitorModeComboBox->setCurrentIndex(
            micMonitorModeComboBox->findData(
                    static_cast<int>(EngineMixer::MicMonitorMode::Main)));

    latencyCompensationSpinBox->setValue(latencyCompensationSpinBox->minimum());

    settingChanged();
#ifdef __RUBBERBAND__
    updateKeylockDualThreadingCheckbox();
#endif
}

void DlgPrefSound::bufferUnderflow(double count) {
    bufferUnderflowCount->setText(QString::number(count));
    update();
}

void DlgPrefSound::outputLatencyChanged(double latency) {
    currentLatency->setText(QString("%1 ms").arg(latency));
    update();
}

void DlgPrefSound::latencyCompensationSpinboxChanged(double value) {
    m_pLatencyCompensation.set(value);
    checkLatencyCompensation();
}

void DlgPrefSound::mainDelaySpinboxChanged(double value) {
    m_pMainDelay.set(value);
}

void DlgPrefSound::headDelaySpinboxChanged(double value) {
    m_pHeadDelay.set(value);
}

void DlgPrefSound::boothDelaySpinboxChanged(double value) {
    m_pBoothDelay.set(value);
}

void DlgPrefSound::mainMixChanged(int value) {
    m_pMainEnabled->set(value);
}

void DlgPrefSound::mainEnabledChanged(double value) {
    const bool mainEnabled = (value != 0);
    mainMixComboBox->setCurrentIndex(mainEnabled ? 1 : 0);
}

void DlgPrefSound::mainOutputModeComboBoxChanged(int value) {
    m_pMainMonoMixdown->set(static_cast<double>(value));

#ifdef __RUBBERBAND__
    updateKeylockDualThreadingCheckbox();
#endif
}

void DlgPrefSound::mainMonoMixdownChanged(double value) {
    const bool mainMonoMixdownEnabled = (value != 0);
    mainOutputModeComboBox->setCurrentIndex(mainMonoMixdownEnabled ? 1 : 0);
}

void DlgPrefSound::micMonitorModeComboBoxChanged(int value) {
    EngineMixer::MicMonitorMode newMode =
            static_cast<EngineMixer::MicMonitorMode>(
                    micMonitorModeComboBox->itemData(value).toInt());

    m_pMicMonitorMode.set(static_cast<double>(newMode));

    checkLatencyCompensation();
}

void DlgPrefSound::checkLatencyCompensation() {
    EngineMixer::MicMonitorMode configuredMicMonitorMode =
            static_cast<EngineMixer::MicMonitorMode>(
                    static_cast<int>(m_pMicMonitorMode.get()));

    // Do not clear the SoundManagerConfig on startup, from slotApply, or from slotUpdate
    if (!m_bSkipConfigClear) {
        m_config.clearInputs();
        m_config.clearOutputs();
    }

    emit writePaths(&m_config);

    if (m_config.hasMicInputs() && !m_config.hasExternalRecordBroadcast()) {
        micMonitorModeComboBox->setEnabled(true);
        if (configuredMicMonitorMode == EngineMixer::MicMonitorMode::DirectMonitor) {
            latencyCompensationSpinBox->setEnabled(true);
            QString lineBreak("<br/>");
            // TODO(Be): Make the "User Manual" text link to the manual.
            if (m_pLatencyCompensation.get() == 0.0) {
                latencyCompensationWarningLabel->setText(kWarningIconHtmlString +
                        tr("Microphone inputs are out of time in the record & "
                           "broadcast signal compared to what you hear.") +
                        lineBreak +
                        tr("Measure round trip latency and enter it above for "
                           "Microphone Latency Compensation to align "
                           "microphone timing.") +
                        lineBreak +
                        tr("Refer to the Mixxx User Manual for details.") +
                        "</html>");
                latencyCompensationWarningLabel->show();
            } else if (m_bLatencyChanged) {
                latencyCompensationWarningLabel->setText(kWarningIconHtmlString +
                        tr("Configured latency has changed.") + lineBreak +
                        tr("Remeasure round trip latency and enter it above "
                           "for Microphone Latency Compensation to align "
                           "microphone timing.") +
                        lineBreak +
                        tr("Refer to the Mixxx User Manual for details.") +
                        "</html>");
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
