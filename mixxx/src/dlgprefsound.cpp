/**
 * @file dlgprefsound.cpp
 * @author Bill Good <bkgood at gmail dot com>
 * @date 20100625
 */

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#include <QDebug>
#include <QMessageBox>
#include "dlgprefsound.h"
#include "dlgprefsounditem.h"
#include "engine/enginemaster.h"
#include "playermanager.h"
#include "soundmanager.h"
#include "sounddevice.h"

/**
 * Construct a new sound preferences pane. Initializes and populates all the
 * all the controls to the values obtained from SoundManager.
 */
DlgPrefSound::DlgPrefSound(QWidget *pParent, SoundManager *pSoundManager,
                           PlayerManager* pPlayerManager, ConfigObject<ConfigValue> *pConfig)
        : QWidget(pParent)
        , m_pSoundManager(pSoundManager)
        , m_pPlayerManager(pPlayerManager)
        , m_pConfig(pConfig)
        , m_settingsModified(false)
        , m_loading(false)
        , m_forceApply(false)
{
    setupUi(this);

    connect(m_pSoundManager, SIGNAL(devicesUpdated()),
            this, SLOT(refreshDevices()));

    applyButton->setEnabled(false);
    connect(applyButton, SIGNAL(clicked()),
            this, SLOT(slotApply()));

    apiComboBox->clear();
    apiComboBox->addItem(tr("None"), "None");
    updateAPIs();
    connect(apiComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(apiChanged(int)));

    sampleRateComboBox->clear();
    foreach (unsigned int srate, m_pSoundManager->getSampleRates()) {
        if (srate > 0) {
            // no ridiculous sample rate values. prohibiting zero means
            // avoiding a potential div-by-0 error in ::updateLatencies
            sampleRateComboBox->addItem(QString(tr("%1 Hz")).arg(srate), srate);
        }
    }
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(sampleRateChanged(int)));
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(updateLatencies(int)));
    connect(latencyComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(latencyChanged(int)));

    initializePaths();
    loadSettings();

    connect(apiComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
    connect(sampleRateComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));
    connect(latencyComboBox, SIGNAL(currentIndexChanged(int)),
            this, SLOT(settingChanged()));

    connect(queryButton, SIGNAL(clicked()),
            this, SLOT(queryClicked()));
    connect(resetButton, SIGNAL(clicked()),
            this, SLOT(resetClicked()));

    connect(m_pSoundManager, SIGNAL(outputRegistered(AudioOutput, const AudioSource*)),
            this, SLOT(addPath(AudioOutput)));
    connect(m_pSoundManager, SIGNAL(outputRegistered(AudioOutput, const AudioSource*)),
            this, SLOT(loadSettings()));

    connect(m_pSoundManager, SIGNAL(inputRegistered(AudioInput, AudioDestination*)),
            this, SLOT(addPath(AudioInput)));
    connect(m_pSoundManager, SIGNAL(inputRegistered(AudioInput, AudioDestination*)),
            this, SLOT(loadSettings()));

    m_pMasterUnderflowCount =
                    new ControlObjectThreadMain(ControlObject::getControl(ConfigKey("[Master]", "underflow_count")));
    connect(m_pMasterUnderflowCount, SIGNAL(valueChanged(double)),
            this, SLOT(bufferUnderflow(double)));
}

DlgPrefSound::~DlgPrefSound() {
    delete m_pMasterUnderflowCount;
}

/**
 * Slot called when the preferences dialog  is opened or this pane is
 * selected.
 */
void DlgPrefSound::slotUpdate() {
    // this is unfortunate, because slotUpdate is called every time
    // we change to this pane, we lose changed and unapplied settings
    // every time. There's no real way around this, just anothe argument
    // for a prefs rewrite -- bkgood
    loadSettings();
    m_settingsModified = false;
    applyButton->setEnabled(false);
}

/**
 * Slot called when the Apply or OK button is pressed.
 */
void DlgPrefSound::slotApply() {
    if (!m_settingsModified && !m_forceApply) {
        return;
    }
    m_forceApply = false;
    m_config.clearInputs();
    m_config.clearOutputs();
    emit(writePaths(&m_config));
    int err = m_pSoundManager->setConfig(m_config);
    if (err != OK) {
        QString error;
        QString deviceName(tr("a device"));
        QString detailedError(tr("An unknown error occurred"));
        SoundDevice *device = m_pSoundManager->getErrorDevice();
        if (device != NULL) {
            deviceName = QString(tr("sound device \"%1\"")).arg(device->getDisplayName());
            detailedError = device->getError();
        }
        switch (err) {
        case SOUNDDEVICE_ERROR_DUPLICATE_OUTPUT_CHANNEL:
            error = QString(tr("Two outputs cannot share channels on %1")).arg(deviceName);
            break;
        default:
            error = QString(tr("Error opening %1\n%2")).arg(deviceName, detailedError);
            break;
        }
        QMessageBox::warning(NULL, tr("Configuration error"), error);
    }
    m_settingsModified = false;
    applyButton->setEnabled(false);
    loadSettings(); // in case SM decided to change anything it didn't like
}

/**
 * Slot called by DlgPrefVinyl when it needs slotApply here to call setupDevices.
 * We're graced with this kludge because VC proxies are only initialized in
 * SoundManager::setupDevices and reinit is the only way to make them reread
 * their config.
 */
void DlgPrefSound::forceApply() {
    m_forceApply = true;
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
        addPath(out);
    }
    foreach (AudioInput in, m_pSoundManager->registeredInputs()) {
        addPath(in);
    }
}

void DlgPrefSound::addPath(AudioOutput output) {
    DlgPrefSoundItem *toInsert;
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
    AudioPathType type = output.getType();
    if (AudioPath::isIndexed(type)) {
        toInsert = new DlgPrefSoundItem(outputTab, type,
            m_outputDevices, false, output.getIndex());
    } else {
        toInsert = new DlgPrefSoundItem(outputTab, type,
            m_outputDevices, false);
    }
    connect(this, SIGNAL(refreshOutputDevices(const QList<SoundDevice*>&)),
            toInsert, SLOT(refreshDevices(const QList<SoundDevice*>&)));
    insertItem(toInsert, outputVLayout);
    connectSoundItem(toInsert);
}

void DlgPrefSound::addPath(AudioInput input) {
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
    connect(this, SIGNAL(refreshInputDevices(const QList<SoundDevice*>&)),
            toInsert, SLOT(refreshDevices(const QList<SoundDevice*>&)));
    insertItem(toInsert, inputVLayout);
    connectSoundItem(toInsert);
}

void DlgPrefSound::connectSoundItem(DlgPrefSoundItem *item) {
    connect(item, SIGNAL(settingChanged()),
            this, SLOT(settingChanged()));
    connect(this, SIGNAL(loadPaths(const SoundManagerConfig&)),
            item, SLOT(loadPath(const SoundManagerConfig&)));
    connect(this, SIGNAL(writePaths(SoundManagerConfig*)),
            item, SLOT(writePath(SoundManagerConfig*)));
    connect(this, SIGNAL(updatingAPI()),
            item, SLOT(save()));
    connect(this, SIGNAL(updatedAPI()),
            item, SLOT(reload()));
}

void DlgPrefSound::insertItem(DlgPrefSoundItem *pItem, QVBoxLayout *pLayout) {
    int pos;
    for (pos = 0; pos < pLayout->count() - 1; ++pos) {
        DlgPrefSoundItem *pOther(qobject_cast<DlgPrefSoundItem*>(
            pLayout->itemAt(pos)->widget()));
        if (!pOther) continue;
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
        if (latencyComboBox->count() <= 0) {
            updateLatencies(sampleRateIndex); // so the latency combo box is
            // sure to be populated, if setCurrentIndex is called with the
            // currentIndex, the currentIndexChanged signal won't fire and
            // the updateLatencies slot won't run -- bkgood lp bug 689373
        }
    }
    int latencyIndex = latencyComboBox->findData(m_config.getLatency());
    if (latencyIndex != -1) {
        latencyComboBox->setCurrentIndex(latencyIndex);
    }
    emit(loadPaths(m_config));
    m_loading = false;
}

/**
 * Slot called when the user selects a different API, or the
 * software changes it programatically (for instance, when it
 * loads a value from SoundManager). Refreshes the device lists
 * for the new API and pushes those to the path items.
 */
void DlgPrefSound::apiChanged(int index) {
    m_config.setAPI(apiComboBox->itemData(index).toString());
    refreshDevices();
    // JACK sets its own latency
    if (m_config.getAPI() == MIXXX_PORTAUDIO_JACK_STRING) {
        latencyLabel->setEnabled(false);
        latencyComboBox->setEnabled(false);
    } else {
        latencyLabel->setEnabled(true);
        latencyComboBox->setEnabled(true);
    }
}

/**
 * Updates the list of APIs, trying to keep the API and device selections
 * constant if possible.
 */
void DlgPrefSound::updateAPIs() {
    QString currentAPI(apiComboBox->itemData(apiComboBox->currentIndex()).toString());
    emit(updatingAPI());
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
    emit(updatedAPI());
}

/**
 * Slot called when the sample rate combo box changes to update the
 * sample rate in the config.
 */
void DlgPrefSound::sampleRateChanged(int index) {
    m_config.setSampleRate(
            sampleRateComboBox->itemData(index).toUInt());
}

/**
 * Slot called when the latency combo box is changed to update the
 * latency in the config.
 */
void DlgPrefSound::latencyChanged(int index) {
    m_config.setLatency(
            latencyComboBox->itemData(index).toUInt());
}

/**
 * Slot called whenever the selected sample rate is changed. Populates the
 * latency input box with SMConfig::kMaxLatency values, starting at 1ms,
 * representing a number of frames per buffer, which will always be a power
 * of 2 (so the values displayed in ms won't be constant between sample rates,
 * but they'll be close).
 */
void DlgPrefSound::updateLatencies(int sampleRateIndex) {
    double sampleRate = sampleRateComboBox->itemData(sampleRateIndex).toDouble();
    int oldLatency = latencyComboBox->currentIndex();
    unsigned int framesPerBuffer = 1; // start this at 0 and inf loop happens
    // we don't want to display any sub-1ms latencies (well maybe we do but I
    // don't right now!), so we iterate over all the buffer sizes until we
    // find the first that gives us a latency >= 1 ms -- bkgood
    // no div-by-0 in the next line because we don't allow srates of 0 in our
    // srate list when we construct it in the ctor -- bkgood
    for (; framesPerBuffer / sampleRate * 1000 < 1.0; framesPerBuffer *= 2) {
    }
    latencyComboBox->clear();
    for (unsigned int i = 0; i < SoundManagerConfig::kMaxLatency; ++i) {
        float latency = framesPerBuffer / sampleRate * 1000;
        // i + 1 in the next line is a latency index as described in SSConfig
        latencyComboBox->addItem(QString(tr("%1 ms")).arg(latency,0,'g',3), i + 1);
        framesPerBuffer <<= 1; // *= 2
    }
    if (oldLatency < latencyComboBox->count() && oldLatency >= 0) {
        latencyComboBox->setCurrentIndex(oldLatency);
    } else {
        // set it to the max, let the user dig if they need better latency. better
        // than having a user get the pops on first use and thinking poorly of mixxx
        // because of it -- bkgood
        latencyComboBox->setCurrentIndex(latencyComboBox->count() - 1);
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
    emit(refreshOutputDevices(m_outputDevices));
    emit(refreshInputDevices(m_inputDevices));
}

/**
 * Called when any of the combo boxes in this dialog are changed. Enables the
 * apply button and marks that settings have been changed so that
 * DlgPrefSound::slotApply knows to apply them.
 */
void DlgPrefSound::settingChanged() {
    if (m_loading) return; // doesn't count if we're just loading prefs
    m_settingsModified = true;
    if (!applyButton->isEnabled()) {
        applyButton->setEnabled(true);
    }
}

/**
 * Slot called when the "Query Devices" button is clicked.
 */
void DlgPrefSound::queryClicked() {
    m_pSoundManager->queryDevices();
    updateAPIs();
}

/**
 * Slot called when the "Reset to Defaults" button is clicked.
 */
void DlgPrefSound::resetClicked() {
    SoundManagerConfig newConfig;
    newConfig.loadDefaults(m_pSoundManager, SoundManagerConfig::ALL);
    loadSettings(newConfig);
    settingChanged(); // force the apply button to enable
}

void DlgPrefSound::bufferUnderflow(double count) {
    bufferUnderflowCount->setText(QString::number(count));
    update();
}
