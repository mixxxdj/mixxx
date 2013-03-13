/***************************************************************************
                          dlgprefmodplug.cpp  -  modplug settings dialog
                             -------------------
    copyright            : (C) 2013 by Stefan Nuernberger
    email                : kabelfricker@gmail.com
***************************************************************************/

/***************************************************************************
*                                                                         *
*   This program is free software; you can redistribute it and/or modify  *
*   it under the terms of the GNU General Public License as published by  *
*   the Free Software Foundation; either version 2 of the License, or     *
*   (at your option) any later version.                                   *
*                                                                         *
***************************************************************************/

#include "dlgprefmodplug.h"
#include "ui_dlgprefmodplugdlg.h"
#include "configobject.h"
#include "soundsourcemodplug.h"

#include <QtDebug>

#define CONFIG_KEY "[Modplug]"

DlgPrefModplug::DlgPrefModplug(QWidget *parent, ConfigObject<ConfigValue> *_config) :
    QDialog(parent),
    ui(new Ui::DlgPrefModplug),
    config(_config)
{
    ui->setupUi(this);
}

DlgPrefModplug::~DlgPrefModplug()
{
    delete ui;
}

void DlgPrefModplug::slotApply()
{
    qDebug() << "Modplug Preferences - Apply and save configuration changes";

    applySettings();
    saveSettings();
}

void DlgPrefModplug::slotUpdate()
{
    qDebug() << "Modplug Preferences - Update Configuration Dialog from saved settings";

    loadSettings();
}

void DlgPrefModplug::loadSettings()
{
    ui->memoryLimit->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"PerTrackMemoryLimitMB"),"256").toInt());
    ui->oversampling->setChecked(
        config->getValueString(ConfigKey(CONFIG_KEY,"OversamplingEnabled"),"1") == QString("1"));
    ui->noiseReduction->setChecked(
        config->getValueString(ConfigKey(CONFIG_KEY,"NoiseReductionEnabled"),"1") == QString("1"));
    ui->stereoSeparation->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"StereoSeparation"),"1").toInt());
    ui->maxMixChannels->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"MaxMixChannels"),"128").toInt());
    ui->resampleMode->setCurrentIndex(
        config->getValueString(ConfigKey(CONFIG_KEY,"ResamplingMode"),"3").toInt());
    ui->reverb->setChecked(
        config->getValueString(ConfigKey(CONFIG_KEY,"ReverbEnabled"),"0") == QString("1"));
    ui->reverbDepth->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"ReverbLevel"),"30").toInt());
    ui->reverbDelay->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"ReverbDelay"),"50").toInt());
    ui->megabass->setChecked(
        config->getValueString(ConfigKey(CONFIG_KEY,"MegabassEnabled"),"0") == QString("1"));
    ui->bassDepth->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"MegabassLevel"),"30").toInt());
    ui->bassCutoff->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"MegabassCutoff"),"80").toInt());
    ui->surround->setChecked(
        config->getValueString(ConfigKey(CONFIG_KEY,"SurroundEnabled"),"0") == QString("1"));
    ui->surroundDepth->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"SurroundLevel"),"30").toInt());
    ui->surroundDelay->setValue(
        config->getValueString(ConfigKey(CONFIG_KEY,"SurroundDelay"),"10").toInt());
}

void DlgPrefModplug::saveSettings()
{
    config->set(ConfigKey(CONFIG_KEY,"PerTrackMemoryLimitMB"),ConfigValue(ui->memoryLimit->value()));
    config->set(ConfigKey(CONFIG_KEY,"OversamplingEnabled"), ConfigValue(ui->oversampling->isChecked()));
    config->set(ConfigKey(CONFIG_KEY,"NoiseReductionEnabled"), ConfigValue(ui->noiseReduction->isChecked()));
    config->set(ConfigKey(CONFIG_KEY,"StereoSeparation"),ConfigValue(ui->stereoSeparation->value()));
    config->set(ConfigKey(CONFIG_KEY,"MaxMixChannels"),ConfigValue(ui->maxMixChannels->value()));
    config->set(ConfigKey(CONFIG_KEY,"ResamplingMode"),ConfigValue(ui->resampleMode->currentIndex()));
    config->set(ConfigKey(CONFIG_KEY,"ReverbEnabled"),ConfigValue(ui->reverb->isChecked()));
    config->set(ConfigKey(CONFIG_KEY,"ReverbLevel"),ConfigValue(ui->reverbDepth->value()));
    config->set(ConfigKey(CONFIG_KEY,"ReverbDelay"),ConfigValue(ui->reverbDelay->value()));
    config->set(ConfigKey(CONFIG_KEY,"MegabassEnabled"),ConfigValue(ui->megabass->isChecked()));
    config->set(ConfigKey(CONFIG_KEY,"MegabassLevel"),ConfigValue(ui->bassDepth->value()));
    config->set(ConfigKey(CONFIG_KEY,"MegabassCutoff"),ConfigValue(ui->bassCutoff->value()));
    config->set(ConfigKey(CONFIG_KEY,"SurroundEnabled"),ConfigValue(ui->surround->isChecked()));
    config->set(ConfigKey(CONFIG_KEY,"SurroundLevel"),ConfigValue(ui->surroundDepth->value()));
    config->set(ConfigKey(CONFIG_KEY,"SurroundDelay"),ConfigValue(ui->surroundDelay->value()));
}

void DlgPrefModplug::applySettings()
{
    // read ui parameters and configure soundsource
    unsigned int bufferSizeLimit = ui->memoryLimit->value() * 1024 * 1024;
    ModPlug::ModPlug_Settings settings;

    // Currently this is fixed to 16bit 44.1kHz stereo */
    /* Note that ModPlug always decodes sound at 44100kHz, 32 bit, stereo and then
     * down-mixes to the settings you choose. */
    settings.mChannels = 2;        /* Number of channels - 1 for mono or 2 for stereo */
    settings.mBits = 16;           /* Bits per sample - 8, 16, or 32 */
    settings.mFrequency = 44100;   /* Sampling rate - 11025, 22050, or 44100 */

    settings.mFlags = 0;
    if (ui->oversampling->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_OVERSAMPLING;
    if (ui->noiseReduction->isChecked())
        settings.mFlags |=  ModPlug::MODPLUG_ENABLE_NOISE_REDUCTION;
    if (ui->reverb->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_REVERB;
    if (ui->megabass->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_MEGABASS;
    if (ui->surround->isChecked())
        settings.mFlags |= ModPlug::MODPLUG_ENABLE_SURROUND;

    switch (ui->resampleMode->currentIndex())
    {
    case 0: /* nearest neighbor */
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_NEAREST;
        break;
    case 1: /* linear */
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_LINEAR;
        break;
    case 2: /* cubic spline */
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_SPLINE;
        break;
    case 3: /* 8 bit FIR (also default) */
    default:
        settings.mResamplingMode = ModPlug::MODPLUG_RESAMPLE_FIR;
        break;
    }

    settings.mStereoSeparation = ui->stereoSeparation->value();
    settings.mMaxMixChannels = ui->maxMixChannels->value();
    settings.mReverbDepth = ui->reverbDepth->value();      /* Reverb level 0(quiet)-100(loud)      */
    settings.mReverbDelay = ui->reverbDelay->value();      /* Reverb delay in ms, usually 40-200ms */
    settings.mBassAmount = ui->bassDepth->value();         /* XBass level 0(quiet)-100(loud)       */
    settings.mBassRange = ui->bassCutoff->value();         /* XBass cutoff in Hz 10-100            */
    settings.mSurroundDepth = ui->surroundDepth->value();  /* Surround level 0(quiet)-100(heavy)   */
    settings.mSurroundDelay = ui->surroundDelay->value();  /* Surround delay in ms, usually 5-40ms */
    settings.mLoopCount = 0; /* Number of times to loop.  Zero prevents looping. -1 loops forever. */

    // apply modplug settings
    SoundSourceModPlug::configure(bufferSizeLimit, settings);
}
