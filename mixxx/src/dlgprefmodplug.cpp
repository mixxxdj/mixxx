
#include <QtDebug>

#include "dlgprefmodplug.h"
#include "ui_dlgprefmodplugdlg.h"

#include "configobject.h"
#include "soundsourcemodplug.h"

#define CONFIG_KEY "[Modplug]"

DlgPrefModplug::DlgPrefModplug(QWidget *parent,
                               ConfigObject<ConfigValue> *_config) :
    QDialog(parent),
    m_pUi(new Ui::DlgPrefModplug),
    m_pConfig(_config)
{
    m_pUi->setupUi(this);
    m_pUi->advancedSettings->setVisible(m_pUi->showAdvanced->isChecked());
}

DlgPrefModplug::~DlgPrefModplug()
{
    delete m_pUi;
}

void DlgPrefModplug::slotApply()
{
    applySettings();
    saveSettings();
}

void DlgPrefModplug::slotUpdate()
{
    loadSettings();
}

void DlgPrefModplug::loadSettings()
{
    m_pUi->memoryLimit->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"PerTrackMemoryLimitMB"),"256").toInt());
    m_pUi->oversampling->setChecked(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"OversamplingEnabled"),"1") == QString("1"));
    m_pUi->noiseReduction->setChecked(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"NoiseReductionEnabled"),"0") == QString("1"));
    m_pUi->stereoSeparation->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"StereoSeparation"),"1").toInt());
    m_pUi->maxMixChannels->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"MaxMixChannels"),"128").toInt());
    m_pUi->resampleMode->setCurrentIndex(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"ResamplingMode"),"1").toInt());
    m_pUi->reverb->setChecked(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"ReverbEnabled"),"0") == QString("1"));
    m_pUi->reverbDepth->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"ReverbLevel"),"50").toInt());
    m_pUi->reverbDelay->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"ReverbDelay"),"50").toInt());
    m_pUi->megabass->setChecked(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"MegabassEnabled"),"0") == QString("1"));
    m_pUi->bassDepth->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"MegabassLevel"),"50").toInt());
    m_pUi->bassCutoff->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"MegabassCutoff"),"50").toInt());
    m_pUi->surround->setChecked(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"SurroundEnabled"),"0") == QString("1"));
    m_pUi->surroundDepth->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"SurroundLevel"),"50").toInt());
    m_pUi->surroundDelay->setValue(m_pConfig->getValueString(
        ConfigKey(CONFIG_KEY,"SurroundDelay"),"50").toInt());
}

void DlgPrefModplug::saveSettings()
{
    m_pConfig->set(ConfigKey(CONFIG_KEY,"PerTrackMemoryLimitMB"),
                   ConfigValue(m_pUi->memoryLimit->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"OversamplingEnabled"),
                   ConfigValue(m_pUi->oversampling->isChecked()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"NoiseReductionEnabled"),
                   ConfigValue(m_pUi->noiseReduction->isChecked()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"StereoSeparation"),
                   ConfigValue(m_pUi->stereoSeparation->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"MaxMixChannels"),
                   ConfigValue(m_pUi->maxMixChannels->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"ResamplingMode"),
                   ConfigValue(m_pUi->resampleMode->currentIndex()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"ReverbEnabled"),
                   ConfigValue(m_pUi->reverb->isChecked()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"ReverbLevel"),
                   ConfigValue(m_pUi->reverbDepth->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"ReverbDelay"),
                   ConfigValue(m_pUi->reverbDelay->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"MegabassEnabled"),
                   ConfigValue(m_pUi->megabass->isChecked()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"MegabassLevel"),
                   ConfigValue(m_pUi->bassDepth->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"MegabassCutoff"),
                   ConfigValue(m_pUi->bassCutoff->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"SurroundEnabled"),
                   ConfigValue(m_pUi->surround->isChecked()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"SurroundLevel"),
                   ConfigValue(m_pUi->surroundDepth->value()));
    m_pConfig->set(ConfigKey(CONFIG_KEY,"SurroundDelay"),
                   ConfigValue(m_pUi->surroundDelay->value()));
}

void DlgPrefModplug::applySettings()
{
    // read ui parameters and configure soundsource
    unsigned int bufferSizeLimit = m_pUi->memoryLimit->value() << 20;
    ModPlug::ModPlug_Settings settings;

    // Note that ModPlug always decodes sound at 44.1kHz, 32 bit, stereo
    // and then down-mixes to the settings you choose.
    // Currently this is fixed to 16bit 44.1kHz stereo

    // Number of channels - 1 for mono or 2 for stereo
    settings.mChannels = 2;
    // Bits per sample - 8, 16, or 32
    settings.mBits = 16;
    // Sampling rate - 11025, 22050, or 44100
    settings.mFrequency = 44100;

    // enabled features flags
    settings.mFlags = 0;
    if (m_pUi->oversampling->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_OVERSAMPLING;
    if (m_pUi->noiseReduction->isChecked())
        settings.mFlags |=  ModPlug::MODPLUG_ENABLE_NOISE_REDUCTION;
    if (m_pUi->reverb->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_REVERB;
    if (m_pUi->megabass->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_MEGABASS;
    if (m_pUi->surround->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_SURROUND;

    switch (m_pUi->resampleMode->currentIndex()) {
    case 0: // nearest neighbor
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_NEAREST;
        break;
    case 1: // linear
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_LINEAR;
        break;
    case 2: // cubic spline
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_SPLINE;
        break;
    case 3: // 8 tap FIR (also default)
    default:
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_FIR;
        break;
    }

    // stereo separation 1(joint)-256(fully separated channels)
    settings.mStereoSeparation = m_pUi->stereoSeparation->value();
    // maximum number of mix channels (16-256)
    settings.mMaxMixChannels = m_pUi->maxMixChannels->value();
    // Reverb level 0(quiet)-100(loud)
    settings.mReverbDepth = m_pUi->reverbDepth->value();
    // Reverb delay in ms, usually 40-200ms
    settings.mReverbDelay = m_pUi->reverbDelay->value();
    // XBass level 0(quiet)-100(loud)
    settings.mBassAmount = m_pUi->bassDepth->value();
    // XBass cutoff in Hz 10-100
    settings.mBassRange = m_pUi->bassCutoff->value();
    // Surround level 0(quiet)-100(heavy)
    settings.mSurroundDepth = m_pUi->surroundDepth->value();
    // Surround front-rear delay in ms, usually 5-40ms
    settings.mSurroundDelay = m_pUi->surroundDelay->value();
    // Number of times to loop.  Zero prevents looping. -1 loops forever.
    settings.mLoopCount = 0;

    // apply modplug settings
    SoundSourceModPlug::configure(bufferSizeLimit, settings);
}
