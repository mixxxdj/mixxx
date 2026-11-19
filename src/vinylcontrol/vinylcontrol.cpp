#include "vinylcontrol/vinylcontrol.h"

#include "moc_vinylcontrol.cpp"
#include "vinylcontrol/defs_vinylcontrol.h"
#include "waveform/visualplayposition.h"

VinylControl::VinylControl(UserSettingsPointer pConfig, const QString& group)
        : m_pConfig(pConfig),
          m_group(group),
          m_vinylControlInputGain(kVinylPrefKey, QStringLiteral("gain")),
          m_playButton(group, QStringLiteral("play")),
          m_visualPlayPos(VisualPlayPosition::getVisualPlayPosition(group)),
          m_trackSamples(group, QStringLiteral("track_samples")),
          m_trackSampleRate(group, QStringLiteral("track_samplerate")),
          m_vinylSeek(group, QStringLiteral("vinylcontrol_seek")),
          m_vinylControlRate(group, QStringLiteral("vinylcontrol_rate")),
          m_rateRatio(group, QStringLiteral("rate_ratio")),
          m_duration(group, QStringLiteral("duration")),
          m_mode(group, QStringLiteral("vinylcontrol_mode")),
          m_enabled(group, QStringLiteral("vinylcontrol_enabled")),
          m_wantEnabled(group, QStringLiteral("vinylcontrol_wantenabled")),
          m_cueing(group, QStringLiteral("vinylcontrol_cueing")),
          m_scratching(group, QStringLiteral("vinylcontrol_scratching")),
          m_vinylStatus(group, QStringLiteral("vinylcontrol_status")),
          m_loopEnabled(group, QStringLiteral("loop_enabled")),
          m_signalEnabled(group, QStringLiteral("vinylcontrol_signal_enabled")),
          m_reverseButton(group, QStringLiteral("reverse")),
          m_passthroughEnabled(group, QStringLiteral("passthrough")),
          m_scratchPositionEnabled(group, QStringLiteral("scratch_position_enable")),
          m_iLeadInTime(m_pConfig->getValueString(
                                         ConfigKey(group, "vinylcontrol_lead_in_time"))
                          .toInt()),
          m_dVinylPosition(0.0),
          m_fTimecodeQuality(0.0f) {
    bool gainOk = false;
    double gain = m_pConfig->getValueString(ConfigKey(kVinylPrefKey, "gain"))
                          .toDouble(&gainOk);
    m_vinylControlInputGain.set(gainOk ? gain : 1.0);

    //Enabled or not -- load from saved value in case vinyl control is restarting
    m_bIsEnabled = m_wantEnabled.get() > 0.0;
}

bool VinylControl::isEnabled() {
    return m_bIsEnabled;
}

void VinylControl::toggleVinylControl(bool enable) {
    if (m_pConfig) {
        m_pConfig->set(ConfigKey(m_group,"vinylcontrol_enabled"), ConfigValue((int)enable));
    }

    m_enabled.set(enable);

    // Reset the scratch control to make sure we don't get stuck moving forwards or backwards.
    // actually that might be a good thing
    //if (!enable)
    //    controlScratch->set(0.0);
}

VinylControl::~VinylControl() {
    bool wasEnabled = m_bIsEnabled;
    m_enabled.set(false);
    m_vinylStatus.set(kVinylStatusDisabled);
    if (wasEnabled) {
        // if vinyl control is just restarting, indicate that it should
        // be enabled
        m_wantEnabled.set(true);
    }
}
